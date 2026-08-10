# S2Dumper

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://img.shields.io/github/actions/workflow/status/Swiftly-Tracker/s2dumper/build.yml?branch=main)](https://github.com/Swiftly-Tracker/s2dumper/actions)
[![Release](https://img.shields.io/github/v/release/Swiftly-Tracker/s2dumper?include_prereleases)](https://github.com/Swiftly-Tracker/s2dumper/releases)

S2Dumper is a dumper for Source 2 game binaries, built in C++. It loads a game's binaries offline (no running server needed) and dumps commands, convars, interfaces, schema/SDK classes, the entity system, and datamaps to disk.

## What it dumps

Running S2Dumper produces the following files inside `<output_path>`:

- **`commands.json`** — registered console commands
- **`convars.json`** — registered console variables (convars)
- **`interfaces.txt`** — exposed engine/game interfaces
- **`sdk.json`** — schema system classes, fields and enums
- **`entities.json`** — entity system classes
- **`datamaps.json`** — entity datamaps
- **`think_functions.txt`** — collected entity think functions

## Supported Games

- **Counter-Strike 2**

## Install / Build

### Prerequisites

- **Windows**: Visual Studio 2022, [XMake](https://xmake.io)
- **Linux**: GCC 14+, [XMake](https://xmake.io)

### Prebuilt

Grab the archive for your platform from the [latest release](https://github.com/Swiftly-Tracker/s2dumper/releases/latest).

### From source

```bash
git clone --recurse-submodules https://github.com/Swiftly-Tracker/s2dumper.git
cd s2dumper

# Windows
xmake -y

# Linux
xmake f --cc=gcc-14 --cxx=g++-14 -y
xmake build -y
```

Output binary:

- Windows: `build/windows/x64/release/s2dumper.exe`
- Linux: `build/linux/x86_64/release/s2dumper`

### Install into the game

S2Dumper needs to sit next to the game's own binaries to load them. After building (or downloading a prebuilt archive), copy the binary into the game install:

- Windows: `game/bin/win64/`
- Linux: `game/bin/linuxsteamrt64/`

Run it from that directory — not from the build output folder.

## Usage

```
s2dumper <output_path> <game>
```

- `<output_path>` — directory the dump files are written to (created if missing)
- `<game>` — short name of the game, e.g. `cs2`

Example, run from inside `game/bin/win64`:

```bash
s2dumper.exe ./dump cs2
```

On Linux, from inside `game/bin/linuxsteamrt64`:

```bash
./s2dumper ./dump cs2
```

## Architecture

```
s2dumper/
├── src/
│   ├── app/            # Application bootstrap, module loading
│   ├── dumper/         # Per-system dumpers (commands, convars, interfaces, schema, entities, datamaps)
│   ├── binary/          # Binary loading/parsing
│   ├── dynlib/          # Dynamic library loading
│   ├── hooks/           # Virtual function hooking
│   ├── shared/           # JSON, string and hash utilities
│   └── entrypoint.cpp   # CLI entry point
└── vendor/
    ├── s2sdk/            # Source 2 SDK (submodule)
    ├── safetyhook/       # Hooking library
    └── s2binlib/         # Binary loading support library
```

## Community

- **Issues**: [Report bugs and request features](https://github.com/Swiftly-Tracker/s2dumper/issues)

## Acknowledgements

All of the acknowledgements can be seen in [ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md)

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

---

<div align="center">
  <strong>Made with ❤️ by the Swiftly Development team</strong>
</div>
