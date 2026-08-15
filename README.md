# Othello

This project develops an AI to play Othello using the Minimax algorithm with Alpha-Beta pruning. The objective is to capture as many opponent's pieces as possible. Learn the rules on [Wikipedia](https://en.wikipedia.org/wiki/Reversi).

## Interface

The game ships with an **SFML** graphical UI: click highlighted squares to place discs, watch the AI respond, and restart from the end-of-match overlay.

## Dependencies

Ensure you have a modern C++ compiler (supporting C++17) and `cmake` (version 3.5 or newer).

- [CMake](https://cmake.org/download/) (minimum version 3.5)
- Make: Pre-installed on Unix-like OS. For Windows, use `nmake` from Visual Studio or the [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019).
- [SFML 2.5+](https://www.sfml-dev.org/download.php) (`graphics`, `window`, `system`)
  - Linux (Debian/Ubuntu): `sudo apt install libsfml-dev`
  - macOS (Homebrew): `brew install sfml`
  - Windows: install SFML and point CMake at it, or use vcpkg (`vcpkg install sfml`)

## Getting Started

Follow these instructions to get a copy of the project running on your local machine.

### Cloning the Repository

Clone the repository with:
```bash
git clone https://github.com/muhamm-ad/Othello.git
```

### Building and Running the Project

Navigate to the project root directory and run the provided script:
- For Linux:
  ```bash
  chmod u+x run.sh
  ./run.sh
  ```
- For Windows:
  ```cmd
  run.cmd
  ```

The script automates building and running the game. After building, you can choose to run the game immediately, selecting 'X' or 'O' as your piece.

To run the game manually later, use:
- For Linux:
  ```bash
  ./build/Othello <X|O>
  ```
- For Windows:
  ```cmd
  .\build\Othello.exe <X|O>
  ```

Replace `<X|O>` with 'X' or 'O', depending on the piece you want to play with.
(`X` is dark and moves first; `O` is light.)

### Controls

- **Left click** a glowing cell to play
- **Esc** quit
- **R** or the on-screen button to restart after a match

## Contributing

Contributions are welcome. Open issues or submit pull requests.

## Authors

* [muhamm-ad · GitHub](https://github.com/muhamm-ad)

## License

This project is licensed under the GNU General Public License V3.

Fonts under `gameViewer/assets/fonts/` are DejaVu (Bitstream Vera-derived), redistributable under their own license.
