#include <grrlib.h>
#include <stdio.h>
#include <ogc/lwp_watchdog.h> // Needed for gettime and ticks_to_millisecs
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <math.h>
#include <asndlib.h>
#include <mp3player.h>
#include <vector>
#include "random.hpp"

// Font
#include "BMfont2_png.h"
#include "BMfont4_png.h"

// Images
#include "skeleton_jpg.h"

#include "electrocuted1_jpg.h"
#include "electrocuted10_jpg.h"

// Sounds
#include "chill_mp3.h"
#include "electrocute_mp3.h"

#include <iostream>
#include <format>
#include <string>

// Random
using Random = effolkronium::random_static;

// Size of the sketch (fix for processing code)
int width;
int height;

// Colors (hex + transparency)
enum Colors
{
	MAROON = 0x800000FF,
	WHITE = 0xFFFFFFFF,
	TRANSPARENT_WHITE = 0xFFFFFFDD,
	BLACK = 0x000000FF,
	LIME = 0x00FF00FF,
	ORANGE = 0xFFA000FF,
	RED = 0xFF0000FF,
	BLUE = 0x0000FFFF,
	SILVER = 0xC0C0C0FF
};

// Level and highscore
int level = 1;
int score = 0;
int highscore = 0;

// Timer
int levelTimeLimit = 60;
int startTime = time(NULL);
int currentTime()
{
	return time(NULL) - startTime;
}

int remainingTime()
{
	return std::max(0, levelTimeLimit - currentTime()); // Zorg dat tijd niet negatief wordt
}

// Font
GRRLIB_texImg *fontTexture;

// Images
GRRLIB_texImg *skeleton_img;
GRRLIB_texImg *electrocutedImages[2];

// WiiMote
ir_t ir1; // infrared
int buttonsDown;
int buttonsHeld;
int mouseX;
int mouseY;
bool mousePressed;

// Shapes
struct Rect
{
	int x, y, width, height;
};

struct PathLine
{
	int x1, y1, x2, y2;
};

// Game logic, adapted from processing
int stickX = 125;
int stickY = 300;
bool stickPickedUp = false;
bool buzzed = false;
bool shiftPressed = false;		// Flag to check if Shift is pressed
bool gameOver = false;			// Flag to check if game is over
bool gameStarted = false;		// Flag to check if the game has started
bool inMainMenu = true;			// Flag to check if in main menu
bool gameWon = false;			// Flag to check if game is won
bool electrocutePlayed = false; // Flag to check if electrocute sound has been played
bool chillPlayed = false;		// Flag to check if soundtrack has been started
int rumbleTimer = 0;			// Rumble for one second when you lose

// math helpers -------------------------------------
int constrain(int val, int min, int max)
{
	return std::max(std::min(val, max), min);
}

bool pointInRectangle(int px, int py, int x1, int y1, int x2, int y2)
{
	return (px > x1 && px < x2 && py > y1 && py < y2);
}

int dist(int x1, int y1, int x2, int y2)
{
	return sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
}

float distanceToLine(int px, int py, int x1, int y1, int x2, int y2)
{
	float A = px - x1;
	float B = py - y1;
	float C = x2 - x1;
	float D = y2 - y1;

	float dot = A * C + B * D;
	float len_sq = C * C + D * D;
	float param = -1;

	if (len_sq != 0)
	{
		param = dot / len_sq;
	}

	float xx, yy;

	if (param < 0)
	{
		xx = x1;
		yy = y1;
	}
	else if (param > 1)
	{
		xx = x2;
		yy = y2;
	}
	else
	{
		xx = x1 + param * C;
		yy = y1 + param * D;
	}

	float dx = px - xx;
	float dy = py - yy;

	return sqrt(dx * dx + dy * dy);
}

// the path!! -------------------------------------
// std::vector<PathLine> pathLines;
#define NUM_POINTS 6
PathLine pathLines[NUM_POINTS + 1];


int startX = 125, startY = 300;
int endX = 525, endY = 300;


void generateRandomPath()
{	
	// Calculate equal spacing between startX and endX
	int segmentWidth = (endX - startX) / (NUM_POINTS + 1); // Total space divided by number of points

	int prevX = startX;
	int prevY = startY;

	// Generate points with equal distance between them
	for (int i = 0; i < NUM_POINTS; i++)
	{
		int x = prevX + segmentWidth; // Calculate the next X position, evenly spaced
		// int y = 50; //Random::get(50, 300); // Randomize Y position between 150 and 250
		int y = Random::get(50, 300); // Randomize Y position between 150 and 250

		// Ensure the points are spaced out evenly
		x = constrain(x, startX + 10, endX - 10); // Constrain to avoid points on edges
		y = constrain(y, 0, height);			  // Constrain Y to canvas height

		// FIXME something goes wrong here
		PathLine newPoint = PathLine {.x1 = prevX, .y1 = prevY, .x2 = x, .y2 = y};
		pathLines[i] = newPoint;

		prevX = x; // Update the previous x for the next iteration
		prevY = y;
	}

	PathLine endPoint = PathLine{.x1 = prevX, .y1 = prevY, .x2 = endX, .y2 = endY};

	pathLines[NUM_POINTS] = endPoint;
}


void drawPath()
{
	for (PathLine p : pathLines)
	{
		GRRLIB_Line(p.x1, p.y1, p.x2, p.y2, SILVER);
	}
}

bool isOnPath(int x, int y)
{
	for (PathLine p : pathLines)
	{
		if (distanceToLine(x, y, p.x1, p.y1, p.x2, p.y2) < 25.0)
		{
			return true;
		}
	}

	return false;
}

// game logic -------------------------------------
void checkGameOver()
{
	if (stickPickedUp && !isOnPath(stickX, stickY))
	{
		buzzed = true;
		gameOver = true;
	}
}

void resetGame()
{
	startTime = time(NULL);
	gameStarted = false;
	gameOver = false;
	gameWon = false;
	inMainMenu = true;
	stickX = 125;
	stickY = 300;
	stickPickedUp = false;
	buzzed = false;
	electrocutePlayed = false; // Reset electrocutePlayed flag
	chillPlayed = false;
	rumbleTimer = 0;
}

// Electrocuted animation
void loadElectrocutedAnimation()
{
	electrocutedImages[0] = GRRLIB_LoadTexture(electrocuted1_jpg);
	electrocutedImages[1] = GRRLIB_LoadTexture(electrocuted10_jpg);
}

int electrocutedFrame = 0;
void showElectrocutedAnimation()
{
	GRRLIB_DrawImg(170, 130, electrocutedImages[electrocutedFrame / 4 % 2], 0, 1.5, 1.5, TRANSPARENT_WHITE); // Draw a jpeg
	electrocutedFrame++;
}

void showMainMenu()
{
	if (!chillPlayed)
	{
		// Start playing background music.
		// TODO: figure out if it loops
		MP3Player_PlayBuffer(chill_mp3, chill_mp3_size, NULL);
		chillPlayed = true;
	}

	// Show menu text
	const char *menuText = "BUZZWIRE";
	GRRLIB_Printf(width / 2 - (strlen(menuText) * 16), 100, fontTexture, WHITE, 2, "%s", menuText);

	// Show highscore text
	std::string highscoreText = std::format("HIGHSCORE: {}", highscore);
	GRRLIB_Printf(width / 2 - (strlen(highscoreText.c_str()) * 8), 140, fontTexture, WHITE, 1, highscoreText.c_str());

	// Start Button
	Rect startButtonRect = Rect{
		.x = width / 2 - 75,
		.y = height / 2 - 25,
		.width = 150,
		.height = 50};

	bool hoveringStartButton = GRRLIB_PtInRect(startButtonRect.x, startButtonRect.y, startButtonRect.width, startButtonRect.height, mouseX, mouseY);
	int startButtonColor = hoveringStartButton ? WHITE : LIME;
	int startTextColor = BLACK;
	GRRLIB_Rectangle(startButtonRect.x, startButtonRect.y, startButtonRect.width, startButtonRect.height, startButtonColor, true);
	GRRLIB_Printf(width / 2 - 36, height / 2 - 6, fontTexture, startTextColor, 1, "START");

	// showElectrocutedAnimation();

	if (hoveringStartButton)
	{
		rumbleTimer = 1;
	}

	// Detect button click for Start
	if (mousePressed && hoveringStartButton)
	{
		// Generate a path!
		generateRandomPath();
		// SYS_Report("points: %i\r", pathLines.size()); // Log to check if generation went ok

		inMainMenu = false;
		gameStarted = true;
		gameOver = false;
		gameWon = false;
		electrocutePlayed = false; // Reset electrocutePlayed and chillPlayed flags when starting a new game
	}
}

void showAndCheckRestartButton()
{
	Rect restartButtonRect = Rect{
		.x = width / 2 - 75,
		.y = height / 2 + 80,
		.width = 150,
		.height = 50};

	bool hoveringRestartButton = GRRLIB_PtInRect(restartButtonRect.x, restartButtonRect.y, restartButtonRect.width, restartButtonRect.height, mouseX, mouseY);
	int restartButtonColor = hoveringRestartButton ? WHITE : ORANGE;
	int restartTextColor = BLACK;
	GRRLIB_Rectangle(restartButtonRect.x, restartButtonRect.y, restartButtonRect.width, restartButtonRect.height, restartButtonColor, true);
	GRRLIB_Printf(width / 2 - 55, height / 2 + 100, fontTexture, restartTextColor, 1, "RESTART");

	// Detect button click for Restart
	if (mousePressed && hoveringRestartButton)
	{
		level = 1;
		resetGame();
	}
}

void showGameOver()
{
	// Play the electrocute sound only once if the game is over
	if (!electrocutePlayed)
	{
		MP3Player_Stop();
		MP3Player_Volume(500);
		MP3Player_PlayBuffer(electrocute_mp3, electrocute_mp3_size, NULL);
		electrocutePlayed = true; // Mark electrocute sound as played
	}

	// Scale the skeleton image to 60%
	showElectrocutedAnimation();

	// Game Over text
	const char *menuText = "GAME OVER";
	GRRLIB_Printf(width / 2 - (strlen(menuText) * 16), 100, fontTexture, RED, 2, "%s", menuText);

	showAndCheckRestartButton();
}

void showGameWon()
{
	const char *menuText = "YOU WON!";
	int textX = width / 2; // Breedte van het scherm / 2
	int textY = 100;	   // Y-positie van de tekst
	GRRLIB_Printf(textX - (strlen(menuText) * 16), textY, fontTexture, LIME, 2, "%s", menuText);

	showAndCheckRestartButton();
}

void nextLevel()
{
	generateRandomPath();
	score += remainingTime();
	startTime = time(NULL);

	// Update highscore als de nieuwe score beter is
	if (score > highscore)
	{
		highscore = score;
	}

	level++;
	resetGame();
	inMainMenu = false;
	gameStarted = true;
}

void playGame()
{
	// Draw path
	drawPath();

	// Draw level, score and time left
	std::string levelText = std::format("LEVEL {}", level);
	GRRLIB_Printf(width / 2 - (strlen(levelText.c_str()) * 16), 30, fontTexture, WHITE, 2, levelText.c_str());

	// Score
	std::string scoreText = std::format("SCORE: {}", score);
	GRRLIB_Printf(width / 2 - (strlen(scoreText.c_str()) * 8), 350, fontTexture, WHITE, 1, scoreText.c_str());

	// Time left
	std::string timeLeftText = std::format("{}", remainingTime());
	// SYS_Report("time left: %i\r", timeLeftText);
	GRRLIB_Printf(width / 2 - (strlen(timeLeftText.c_str()) * 8), 400, fontTexture, remainingTime() < 10 ? RED : WHITE, 1, timeLeftText.c_str());

	// Green start block
	GRRLIB_Rectangle(100, 300, 50, 100, LIME, true);

	// Blue end block
	GRRLIB_Rectangle(500, 300, 50, 100, BLUE, true);

	// Move stick if picked up
	if (stickPickedUp && shiftPressed && !gameOver)
	{
		stickX = mouseX;
		stickY = mouseY;
	}

	// The circle buzzer
	GRRLIB_Circle(stickX, stickY, 25, TRANSPARENT_WHITE, true);

	// Check if the stick is picked up
	if (!stickPickedUp && dist(stickX, stickY, mouseX, mouseY) < 20)
	{
		stickPickedUp = true;
	}

	// Check if buzzer is hit (end block reached)
	if (stickPickedUp && (stickX > 500 && stickY > 300))
	{
		gameWon = true;
		gameStarted = false;
		nextLevel();
	}

	// Check if the game is over (off path)
	if (remainingTime() <= 0)
	{
		buzzed = true;
		gameOver = true;
	}

	if (!gameWon)
	{
		checkGameOver(); // Only check game over if the game hasn't been won yet
	}

	// Display game over message if off path, and rumble
	if (gameOver)
	{
		score = 0;
		rumbleTimer = 60;
		gameStarted = false;
	}
}

int main()
{
	// Initialize GRRLIB and WiiMote
	GRRLIB_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	// Set width and height
	width = rmode->viWidth;
	height = rmode->viHeight;

	// Load font
	fontTexture = GRRLIB_LoadTexture(BMfont4_png);
	GRRLIB_InitTileSet(fontTexture, 16, 16, 32);

	// Load images
	skeleton_img = GRRLIB_LoadTexture(skeleton_jpg);
	loadElectrocutedAnimation();

	// Initialize MP3 player and load sounds
	ASND_Init();
	MP3Player_Init();

	while (true)
	{
		// Read input
		WPAD_SetVRes(0, width, height);
		WPAD_ScanPads();
		buttonsDown = WPAD_ButtonsDown(0);
		buttonsHeld = WPAD_ButtonsHeld(0);
		WPAD_IR(WPAD_CHAN_0, &ir1);

		// Rumble!
		// printf("rumbleTimer: %i", rumbleTimer);
		// SYS_Report("rumbleTimer: %i\r", rumbleTimer);

		// SYS_Report("timer: %i\r", currentTime());
		if (rumbleTimer > 0)
		{
			WPAD_Rumble(0, 1);
			rumbleTimer--;
		}
		else
		{
			WPAD_Rumble(0, 0);
		}

		// Cursor and mousePressed
		mouseX = ir1.sx - 190;
		mouseY = ir1.sy - 210;
		mousePressed = buttonsHeld & WPAD_BUTTON_A;
		shiftPressed = buttonsHeld & WPAD_BUTTON_B;

		// Clear the screen with black
		GRRLIB_FillScreen(BLACK);

		// If in main menu, show the Start button
		if (inMainMenu)
		{
			showMainMenu();
		}

		// If the game has started, show the game screen
		if (gameStarted && !gameOver && !gameWon)
		{
			playGame();
		}

		// If the game is over, show the Game Over screen
		if (gameOver)
		{
			showGameOver();
		}


		if (buttonsDown & WPAD_BUTTON_HOME)
		{
			break;
		}

		// Draw cursor!
		GRRLIB_Circle(mouseX, mouseY, 25, WHITE, true);

		GRRLIB_Render();
	}

	// GRRLIB_FreeTexture(fontTexture);
	GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB
	return 0;
}