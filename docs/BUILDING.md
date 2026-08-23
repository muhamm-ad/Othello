# Building from Source

This covers building the game yourself, useful if you want to modify the code, or if your platform isn't covered by the [prebuilt releases](https://github.com/muhamm-ad/Othello/releases).

## Time estimate

On a machine with nothing installed, budget **~20-40 minutes**, most of it spent installing build tools, a one-time cost that has nothing to do with this project specifically. Once those are in place, actually building the game takes a few minutes.

## Prerequisites

- A C++17 compiler:
  - **Linux**: GCC or Clang, e.g. `sudo apt install build-essential` on Debian/Ubuntu.
  - **macOS**: Xcode Command Line Tools, `xcode-select --install`.
  - **Windows**: [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) (select the "Desktop development with C++" workload), or full Visual Studio.
- [CMake](https://cmake.org/download/) 3.21 or newer.
- [Git](https://git-scm.com/downloads).
- [Ninja](https://ninja-build.org/), the build tool that actually invokes the compiler:
  - **Linux**: `sudo apt install ninja-build` (Debian/Ubuntu) or your distro's equivalent.
  - **macOS**: `brew install ninja`.
  - **Windows**: easiest is to enable the optional **"C++ CMake tools for Windows"** component when installing the Visual Studio Build Tools (it bundles Ninja alongside CMake, so there's nothing extra to install). Otherwise, `winget install Ninja-build.Ninja` works standalone.

You do **not** need to install SFML yourself, it's handled by vcpkg, see below.

## SFML via vcpkg

This project uses [vcpkg](https://vcpkg.io) in manifest mode (`vcpkg.json` at the repo root) to fetch and build SFML automatically. `vcpkg.json` only lists `graphics` and `window` as features, `System` isn't listed because it isn't an optional feature in vcpkg's SFML port: it's the base module every other module already depends on, so it's always built regardless of which features are enabled. All three (`SFML::Graphics`, `SFML::Window`, `SFML::System`) end up available to `find_package` either way; `audio` and `network` are the ones actually skipped here, since the game doesn't use them. You don't install SFML manually or set any environment variables for it, `run.cmd`/`run.sh` bootstrap a local copy of vcpkg (cloned into `vcpkg/` at the repo root, gitignored) the first time they run.

The very first build compiles SFML and its dependencies (FreeType, zlib, etc.) from source, which typically takes a couple of minutes. This is a **one-time cost per machine**: vcpkg caches the compiled packages in a local archive, so later builds, even after deleting `build/` reuse that cache and skip straight to configuring.

## Build & run

Clone the repository:

```bash
git clone https://github.com/muhamm-ad/Othello.git
cd Othello
```

Run the platform script from the project root:

- **Windows** from a *Developer Command Prompt* or *Developer PowerShell for VS* (so `cl` is on `PATH`):

  ```cmd
  .\run.cmd
  ```

- **Linux/macOS**:

  ```bash
  chmod u+x run.sh
  ./run.sh
  ```

The script bootstraps vcpkg if needed, configures with CMake + Ninja, and builds. Builds are incremental, rerunning the script after a code change only recompiles what's affected. When it finishes, it asks whether you want to launch the game immediately.

If something seems off (a stale cache, a half-finished configure), pass `--clean` to force a fresh `build/` directory:

```bash
./run.sh --clean      # Linux/macOS
.\run.cmd --clean       # Windows
```

To run the game manually afterward:

- **Windows**: `.\build\Othello.exe`
- **Linux/macOS**: `./build/Othello`

On launch, pick a difficulty and whether you play as Black or White (Black moves first).

## If you'd rather run the steps yourself instead of trusting the script above

1. Bootstrap vcpkg (skip if `vcpkg/vcpkg.exe` or `vcpkg/vcpkg` already exists, this step only needs to happen once):

   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   ```

   - Windows: `vcpkg\bootstrap-vcpkg.bat`
   - Linux/macOS: `./vcpkg/bootstrap-vcpkg.sh`

2. Configure CMake with the Ninja generator, pointing it at vcpkg's toolchain file:

   ```bash
   cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
   ```

   On the first run, this step is also where SFML actually gets built via vcpkg, see the timing note above. This command is safe to rerun any time; CMake only regenerates what's stale.

3. Build:

   ```bash
   cmake --build build --parallel
   ```

   This is incremental, rerunning it after editing code only recompiles the affected files, not the whole project.

4. Run:
   - Windows: `.\build\Othello.exe`
   - Linux/macOS: `./build/Othello`

   On launch, pick a difficulty and whether you play as Black or White.

## Troubleshooting

- **CMake can't find `Ninja`/`cl`, or configuring fails early on Windows**: make sure you're running from a "Developer Command Prompt for VS" or "Developer PowerShell for VS" (installed alongside the Visual Studio Build Tools) a plain terminal doesn't have the compiler on `PATH`. If `cl` is found but `ninja` isn't, install it (see Prerequisites above).
- **`CMake Error: ... Does not match the generator used previously`**: your `build/` directory was configured with a different generator than Ninja at some point (e.g. you manually ran `cmake` with `-G "Visual Studio 17 2022"` or similar). Run the script with `--clean`, or delete `build/` manually, then reconfigure.
- **Compile errors mentioning `sf::Event`, `sf::Style`, or `loadFromFile`**: this usually means CMake picked up a different, system-installed SFML instead of the vcpkg-provided one. The code here targets SFML 3's API, which made breaking changes from SFML 2.x (event handling, font loading, window construction, and more). Make sure `CMAKE_TOOLCHAIN_FILE` actually points at vcpkg and that no other `SFML_DIR` or `CMAKE_PREFIX_PATH` is overriding it.
- **A previous vcpkg clone got interrupted (partial checkout)**: delete the `vcpkg/` folder at the repo root and re-run the script, it re-clones automatically whenever `vcpkg/vcpkg.exe` (or `vcpkg/vcpkg`) isn't found.
