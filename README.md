# Othello
This project aims to develop an artificial intelligence that can play the game of Othello, using the Minimax algorithm with Alpha-Beta pruning.<br>
The objective of the game is to capture as many opponent's pieces as possible to win the game. The rules of the game are available on [Wikipedia](https://en.wikipedia.org/wiki/Reversi).

<!-- ## Description -->
<!-- TODO -->

## Dependencies

Before building and running the project, the following dependencies need to be installed. Ensure you have a modern C++ compiler (supporting C++17) and `cmake` (version 3.5 or newer) installed on your system.
- `cmake` (minimum version 3.5)
- `make` (for Unix-like OS) / `nmake` (for Windows)
<!-- - `ncurses` library: This library is used for creating text-based interfaces. More details can be found [here](https://invisible-island.net/ncurses/announce.html). -->

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Cloning the Repository

To clone this repository, run the following command in your terminal:

```bash
git clone https://github.com/muhamm-ad/Othello.git
```

### Building the Project

Navigate to the root directory of the project, and run the provided bash script `run.sh` for Linux or `run.cmd` for Windows. This script automates the process of creating a build directory, running `cmake`, `make` or `nmake`, and running the game.

Before running the script, you may need to change its permissions to make it executable. Here's how you can do this:

For Linux:

```bash
chmod +x run.sh
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

## Contributing

Contributions to improve the project are welcome. Please feel free to open issues or submit pull requests.

## Authors

* [muhamm-ad · GitHub](https://github.com/muhamm-ad)

## License
This project is licensed under the GNU General Public License.