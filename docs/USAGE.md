# Usage

This guide assumes the project is already built (see
[INSTALLATION.md](INSTALLATION.md)) and that `zappy_server` / `zappy_gui` exist
at the repository root.

## Running the server

```sh
./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq
```

| Flag | Argument        | Description                                                |
|------|-----------------|-------------------------------------------------------------|
| `-p` | `port`          | TCP port the server listens on                              |
| `-x` | `width`         | Width of the world (number of tiles)                        |
| `-y` | `height`        | Height of the world (number of tiles)                       |
| `-n` | `name1 name2 …` | One or more team names (at least one required)              |
| `-c` | `clientsNb`     | Number of AI clients allowed per team                       |
| `-f` | `freq`          | Game speed: number of actions executed per second           |

```sh
./zappy_server --help   # print usage
```

### Example

A 10x10 world with two teams (`red`, `blue`), 3 slots each, running at the
default speed of 100:

```sh
./zappy_server -p 4242 -x 10 -y 10 -n red blue -c 3 -f 100
```

## Running the GUI

```sh
./zappy_gui
```

The GUI currently connects to a hardcoded `127.0.0.1:4242` (see
[`gui/src/main.cpp`](../gui/src/main.cpp)) — make sure a server is running on
that address/port **before** starting the GUI.

### Window navigation

- The GUI boots into an intro animation, then the main title screen, then a
  menu, before connecting and entering the game view (see
  [`SceneManager`](../gui/src/SceneManager/SceneManager.hpp) for the full
  scene flow).
- Once in the game view, a permanent **HUD bar** at the top of the window shows
  live stats: map size, server frequency, total players, eggs, the player count
  per team (color-coded), and the number of players at each level (1–8). See
  [`Hud`](../gui/src/Scene/Hud.hpp).

## Running the bundled AI client

The repository ships an autonomous AI in [`ai/`](../ai), packaged into a single
executable launcher `zappy_ai` (built by `make` / `make zappy_ai`):

```sh
./zappy_ai -p port -n name -h machine
```

| Flag     | Argument  | Description                                        |
|----------|-----------|----------------------------------------------------|
| `-p`     | `port`    | TCP port of the server                             |
| `-n`     | `name`    | Team name to join                                  |
| `-h`     | `machine` | Server hostname (defaults to `127.0.0.1`)          |
| `--help` |           | Print usage and exit                               |

```sh
./zappy_ai -p 4242 -n team1               # localhost by default
./zappy_ai -p 4242 -n team1 -h 10.0.0.5
```

Once launched the AI is fully autonomous: it connects, forks new players to
fill the team, coordinates same-level players over `Broadcast`, and runs
incantations. Pass `--debug` for a per-bot activity log instead of the default
live dashboard.

## Connecting custom AI clients

Any TCP client following the Zappy protocol can connect as an AI player. On
connection, send the team name followed by `\n`; the server replies with the
number of available slots and the map size:

```sh
nc 127.0.0.1 4242
red
```

```
WELCOME
red
3
10 10
```

From there, send one command per line (e.g. `Forward`, `Right`, `Left`,
`Look`, `Inventory`, `Broadcast <text>`, `Take <object>`, `Set <object>`,
`Connect_nbr`, `Fork`, `Eject`, `Incantation`).

## Development workflow

```sh
make debug    # build with -O0 -g -DDEBUG_MODE for both projects
make format   # apply clang-format to server/src and gui/src
make re       # full clean rebuild
```

### Tests

```sh
make tests_run            # run every suite (server unit+functional, AI, GUI)
make tests_unit_server    # Criterion unit tests for the game logic
make tests_func_server    # pytest black-box protocol tests against a live server
make tests_unit_ai        # pytest unit tests for the Python AI
make tests_unit_gui       # Criterion tests for GUI math + protocol parsing
```

See [TESTING.md](TESTING.md) for what each suite covers and its requirements.

`DEBUG_MODE` enables verbose logging via the `LOG_DEBUG` macro (see
[`Logger.hpp`](../server/src/Logger.hpp) / [`gui/src/Logger.hpp`](../gui/src/Logger.hpp)).
