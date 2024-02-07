# Brick Breaker Game

This is a simple Brick Breaker game implemented in C++ using the console. Below is a brief overview of the code structure and functionality.

## Game Components
The game consists of the following main components:
1. Ball: Represents the bouncing ball in the game.
2. Bat: Represents the paddle controlled by the player to bounce the ball.
3. Bricks: The target objects that the player aims to destroy by bouncing the ball off the bat.
4. Game Area: The playable area within the console window.
5. Game Info: Stores information about the game state such as score, remaining lives, and game over status.

## Functionality
- **Movement**: The player can move the bat horizontally using the left and right arrow keys.
- **Collision Detection**: The game detects collisions between the ball and the bat, as well as between the ball and the bricks.
- **Score Keeping**: The game keeps track of the player's score, remaining lives, and the number of bricks destroyed.
- **Game Over**: The game ends when the player loses all lives or destroys all bricks.
- **Pause Menu**: The player can pause the game and access the menu to resume the game, start a new game, load a saved game, view score history, or exit the game.

## Installation and Execution
To run the game:
1. Compile the code using a C++ compiler.
2. Execute the compiled executable.
3. Use the arrow keys to move the bat and press 'Esc' to pause the game.

## Dependencies
- Windows API for console graphics.
- Conio.h for keyboard input.
- Cstdlib for standard library functions.
- Ctime for time-related functions.
- Fstream for file handling.

## Usage
- Use the left and right arrow keys to move the bat.
- Press 'Esc' to pause the game and access the menu.
- Select options from the menu using the arrow keys and 'Enter'.
- Find source.cpp and run it in the C++ compiler
- make sure the dependencies files are in the same directory as the source.cpp
