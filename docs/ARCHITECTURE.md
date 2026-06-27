# Architecture

## Overview

```
┌──────────────┐        TCP (text protocol)        ┌──────────────┐
│  AI clients   │ ───────────────────────────────▶ │              │
│ (one per fd)  │ ◀─────────────────────────────── │              │
└──────────────┘                                    │  zappy_server │
┌──────────────┐        TCP (GUI protocol)          │  (game loop)  │
│  zappy_gui    │ ───────────────────────────────▶ │              │
│  (renderer)   │ ◀─────────────────────────────── │              │
└──────────────┘                                    └──────────────┘
```

The server is the single source of truth for the game state (map, players,
teams, eggs, resources). AI clients and the GUI are both regular TCP clients;
the server tells them apart during the handshake (`GRAPHIC` vs. team name) and
dispatches commands to two different command tables (`_aiCommands` /
`_guiCommands`, see [`Client.hpp`](../server/src/Client.hpp)).

## Repository layout

```
.
├── server/
│   └── src/
│       ├── main.cpp            # CLI args parsing, bootstraps Server + GameLogic
│       ├── Server.{hpp,cpp}     # TCP accept loop, poll() based event loop
│       ├── Client.{hpp,cpp}     # Per-connection state, command dispatch
│       ├── Socket.{hpp,cpp}     # Low-level socket wrapper
│       ├── Parser.{hpp,cpp}     # Tokenizes raw command lines
│       ├── Logger.hpp           # Leveled logging (LOG_DEBUG/INFO/...)
│       ├── Commands/            # One class per AI command (Command pattern)
│       │   └── Gui/             # One class per GUI command
│       └── Game/                # Game simulation
│           ├── GameLogic.{hpp,cpp}  # Main simulation: ticks, win condition, command effects
│           ├── Map.hpp / Tile.hpp   # Toroidal grid of resource tiles
│           ├── Team.{hpp,cpp}       # Team roster, eggs, slots
│           ├── Player.{hpp,cpp}     # Player state (position, level, inventory, ...)
│           ├── Egg.hpp              # Egg state (spawn slot reservation)
│           └── Common.hpp           # Shared constants/enums
│
└── gui/
    └── src/
        ├── main.cpp              # Entry point, creates Core and runs it
        ├── Core/
        │   ├── Core.{hpp,cpp}    # App bootstrap: window, network, scene manager, main loop
        │   └── Shader/           # GLSL shaders + Shader wrapper class
        ├── Window/               # Platform windowing (X11 on Linux, Cocoa on macOS)
        ├── SceneManager/         # Owns the current IScene, switches scenes
        ├── IScene/                # Scene interface (onEnter/update/draw/onExit)
        ├── Scene/                 # Concrete scenes: Intro, MainTitle, Menu, LoadNetwork, ...
        ├── Network/               # TCP client + protocol parsing -> GameState
        ├── GameState.hpp          # Snapshot of the world as seen by the GUI
        ├── Render/                # Camera, Skybox, generic render helpers
        ├── Sprite/                # Sprite + instanced grid rendering
        ├── Text/, Font/           # Text rendering (stb_truetype based)
        ├── Texture/               # Texture loading/management (stb_image based)
        ├── Audio/                 # Sound playback (miniaudio based)
        └── Utils/                 # Math helpers, OpenGL includes
```

## Server architecture

### Event loop

[`Server`](../server/src/Server.hpp) owns a listening [`Socket`](../server/src/Socket.hpp)
and a `poll()`-based event loop (`run()`):

1. Poll all client file descriptors + the listening socket.
2. On the listening socket: accept a new connection, wrap it in a `Client`.
3. On a client socket: read available bytes, split into lines, dispatch each
   line to `Client::processCommand`.
4. Each tick, `GameLogic::poll()` advances the simulation (resource respawn,
   player "life" countdown / starvation, win condition check).

### Clients & commands

[`Client`](../server/src/Client.hpp) buffers incoming bytes, splits them into
command lines (via [`Parser::ParseArgs`](../server/src/Parser.hpp)), and
dispatches them to one of two command tables depending on `ClientType`:

- **AI commands** (`commands::*`, implementing [`ICommand`](../server/src/Commands/ICommand.hpp)):
  `Forward`, `Right`, `Left`, `Inventory`, `Broadcast`, `Connect_nbr`, `Eject`,
  `Take`, `Set`, plus `Look`/`Incantation`/`fork` style handling inside
  `GameLogic`/`GameCommands`.
- **GUI commands** (`commands::gui::*`): `msz`, `bct`, `mct`, `tna` — these
  push world-state snapshots back to a connected GUI client.

Each command class implements a single `execute(client, args)` method —
classic **Command pattern**, making it easy to add a new protocol command by
dropping a new `.hpp`/`.cpp` pair and registering it in
`Client::registerCommands()`.

### Game simulation

[`game::GameLogic`](../server/src/Game/GameLogic.hpp) holds:

- `Map` — a toroidal (wrap-around) grid of `Tile`s, each with resource counts.
- `std::vector<std::unique_ptr<Team>>` — teams, each owning `Player`s and
  unhatched `Egg`s.
- Timers (`_lastLifeTime`, `_lastRessourceTime`) driving periodic updates at a
  rate derived from `_freq` (the `-f` CLI argument).

Player actions (`playerForward`, `playerTakeRessources`,
`playerBroadcast`, `playerEject`, ...) mutate the map/team/player state and
return the protocol responses the issuing AI client expects.

## GUI architecture

### Bootstrap & main loop

[`Zappy::Core`](../gui/src/Core/Core.hpp) is the application root:

- Opens a [`Window`](../gui/src/Window/Window.hpp) (X11/GLX on Linux, Cocoa/OpenGL on macOS).
- Creates the default `Shader`, the `SceneManager`, and the network stack
  (`TcpClient` + `NetworkManager`).
- `run()` loops at ~60 FPS (`FPS60` = 16 ms budget): poll window events, update
  the network manager, update/draw the current scene, swap buffers.

### Scenes

[`IScene`](../gui/src/IScene/IScene.hpp) defines the contract:
`onEnter() / update(events, gameState, netEvents, dt) / draw(shader) / onExit()`.
[`SceneManager`](../gui/src/SceneManager/SceneManager.hpp) owns the active
scene and switches between them based on the `SceneState` an `update()`
returns:

```
NONE → INTRO → TITLE → MENU → LOAD_NETWORK → GAME
```

Each scene receives shared resources (`TextureManager`, `audioManager`,
`NetworkManager`) instead of owning them, so assets and the connection
persist across scene transitions.

### Networking & game state

[`NetworkManager`](../gui/src/Network/NetworkManager.hpp) wraps an
[`INetworkClient`](../gui/src/Network/INetworkClient.hpp) (implemented by
`TcpClient`). It:

- Connects to the server as a GUI client.
- Reads lines, splits them, and routes each protocol command (`msz`, `bct`,
  `tna`, `pnw`, `ppo`, `plv`, `pin`, `pdi`, `sgt`, `pbc`, `pic`, `seg`, `enw`,
  `pgt`, `pdr`, `ebo`, `edi`, `pex`, `pie`, `pfk`, `sst`, `smg`, `suc`, `sbp`, ...)
  to a dedicated `handleXxx` method.
- Maintains a [`GameState`](../gui/src/GameState.hpp) snapshot (map size, tile
  contents, players, teams, eggs) consumed by scenes for rendering.
- Exposes one-shot [`NetworkEvent`](../gui/src/Network/NetworkEvent.hpp)s
  (e.g. "player died", "incantation started") via `consumeEvents()`, for
  triggering animations/sounds without re-deriving them from the state diff.

### Rendering pipeline

- `Render/` — `Camera`, `Skybox`, generic render utilities.
- `Sprite/` — textured quads and an `InstancedGrid` for drawing the tile map
  efficiently with instanced draw calls.
- `Core/Shader/` — GLSL shaders (vertex/fragment/particle/skybox/text/fade...)
  and the `Shader` class that compiles/links/binds them.
- `Texture/` — `Texture` (stb_image-backed) and `TextureManager` (cache by
  path).
- `Font/` + `Text/` — `stb_truetype`-backed glyph atlas and text rendering.
- `Audio/` — `audio` (miniaudio-backed playback) and `audioManager` (sound
  cache/mixing).

## AI architecture

The AI ([`ai/`](../ai)) is an `asyncio` Python program packaged into a single
executable launcher (`zappy_ai`) by [`ai/compile.py`](../ai/compile.py) (a
`zipapp` archive). It runs a *swarm* of cooperative bots over one event loop:

- [`zappy_ai.py`](../ai/zappy_ai.py) — CLI parsing (`-p`/`-n`/`-h`, `--help`),
  spawns the first bot, and renders a live dashboard (or a debug log).
- [`bot.py`](../ai/bot.py) — the `Bot` state machine. Each bot cycles through
  `collect` → `lead` / `follow` → `wait_incantation`, manages its hunger,
  forks new players to fill the team, and elects a leader per level through
  `Broadcast` messages keyed by a team hash.
- [`network.py`](../ai/network.py) — one async TCP connection per bot. A
  background reader routes server lines into two queues: command **responses**
  vs. asynchronous **events** (`message` broadcasts, `eject`).
- [`const.py`](../ai/const.py) — the elevation requirement table, resource
  order, and the tile-direction → move-sequence table.
- [`stats.py`](../ai/stats.py) — shared counters powering the dashboard.

Bots never block: `cmd()` sends one command and awaits its response, so a bot
stays within the server's 10-pending-request window.

## Network protocol (summary)

The protocol follows the standard Zappy specification:

- **AI clients** authenticate with a team name and receive `WELCOME` /
  available-slots / map size, then send one command at a time (`Forward`,
  `Right`, `Left`, `Look`, `Inventory`, `Broadcast text`, `Connect_nbr`,
  `Fork`, `Eject`, `Take object`, `Set object`, `Incantation`) and receive
  `ok` / `ko` / data responses, queued and executed with per-command time
  costs.
- **GUI clients** authenticate with `GRAPHIC` and receive a continuous stream
  of world-state updates (`msz`, `bct`, `tna`, `pnw`, `ppo`, `plv`, `pin`,
  `pdi`, `pex`, `pbc`, `pic`, `pie`, `pfk`, `enw`, `ebo`, `edi`, `sgt`, `sst`,
  `seg`, `smg`, `suc`, `sbp`, ...) describing every change in the world.

See [`server/src/Commands/`](../server/src/Commands) and
[`gui/src/Network/NetworkManager.cpp`](../gui/src/Network/NetworkManager.cpp)
for the authoritative implementation of each command.

## Testing

Each component is independently testable, and the design reflects that:

- **Server** — game logic lives in a `zappy_core` library separate from
  `main.cpp`, so Criterion unit tests link against it and call `GameLogic`,
  `Player`, `Map`, `Team` directly. Protocol behavior is covered end-to-end by
  pytest functional tests that drive a real `zappy_server` over TCP.
- **AI** — the `Bot` depends on an injected network object, so async fakes
  replace the socket and exercise its parsing/decision logic without a server.
- **GUI** — `NetworkManager` depends on the `INetworkClient` interface, so a
  fake client feeds it protocol lines and the resulting `GameState` is
  asserted; the math helpers are header-only and tested in isolation.

All suites run from the top-level `Makefile` (`make tests_run`). See
[TESTING.md](TESTING.md) for the full breakdown.
