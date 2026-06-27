# Installation

This guide walks you through everything needed to build the **server** and the
**GUI** from a clean machine.

## 1. Requirements

| Tool         | Minimum version | Purpose                                  |
|--------------|-----------------|-------------------------------------------|
| CMake        | 3.25            | Build configuration                       |
| Ninja        | any             | Build generator (used by the Makefile)    |
| Clang or GCC | C++20 support   | Compiler                                   |
| vcpkg        | latest          | Third-party dependencies (GUI only)       |
| ccache       | any (optional)  | Speeds up rebuilds, auto-detected         |
| Criterion    | 2.x (optional)  | Server & GUI unit tests (`pkg-config`)    |
| Python       | 3.10+ (optional)| Server & AI test suites (auto venv)       |

### Platform-specific system libraries (GUI only)

The GUI links against system graphics libraries that are **not** managed by
vcpkg:

- **Linux**: OpenGL and X11 development headers
  ```sh
  # Debian / Ubuntu
  sudo apt install libgl1-mesa-dev libx11-dev

  # Fedora
  sudo dnf install mesa-libGL-devel libX11-devel

  # Arch
  sudo pacman -S mesa libx11
  ```
- **macOS**: Cocoa and OpenGL frameworks, provided by the Xcode Command Line
  Tools:
  ```sh
  xcode-select --install
  ```

The server has **no** third-party dependencies — only the C++ standard library
and POSIX sockets.

## 2. Setting up vcpkg

The GUI depends on two header-only libraries fetched and built through
[vcpkg](https://vcpkg.io):

- [`stb`](https://github.com/nothings/stb) — image loading (`stb_image.h`) and
  font rasterization (`stb_truetype.h`)
- [`miniaudio`](https://github.com/mackron/miniaudio) — audio playback

> ⚠️ **Important**: the `vcpkg` package from Homebrew/apt only installs the
> `vcpkg` binary, **not** the ports tree or the CMake toolchain file
> (`scripts/buildsystems/vcpkg.cmake`). You need a full git clone.

### Step by step

1. Clone the vcpkg repository (anywhere on your machine — your home directory
   is a good default):

   ```sh
   git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
   ```

2. Bootstrap it:

   ```sh
   ~/vcpkg/bootstrap-vcpkg.sh -disableMetrics
   ```

3. Export `VCPKG_ROOT` so the project's `Makefile` can locate the vcpkg
   toolchain file. Add this line to your shell profile
   (`~/.zshrc`, `~/.bashrc`, `~/.profile`, ...):

   ```sh
   export VCPKG_ROOT="$HOME/vcpkg"
   ```

4. Reload your shell:

   ```sh
   source ~/.zshrc   # or open a new terminal
   ```

That's it — `stb` and `miniaudio` are declared in [`gui/vcpkg.json`](../gui/vcpkg.json)
and will be downloaded and compiled automatically by CMake on the first build.

## 3. Building the project

Once `VCPKG_ROOT` is exported:

```sh
make                # build server + GUI (Release)
make debug          # build server + GUI (Debug, -O0 -g -DDEBUG_MODE)
make zappy_server   # build only the server
make zappy_gui      # build only the GUI
```

The resulting binaries `zappy_server` and `zappy_gui` are copied to the
repository root.

### Other useful targets

```sh
make format   # run clang-format on server/src and gui/src
make clean    # remove the build directories
make fclean   # clean + remove the zappy_server / zappy_gui / zappy_ai binaries
make re       # fclean + all (full rebuild)
make tests_run  # run the server, AI and GUI test suites (see TESTING.md)
```

## 4. Using the Nix development shell (optional)

A [`flake.nix`](../flake.nix) is provided with the required build tools
(`clang`, `cmake`, `ninja`, `pkg-config`):

```sh
nix develop
```

You still need to set up `VCPKG_ROOT` as described above — Nix does not
provide vcpkg.

## Troubleshooting

### `VCPKG_ROOT is not set`

```
Error: VCPKG_ROOT is not set. Install vcpkg and export VCPKG_ROOT=/path/to/vcpkg
```

You either haven't exported `VCPKG_ROOT`, or you're using a shell that
doesn't source the profile where it's defined. Verify with:

```sh
echo $VCPKG_ROOT
ls $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

If the second command fails, your `VCPKG_ROOT` points to an incomplete
install (e.g. the Homebrew binary-only package) — re-clone the full repo as
described above.

### `Could NOT find X11` / `Could NOT find OpenGL` (Linux)

Install the development packages listed in
[Platform-specific system libraries](#platform-specific-system-libraries-gui-only).

### First build is slow

The first configure step downloads and builds `stb` and `miniaudio` through
vcpkg. Subsequent builds reuse vcpkg's binary cache and are fast.
