# Contributing

## Branching

- `main` is the stable/integration branch.
- Feature work happens on dedicated branches (e.g. `Game`, `gui`,
  `gui-network`) and is merged back via pull requests.

## Commit messages

This project follows [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short summary>
```

Common types: `feat`, `fix`, `refactor`, `chore`, `docs`.
The scope is usually `server`, `gui`, or a sub-area
(e.g. `server/game`, `gui/audio`, `make`, `nix`).

Examples from the repository's history:

```
fix(server/game): improve debug messages in GameLogic
feat(make): move to vcpkg to download gui's required packages
fix(gui/audio): use the system's miniaudio header file
```

## Code style

- C++20, formatted with `clang-format`. Run before committing:

  ```sh
  make format
  ```

- Compiler warnings are treated strictly (`-Wall -Wextra -Wpedantic` plus a
  curated set of `-Werror=...` flags, see [`server/CMakeLists.txt`](../server/CMakeLists.txt)
  and [`gui/CMakeLists.txt`](../gui/CMakeLists.txt)). New code must build
  warning-clean.

## Adding a new server command

1. Create `server/src/Commands/<Name>.{hpp,cpp}` (or
   `server/src/Commands/Gui/<name>.{hpp,cpp}` for GUI-facing commands)
   implementing [`ICommand`](../server/src/Commands/ICommand.hpp).
2. Implement the actual game-state mutation in
   [`game::GameLogic`](../server/src/Game/GameLogic.hpp) if needed.
3. Register the command in `Client::registerCommands()`
   ([`server/src/Client.cpp`](../server/src/Client.cpp)), in `_aiCommands` or
   `_guiCommands` as appropriate.

## Adding a new GUI scene

1. Create `gui/src/Scene/<Name>.hpp` implementing
   [`IScene`](../gui/src/IScene/IScene.hpp) (`onEnter`, `update`, `draw`,
   `onExit`).
2. Add a matching value to `Zappy::SceneState`
   ([`gui/src/IScene/IScene.hpp`](../gui/src/IScene/IScene.hpp)).
3. Handle the transition in
   [`SceneManager::update`](../gui/src/SceneManager/SceneManager.hpp).

## Dependencies

Only add new third-party C++ dependencies to the GUI via vcpkg
(`gui/vcpkg.json`) — see [INSTALLATION.md](INSTALLATION.md). Do not vendor
single-header libraries into `gui/src/Utils/` or add ad-hoc
`file(DOWNLOAD ...)` steps to CMake.

## Pull requests

- Keep PRs scoped to one area (`server` or `gui`) when possible.
- Ensure `make` succeeds for both `zappy_server` and `zappy_gui` before
  opening a PR.
- Run `make format` to avoid noisy formatting diffs.
