#include "mygraphics.h"
#include <conio.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

// Colors
COLORREF windowColor = RGB(12, 12, 12);
COLORREF whiteColor = RGB(250, 250, 250);
COLORREF blackColor = RGB(0, 0, 0);
COLORREF redColor = RGB(255, 0, 0);
COLORREF greenColor = RGB(106, 168, 79);
COLORREF yellowColor = RGB(255, 255, 0);
COLORREF grayColor = RGB(102, 102, 102);
COLORREF pinkColor = RGB(244, 204, 204);

// Window sizing
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

// Key Codes
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_RIGHT 77
#define KEY_LEFT 75
#define ESCAPE 27

// structure to store game information
struct GameInfo {
	bool gameOver;
	int remainingBalls;
	int score;
	float lives;
};
// Instance of Game Information structure
GameInfo gameInfo;

// Structure to store game area dimensions and cordinates
struct GameArea {
	int minX;
	int minY;
	int maxX = 800;
	int maxY = 600;
};
// Instance of Game Area to be used
GameArea gameArea;

// Structure to save ball information
struct Ball {
	int x;
	int y;
	int velocity = 10;
	int radius = 10;

	int xFactor;
	int yFactor;
};
// Instance of ball to be used
Ball ball;

// Structure to save bat information
struct Bat {
	int x;
	int y;
	int width = 200;
	int velocity;
};
// Instance of Bat to be used
Bat bat;

// Structure to save brick information
struct Brick
{
	int x = -1;
	int y = -1;
	bool visible = true;
	int type = -1;
};
// Array of bricks to be used
Brick bricks[35];

// Structure for brick's grid information
struct BricksInfo
{
	int width = gameArea.maxX / 5;
	int height = 35;
	int spacing = 10;

	// The rows which are added dynamically
	int rows;

	// Max rows and cols
	int maxCols = 5;
	int maxRows = 6;

	// To save the lowest brick position. It removes overhead while determining brick and ball collision.
	int lowestBrickY = 0;
};
// Instance of brick's grid to be used
BricksInfo bricksInfo;

// function to set console window height, width, etc.
void setWindow()
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions

	MoveWindow(console, 0, 0, gameArea.maxX + 30, gameArea.maxY + 30, TRUE);

	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hstdout, &csbi);

	csbi.dwSize.X = csbi.dwMaximumWindowSize.X;
	csbi.dwSize.Y = csbi.dwMaximumWindowSize.Y - 30;
	SetConsoleScreenBufferSize(hstdout, csbi.dwSize);

	HWND x = GetConsoleWindow();
	ShowScrollBar(x, SB_BOTH, FALSE);
}
void scoreHistory()
{
	if (gameInfo.gameOver == true || gameInfo.remainingBalls <= 0)
	{
		string myText;

		//appending in a file
		fstream myfile("scorehistory.txt", ios::app);
		myfile << gameInfo.score << endl;
		myfile.close();

		//read from file
		ifstream readmyfile("scorehistory.txt");
		while (getline(cin, myText))
		{
			cout << gameInfo.score;
		}
		readmyfile.close();
	}
}

// move cursor to x,y location on screen and write this text
void gotoXY(int x, int y, string text)
{
	CursorPosition.X = x;
	CursorPosition.Y = y;
	SetConsoleCursorPosition(console, CursorPosition);
	cout << text;
}

// Function to generate a random number within given range
int generateRandom(int min, int max)
{
	return (rand() % (max - min + 1)) + min;                
}

// Update Scoring on the bottom of screen
void showScore()
{
	stringstream info;

	// Show Score, Lives, etc.
	info << "  SCORE: " << gameInfo.score << "  ";
	info << "\t\t\t\t  LIVES: " << gameInfo.lives << "  ";
	info << "\t\t\t\t  REMAINING: " << gameInfo.remainingBalls << "   ";

	// Print Info
	gotoXY(0, 33, info.str());
}

// Logic to remove adjacent bricks in case ball has hit the yellow brick
void removeAdjacentBricks(int brickNo)
{
	int upperBrickNo = brickNo - bricksInfo.maxCols;
	// if its not out of bricks array
	if (upperBrickNo >= 0)
		bricks[upperBrickNo].visible = false;

	int lowerBrickNo = brickNo + bricksInfo.maxCols;
	// if its not out of the bricks array
	if (lowerBrickNo <= (bricksInfo.maxCols * bricksInfo.rows))
		bricks[lowerBrickNo].visible = false;

	int leftBrickNo = brickNo - 1;
	// if previous brick is not the last brick of row
	if (leftBrickNo % bricksInfo.maxCols != bricksInfo.maxCols - 1)
		bricks[leftBrickNo].visible = false;

	int rightBrickNo = brickNo + 1;
	// if next brick is not the first brick of row
	if (rightBrickNo % bricksInfo.maxCols != 0)
		bricks[rightBrickNo].visible = false;
}

// Logic to add a new layer of bricks in case ball has hit a gray brick
void addNewLayer(int brickNo)
{
	// if the ball has hit any brick from last layer, ignore adding
	int x = (bricksInfo.maxRows + 1) * bricksInfo.maxCols - (bricksInfo.maxCols - 1);
	if (brickNo >= x)
		return;

	// check which layer is empty
	// Idea is to first check the empty row, save its index. 
	// And then check it's previous row. If it has any brick in it then add layer on previosuly found row.
	int lastLayer = -1;
	for (int i = bricksInfo.maxRows; i >= 0; i--)
	{
		bool isCurrentLayerEmpty = true;
		for (int j = 0; j < bricksInfo.maxCols; j++)
		{
			int brickNo = i * bricksInfo.maxCols + j;

			// this layer was never built due to random rows generation, so we can use this one
			if (bricks[brickNo].x < 0)
			{
				isCurrentLayerEmpty = true;
				break;
			}

			// this row was built and there is some visible brick in it, so we should use this now
			if (bricks[brickNo].visible == true)
			{
				isCurrentLayerEmpty = false;
				break;
			}
		}

		// if there is any layer found previously and the current layer is not empty, we should add the previously found layer
		if (lastLayer != -1 && isCurrentLayerEmpty == false)
		{
			// Update randomly generated rows numbers if they were less than the newly adding row.
			if (bricksInfo.rows < lastLayer)
				bricksInfo.rows = lastLayer;

			// Delay so that all bricks in the new layer are added completely. Otherwise this conflicts with moving ball.
			Sleep(50);
			for (int n = 0; n < bricksInfo.maxCols; n++)
			{
				int brickNo = lastLayer * bricksInfo.maxCols + n;

				bricks[brickNo].visible = true;
				bricks[brickNo].x = n * bricksInfo.width;
				bricks[brickNo].y = lastLayer * bricksInfo.height;
				bricks[brickNo].type = generateRandom(0, 4);
			}
			break;
		}
		// If the iterating layer is empty, save its index for later use.
		else if (isCurrentLayerEmpty == true)
			lastLayer = i;
	}
}

// Scoring logic
void updateScore(int brickNo)
{

	// Game is over if lives are less than zero
	if (gameInfo.lives <= 0)
		gameInfo.gameOver = true;

	Brick brick = bricks[brickNo];

	switch (brick.type)
	{
	case 0: // plain - user will get 2 scores
		gameInfo.score += 2;
		break;
	case 1: // red triangle - no score, half life lost, 
		gameInfo.lives -= 0.5;
		break;
	case 2: // green circle
		// Need to consider half life case also
		if (gameInfo.lives <= 4)
			gameInfo.lives += 1;
		else if (gameInfo.lives > 4 && gameInfo.lives < 5)
			gameInfo.lives = 5;
		else
			gameInfo.score += 10;
		break;
	case 3: // yellow parallelogram
		gameInfo.score += 10;
		// remove adjucement bricks
		removeAdjacentBricks(brickNo);
		break;
	case 4: // Gray rectangle
		gameInfo.score -= bricksInfo.maxCols;
		// add new layer if less than 7
		addNewLayer(brickNo);
		break;
	default:
		break;
	}

	// Show updated score
	showScore();
}

// Initialize or reset the initial parameters of game, both in case of new game or reset game.
void setup()
{
	// Random number generator initiliazation
	srand(time(0));                                    

	// Set Bat's initial position
	bat.x = 300;
	bat.y = 500;

	// Set Ball's initial position
	ball.x = 450;
	ball.y = 350;

	// Set bricks
	for (int i = 0; i < 35; i++)
	{
		bricks[i].x = -1;
		bricks[i].y = -1;
		bricks[i].visible = true;
		bricks[i].type = -1;
	}

	// Set game scores
	gameInfo.gameOver = false;
	gameInfo.remainingBalls = 35;
	gameInfo.score = 0;
	gameInfo.lives = 3.0;

	// Generate Random layers
	bricksInfo.rows = generateRandom(1, bricksInfo.maxRows);

	// set ball movement direction (x,y)
	ball.xFactor = -1 * ball.velocity;
	ball.yFactor = -1 * ball.velocity;

	// Set bat initial position and velocity
	bat.velocity = ball.velocity + 20;

	// set window width, height and remove scroll
	setWindow();

	// Show initial score
	showScore();
}

// Draw menu, both on startup and on pause.
// the addResume paramter denotes whehter it is a call from pause of not.
// By default addResume is false which means it's a call from startup of game
void showMenu(bool addResume = false)
{
	int x = 400;
	int y = 100;
	int menuItem = 1;

	// In case it's a call from pause, change these values
	// Also set default selection
	if (addResume)
	{
		x = 350;
		menuItem = 0;
		myDrawText(x - 20, y + 30, 25, (char*)"->", whiteColor, windowColor);
	}
	else
		myDrawText(x - 20, y + 60, 25, (char*)"->", whiteColor, windowColor);

	while (true)
	{
		int key = 0;

		myDrawText(x - 30, y - 30, 65, (char*)"B R I C K   B R E A K E R", whiteColor, windowColor);

		if (addResume)
			myDrawText(x, y + 30, 25, (char*)"Resume", whiteColor, windowColor);

		myDrawText(x, y + 60, 25, (char*)"Start New Game", whiteColor, windowColor);
		myDrawText(x, y + 90, 25, (char*)"Load Saved Game", whiteColor, windowColor);
		myDrawText(x, y + 120, 25, (char*)"Score History", whiteColor, windowColor);
		myDrawText(x, y + 150, 25, (char*)"Exit", whiteColor, windowColor);
		myDrawText(x - 35, y + 210, 15, (char*)"Use Up / Down keys to change", whiteColor, windowColor);
		myDrawText(x - 35, y + 240, 15, (char*)"Press Enter to select", whiteColor, windowColor);

		key = _getch();

		// if user has pressed up / down arrow keys
		if (key == 224)
		{
			switch ((key = _getch()))
			{
			case KEY_UP:
				if (addResume)
				{
					if (menuItem >= 1)
						menuItem--;
				}
				else
				{
					if (menuItem > 1)
						menuItem--;
				}
				break;
			case KEY_DOWN:
				if (menuItem < 4)
					menuItem++;
				break;
			default:
				break;
			}
		}
		// If user has pressed ENTER
		else if (key == 13)
		{
			if (menuItem == 0)
			{
				system("cls");
				return;
			}
			else if (menuItem == 1)
			{
				system("cls");
				setup();
				return;
			}
			else if (menuItem == 2)
			{
				// TODO : Load game from file
				return;
			}
			else if (menuItem == 3)
			{
				//scoreHistory();
			}

			else if (menuItem == 4)
				exit(3);
		}

		// Logic to indicate selection 
		if (menuItem == 0)
		{
			myDrawText(x - 20, y + 30, 25, (char*)"->", whiteColor, windowColor);
			myDrawText(x - 20, y + 60, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 90, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 120, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 150, 25, (char*)"->", windowColor, windowColor);
		}
		else if (menuItem == 1)
		{
			myDrawText(x - 20, y + 30, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 60, 25, (char*)"->", whiteColor, windowColor);
			myDrawText(x - 20, y + 90, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 120, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 150, 25, (char*)"->", windowColor, windowColor);
		}
		else if (menuItem == 2)
		{
			myDrawText(x - 20, y + 30, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 60, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 90, 25, (char*)"->", whiteColor, windowColor);
			myDrawText(x - 20, y + 120, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 150, 25, (char*)"->", windowColor, windowColor);
		}
		else if (menuItem == 3)
		{
			myDrawText(x - 20, y + 30, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 60, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 90, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 120, 25, (char*)"->", whiteColor, windowColor);
			myDrawText(x - 20, y + 150, 25, (char*)"->", windowColor, windowColor);
		}
		else if (menuItem == 4)
		{
			myDrawText(x - 20, y + 30, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 60, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 90, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 120, 25, (char*)"->", windowColor, windowColor);
			myDrawText(x - 20, y + 150, 25, (char*)"->", whiteColor, windowColor);
		}
	}
}

// Logic for ball and brick collision
void removeBrick()
{
	// Check only if ball is in bricks grid
	if (ball.y <= bricksInfo.lowestBrickY)
	{
		for (int i = bricksInfo.rows; i >= 0; i--)
		{
			bool breaked = false;
			for (int j = 0; j < bricksInfo.maxCols; j++)
			{
				int brickNo = i * bricksInfo.maxCols + j;

				// check if brick's coordinates are same as ball's coordinates
				if (bricks[brickNo].visible == true
					&& bricks[brickNo].x > -1 && bricks[brickNo].y > -1
					&& ball.y >= bricks[brickNo].y + bricksInfo.spacing && ball.y <= bricks[brickNo].y + bricksInfo.height
					&& ball.x >= bricks[brickNo].x + bricksInfo.spacing && ball.x <= bricks[brickNo].x + bricksInfo.width)
				{
					// set the flag that this brick is destroyed now
					bricks[brickNo].visible = false;

					// ball has collision with brick, so reflect back it
					ball.yFactor *= -1;

					updateScore(brickNo);

					// set the loops break for better performance
					breaked = true;
					break;
				}
			}

			if (breaked)
				break;
		}
	}
}

// logic for the movement of ball, and collision with walls.
void moveBall()
{
	// remove previous ball (hack: make previous ball's color same as windows color)
	myEllipse(ball.x, ball.y, ball.x + ball.xFactor, ball.y + ball.yFactor, windowColor, windowColor);

	// set ball new position
	if (ball.x + ball.xFactor <= gameArea.maxX && ball.x + ball.xFactor >= 0)
		ball.x += ball.xFactor;
	if (ball.y + ball.yFactor <= gameArea.maxY && ball.y + ball.yFactor >= 0)
		ball.y += ball.yFactor;

	// left right walls collision
	if (ball.y < ball.radius || ball.y >= gameArea.maxY - ball.radius)
		ball.yFactor *= -1;

	// top bottom walls collision
	if (ball.x < ball.radius || ball.x > gameArea.maxX - ball.radius)
		ball.xFactor *= -1;

	// Logic for bat and ball collision
	if (ball.y + ball.radius >= bat.y && ball.x >= bat.x && ball.x <= bat.x + bat.width)
		ball.yFactor *= -1;

	removeBrick();

	// If ball is below bat
	if (ball.y > bat.y && gameInfo.lives > 0)
	{
		gameInfo.lives--;
		if (gameInfo.lives <= 0)                         
			gameInfo.gameOver = true;

		ball.x = 450;
		ball.y = 350;

		ball.xFactor = -1 * ball.velocity;
		ball.yFactor = -1 * ball.velocity;

		Sleep(1500);
	}

	// draw ball on new coordinates
	myEllipse(ball.x, ball.y, ball.x + ball.xFactor, ball.y + ball.yFactor, whiteColor, whiteColor);
}

void moveBat(int key)
{
	// check which arrow key is pressed
	switch ((key = _getch()))
	{
	case KEY_LEFT:
		if (bat.x - bat.velocity >= gameArea.minX)
			bat.x -= bat.velocity;
		break;
	case KEY_RIGHT:
		if (bat.x + bat.width <= gameArea.maxX)
			bat.x += bat.velocity;
		break;
	default:
		break;
	}
}

void pauseGame()
{
	system("cls");
	//scoreHistory();
	Sleep(200);
	showMenu(true);
}

// logic for movement of bat, along with arrow keys and pause handling
void listenKeys()
{
	// remove previous bat (hack: make previous bat's color same as windows color)
	myLine(bat.x, bat.y, bat.x + bat.width, bat.y, windowColor);

	int key = 0;

	// check if user has pressed any key
	if (_kbhit())
	{
		key = _getch();
		// if user has pressed arrow key
		if (key == 224)
			moveBat(key);
		// Since this is the only place where key events can be listened, so we need to handle the pause case here as well.
		else if (key == ESCAPE)
			pauseGame();
	}

	// draw the bat on new coordinates
	myLine(bat.x, bat.y, bat.x + bat.width, bat.y, whiteColor);
}

// logic to add Bricks
void addBricks()
{
	// Reset score
	gameInfo.remainingBalls = -1;

	for (int i = 0; i <= bricksInfo.rows; i++)
	{
		for (int j = 0; j < bricksInfo.maxCols; j++)
		{
			int brickNo = i * bricksInfo.maxCols + j;

			// Don't set x or y or type if it is -1, this denotes that this brick is not part of the game at the moment.
			if (bricks[brickNo].x == -1)
				bricks[brickNo].x = j * bricksInfo.width;
			if (bricks[brickNo].y == -1)
				bricks[brickNo].y = i * bricksInfo.height;
			if (bricks[brickNo].type == -1)
				bricks[brickNo].type = generateRandom(0, 4);

			int x1 = bricks[brickNo].x + bricksInfo.spacing;
			int y1 = bricks[brickNo].y + bricksInfo.spacing;
			int x2 = bricks[brickNo].x + bricksInfo.width;
			int y2 = bricks[brickNo].y + bricksInfo.height;

			// check if brick is marked as destroyed (visible == false)
			// if not, then build it
			if (bricks[brickNo].visible == true)
			{
				// Setting remainingBalls = 0 will cause the game to over.
				if (gameInfo.remainingBalls == -1)
					gameInfo.remainingBalls = 1;
				else
					gameInfo.remainingBalls++;

				switch (bricks[brickNo].type)
				{
					// plain
				case 0:
					myRect(x1, y1, x2, y2, pinkColor, pinkColor);
					break;
					// red triangle
				case 1:
				{
					myRect(x1, y1, x2, y2, pinkColor, pinkColor);

					int size = 13;
					for (int k = -size; k < size; k++)
						myLine(x1 + 70, y1 + 18, x1 + 70 + k, y1 + 9, redColor);

					break;
				}
				// green circle
				case 2:
					myRect(x1, y1, x2, y2, pinkColor, pinkColor);
					myEllipse(x1 + 60, y1 + 3, x1 + 60 + 20, y1 + 21, blackColor, greenColor);
					break;
					// yellow parallelogram
				case 3:
					myRect(x1, y1, x2, y2, pinkColor, pinkColor);
					myRect(x1 + 60, y1 + 3, x1 + 60 + 20, y1 + 20, blackColor, yellowColor);
					break;
					// Gray rectangle
				case 4:
					myRect(x1, y1, x2, y2, pinkColor, pinkColor);
					myRect(x1 + 20, y1 + 5, x1 + 20 + 100, y1 + 20, blackColor, grayColor);
					break;
				}

				// Save lowest brick's position
				bricksInfo.lowestBrickY = bricks[brickNo].y + bricksInfo.height;
			}
			// otherwise remove it (hack: make it's fill color as of window's color)
			else
				myRect(x1, y1, x2, y2, windowColor, windowColor);
		}
	}

	showScore();
}

// Logic to show game over or win states
void endGame(string text)
{
	while (1) {
		system("cls");
		Sleep(500);
		stringstream buffer;
		buffer << "\t\t\t\t\t\-------------\n";
		buffer << "\t\t\t\t\t\| " << text << " |\n";
		buffer << "\t\t\t\t\t\-------------\n";
		gotoXY(0, 15, buffer.str());
		showScore();
		//scoreHistory();
		Sleep(500);
	}
	_getch();
}

// Progarm start
int main()
{
	showMenu();
	setup();

	while (true)
	{
		addBricks();
		moveBall();
		listenKeys();

		if (gameInfo.gameOver == true)
			endGame("GAME OVER");

		if (gameInfo.remainingBalls < 0)
		{
			gameInfo.remainingBalls = 0;
			endGame("YOU WON!");
		}

		Sleep(50);
	}
}
