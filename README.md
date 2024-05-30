# Othello

This project develops an AI to play Othello using the Minimax algorithm with Alpha-Beta pruning. The objective is to capture as many opponent's pieces as possible. Learn the rules on [Wikipedia](https://en.wikipedia.org/wiki/Reversi).

## Interface

Currently, the game interface is terminal-based. Players enter their moves in the terminal. A user-friendly GUI is in development.

## Dependencies

Ensure you have a modern C++ compiler (supporting C++17) and `cmake` (version 3.5 or newer).

- [CMake](https://cmake.org/download/) (minimum version 3.5)
- Make: Pre-installed on Unix-like OS. For Windows, use `nmake` from Visual Studio or the [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019).

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

## Contributing

Contributions are welcome. Open issues or submit pull requests.

## Authors

* [muhamm-ad · GitHub](https://github.com/muhamm-ad)

## License

This project is licensed under the GNU General Public License V3.
