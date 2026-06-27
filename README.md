# Zappy

Zappy is a network-based survival game inspired by *Zappy: The Awakening*.
Several teams of AI clients connect to a central **server** and compete to
gather resources, survive, and perform incantations to evolve their players
to the maximum level. A **GUI** client connects to the same server to display
the game in real time, in 3D.

This repository contains:

| Component | Path     | Language        | Description                                  |
|-----------|----------|-----------------|-----------------------------------------------|
| Server    | [`server/`](server) | C++20 | Game engine, world simulation, TCP server |
| GUI       | [`gui/`](gui)       | C++20 + OpenGL | 3D visualization client |
| AI        | [`ai/`](ai)         | Python (asyncio) | Autonomous client swarm: forks, coordinates teams over broadcast, drives incantations toward the higher levels |

## Documentation

| Guide | Description |
|-------|-------------|
| [INSTALLATION.md](docs/INSTALLATION.md) | Requirements, dependencies (vcpkg), and how to build the project on Linux/macOS |
| [USAGE.md](docs/USAGE.md) | How to run the server and the GUI, command-line arguments, examples |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Project layout, server/GUI/AI internal architecture, network protocol overview |
| [TESTING.md](docs/TESTING.md) | Test suites (server/AI/GUI), how to run them, and what they cover |
| [CONTRIBUTING.md](docs/CONTRIBUTING.md) | Coding style, formatting, branching and commit conventions |

## Quick start

```sh
# 1. Set up vcpkg once (see INSTALLATION.md for details)
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh -disableMetrics
export VCPKG_ROOT="$HOME/vcpkg"

# 2. Build everything
make

# 3. Run the server
./zappy_server -p 4242 -x 10 -y 10 -n team1 team2 -c 5 -f 100

# 4. Run the GUI (in another terminal)
./zappy_gui

# 5. Run an AI client (in another terminal)
./zappy_ai -p 4242 -n team1 -h 127.0.0.1
```

## Tests

```sh
make tests_run   # server (unit + functional) + AI + GUI test suites
```

See [TESTING.md](docs/TESTING.md) for per-suite details and requirements.

## License

This project is part of an Epitech curriculum assignment.
