# Othello

This project aims to develop an artificial intelligence that can play the game of Othello, using the Minimax algorithm with Alpha-Beta pruning.<br>
The objective of the game is to capture as many opponent's pieces as possible to win the game. The rules of the game are available on [Wikipedia](https://en.wikipedia.org/wiki/Reversi).

<!-- ## Description -->
<!-- TODO -->

## Interface

Currently, the game interface is terminal-based. Players interact with the game by entering their moves in the terminal in a specific format.

I'm working on adding a user-friendly GUI to enhance the gameplay experience. Stay tuned for updates!

## Dependencies

Before building and running the project, the following dependencies need to be installed. Ensure you have a modern C++ compiler (supporting C++17) and `cmake` (version 3.5 or newer) installed on your system.

- [CMake](https://cmake.org/download/) (minimum version 3.5)
- Make: This is typically pre-installed on Unix-like OS, but if it's missing you can find information on installing it [here](https://www.gnu.org/software/make/). For Windows, `nmake` is included with Visual Studio and the [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019).

Please note, the URLs provided above are valid as of the time of writing, but you may need to search for the latest versions or alternate sources if these links become outdated.

Remember to make sure your system path includes the locations of your installed dependencies so that your system can locate them when building and running the project.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Cloning the Repository

To clone this repository, run the following command in your terminal:
```bash
git clone https://github.com/muhamm-ad/Othello.git
```

### Building and Running the Project

Navigate to the root directory of the project, and run the provided bash script `run.sh` for Linux or `run.cmd` for Windows. <br>
This script automates the process of creating a build directory, running `cmake`, `make` or `nmake`, and running the game.

After the build process, the script will ask you if you want to run the game. If you answer yes, you will be asked to choose the type of piece you want to play with, either 'X' or 'O'. 

Before running the script, you may need to change its permissions to make it executable. Here's how you can do this:

For Linux:
```bash
chmod u+x run.sh
```

For Windows, the scripts should be runnable without additional permissions. However, if you encounter issues, ensure your user account has execution rights on the script file. 

After giving the necessary permissions, you can run the script:

For Linux:
```bash
./run.sh
```

For Windows:
```cmd
run.cmd
```

If you choose not to run the game immediately after the build, you can run it later by executing the Othello binary with your chosen piece type as an argument:

For Linux:
```bash
./build/Othello <X|O>
```

For Windows: <!-- TODO to be verified -->
```cmd
.\build\Othello.exe <X|O>
```


Replace `<X|O>` with either `X` or `O`, depending on the piece you want to play with.

## Contributing

Contributions to improve the project are welcome. Please feel free to open issues or submit pull requests.

## Authors

* [muhamm-ad · GitHub](https://github.com/muhamm-ad)

## License

This project is licensed under the GNU General Public License V3.