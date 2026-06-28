# Testing

The project ships three kinds of automated tests, each runnable from the
top-level `Makefile`:

| Suite | Tech | Scope | Make target |
|-------|------|-------|-------------|
| Server unit | [Criterion](https://github.com/Snaipe/Criterion) | Pure game logic (map, player, team, parser, getDir, conformity) | `make tests_unit_server` |
| Server functional | [pytest](https://pytest.org) | Black-box protocol: a real `zappy_server` driven over TCP | `make tests_func_server` |
| AI unit | pytest | Bot decision logic, protocol parsing, network routing, CLI | `make tests_unit_ai` |
| GUI unit | Criterion | Math helpers + GUI protocol parser (`NetworkManager`) | `make tests_unit_gui` |

Run **everything** with:

```sh
make tests_run
```

## Requirements

- **Criterion** (server & GUI unit tests). Install via your package manager:
  ```sh
  # macOS
  brew install criterion
  # Debian / Ubuntu
  sudo apt install libcriterion-dev
  ```
  The test CMake targets locate it through `pkg-config`.
- **Python 3** (server & AI unit tests). The Makefile creates and populates a
  local virtualenv automatically (`server/tests/.venv`, `ai/tests/.venv`);
  nothing to install by hand.

## Server unit tests

Criterion suites live in [`server/tests/unit/`](../server/tests/unit). They are
compiled against a `zappy_core` static library (every server source except
`main.cpp`) so they can call game logic directly.

```sh
make tests_unit_server
# or, manually:
cmake -S server -B server/build-tests -G Ninja -DBUILD_TESTS=ON \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
ninja -C server/build-tests
./server/build-tests/zappy_tests
```

Coverage highlights:

- `test_conformity.cpp` — every constant in `Common.hpp` (densities, time
  costs, survival time, level/orientation values) pinned to the subject.
- `test_player.cpp` / `test_player_dynamics.cpp` — movement, rotations,
  starvation/food conversion, inventory, levels.
- `test_map.cpp` / `test_tile_map.cpp` — toroidal wrap-around addressing.
- `test_team.cpp` — roster, eggs, slot accounting.
- `test_getdir.cpp` — the sound-direction octant computation for every
  orientation, including world wrap.
- `test_gamelogic*.cpp` — resource seeding, egg spawning, `getDir`,
  `getIndexByName`, `formatBct`, construction invariants.

Run a single test or family with Criterion's filter:

```sh
./server/build-tests/zappy_tests --filter 'player_move/*'
```

## Server functional tests

[`server/tests/functional/`](../server/tests/functional) starts a real server
on a free port (`conftest.py`'s `make_server` fixture) and drives it through a
small line-buffered TCP client ([`zappy_client.py`](../server/tests/functional/zappy_client.py)).

```sh
make tests_func_server
# or, manually (server binary must exist):
cd server/tests/functional && ../.venv/bin/python -m pytest
```

These cover the handshake and slot accounting, movement, look, inventory,
take/set, broadcast direction, fork/eject, incantation, resource respawn,
death/disconnect, the command queue cap, and unknown-command handling.

## AI unit tests

[`ai/tests/`](../ai/tests) test the Python AI without a server, using async
fakes ([`fakes.py`](../ai/tests/fakes.py)) that stand in for the network.

```sh
make tests_unit_ai
# or, manually:
cd ai/tests && .venv/bin/python -m pytest
```

- `test_const.py` — the elevation/resource/movement tables match the subject.
- `test_player.py` — the `Player` command layer: `cmd`/`look`/`inventory`
  parsing, the typed command wrappers (`take`/`set`/`broadcast`/…), passive
  level-ups, `incantation`, and `has_stones` across the whole elevation table.
- `test_bot_logic.py` — the `Bot` strategy: broadcast-key determinism, initial
  state, and event handling (leader election / ejection), driven through a fake
  network queue.
- `test_network_routing.py` — `Network.listen` routes broadcasts, ejections,
  responses and `dead` to the right queue.
- `test_cli.py` — the built `zappy_ai` launcher's `--help` / exit-84 behavior.

## GUI unit tests

[`gui/tests/`](../gui/tests) build a small, render-free test binary: the
header-only math helpers plus the protocol parser (`NetworkManager.cpp`). No
OpenGL, SFML or vcpkg is required — only Criterion.

```sh
make tests_unit_gui
# or, manually:
cmake -S gui/tests -B gui/build-tests -G Ninja
ninja -C gui/build-tests
./gui/build-tests/zappy_gui_tests
```

- `test_math.cpp` — `Zappy::Math` vectors/matrices (`cross`, `dot`,
  `normalize`, `translate`, `scale`, `perspective`, `lookAt`, `ortho`,
  rotations, `lerp`).
- `test_network_parsing.cpp` — feeds protocol lines (`msz`, `bct`, `tna`,
  `pnw`, `ppo`, `plv`, `pin`, `pdi`, ...) through a fake `INetworkClient` and
  asserts on the resulting `GameState`.

## Continuous integration

The same suites run in CI (see [`.github/workflows/ci.yml`](../.github/workflows/ci.yml))
on Linux (x86_64 / arm64) and macOS. Keep them green: a failing test blocks
the build matrix.
