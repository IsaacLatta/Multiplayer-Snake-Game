# Multiplayer Snake Game

## Description
- This snake game uses the Winsock api to facilitate a multiplayer experience. The game includes various fruits holding different values. There are also powerups included such as double points, invincibility star, speed boost and a clone snake that helps the player eat fruit.
- The game also poses enemy threats to the snake as well. These includes different types of mines the player must avoid, as well as creative enemies. Enemies such as the sleeping dragon who guards his treasure (fruits) and a gravity golem whose gravitational field pulls the player toward it, plus others.
- Note: The multiplayer aspect of the game is facilitated through a network connection not on the same machine.
  
## Installation

### Prerequisites
- The game must be run on a Windows version that supports the Winsock api.
- Ensure you have a C++ compiler installed, such as [GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/), or an IDE that supports C++ such as [Visual Studio](https://visualstudio.microsoft.com/).
- Make sure your console or terminal supports UTF-8 encoding. For Windows, you may need to set the code page to 65001 to handle UTF-8 correctly by running `chcp 65001` in the Command Prompt or PowerShell.
- Or by saving all the files with UTF-8 encoding with code page option 65001 selected.

 ### Compiling the Game

- Clone the repository to your local machine or download the source files.
- Open the Command Prompt or terminal window in the directory where the source files are located.
- Use the C++ compiler to compile the source code. For example, with GCC, the command would be: g++ -o SnakeGameServerApp main.cpp -std=c++11
- If using an IDE similar to Visual Studio, compile and run the program using F5 or the Run button. 

### Running the Game

- The server side of the game may be ran independently of the client as a normal singleplayer game. To run the client side application (which must be done in conjuction with the server), repeat the above steps in the directory where the Client side's main.cpp is saved.
- If your machine is protected behind a firewall or IDS, administrator approval may be required to allow network connections for the game. Singleplayer will likely still be able to be played with out additional priveledges.
- Recommendation: The terminal font size should be increased as the emojies are quite small.
- The snake is controlled with the arrow key, the client side will be prompted to input the server's ip address to connect, this address should be IP_v4.


## Troubleshooting
- If you encounter issues with network connectivity, ensure that the correct port is open and not blocked by your firewall. The game has port 50000 hardcoded but this can be changed in the Client.h and Server.h files.
- Problems with character display may require checking the code page settings in your terminal.
