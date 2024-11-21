#include <grrlib.h>
#include <ogc/lwp_watchdog.h> // Needed for gettime and ticks_to_millisecs
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <math.h>
#include <asndlib.h>
#include <mp3player.h>

// Font
#include "BMfont2_png.h"
#include "BMfont4_png.h"

// Images
#include "skeleton_jpg.h"

// Sounds
#include "chill_mp3.h"
#include "electrocute_mp3.h"

// Size of the sketch (fix for processing code)
int width = 640;
int height = 480;

// Colors (hex + transparency)
#define GRRLIB_MAROON 0x800000FF
#define GRRLIB_WHITE 0xFFFFFFFF
#define GRRLIB_TRANSPARENT_WHITE 0xFFFFFFDD
#define GRRLIB_BLACK 0x000000FF
#define GRRLIB_LIME 0x00FF00FF
#define GRRLIB_ORANGE 0xFFA000FF
#define GRRLIB_RED 0xFF0000FF
#define GRRLIB_BLUE 0x0000FFFF
#define GRRLIB_SILVER  0xC0C0C0FF

// Font
GRRLIB_texImg *fontTexture;

// Images
GRRLIB_texImg *skeleton_img;

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

// Game logic, adapted from processing
int stickX = 125;
int stickY = 300;
bool stickPickedUp = false;
bool buzzed = false;
bool shiftPressed = false;		// Flag to check if Shift is pressed
bool gameOver = true;			// Flag to check if game is over
bool gameStarted = false;		// Flag to check if the game has started
bool inMainMenu = true;			// Flag to check if in main menu
bool gameWon = false;			// Flag to check if game is won
bool electrocutePlayed = false; // Flag to check if electrocute sound has been played

// The path!!!
const int PATH_COUNT = 3;
int path[PATH_COUNT][4] = {
	{125, 300, 125, 100},
	{125, 100, 525, 100},
	{525, 100, 525, 300}
};

bool pointInRectangle(int px, int py, int x1, int y1, int x2, int y2)
{
	return (px > x1 && px < x2 && py > y1 && py < y2);
}

int dist(int x1, int y1, int x2, int y2) {
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

bool isOnPath(int x, int y)
{
	for (int i = 0; i < 3; i++)
	{
		if (distanceToLine(x, y, path[i][0], path[i][1], path[i][2], path[i][3]) < 25.0)
		{
			return true;
		}
	}
	return false;
}

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
	gameStarted = false;
	gameOver = false;
	gameWon = false;
	inMainMenu = true;
	stickX = 125;
	stickY = 300;
	stickPickedUp = false;
	buzzed = false;
	electrocutePlayed = false; // Reset electrocutePlayed flag
}

void showMainMenu()
{
	GRRLIB_FillScreen(GRRLIB_BLACK);

	const char *menuText = "MAIN MENU";
	int textX = 640 / 2; // Breedte van het scherm / 2
	int textY = 100;	 // Y-positie van de tekst
	GRRLIB_Printf(textX - (strlen(menuText) * 16), textY, fontTexture, GRRLIB_WHITE, 2, "%s", menuText);

	// Start Button
	Rect startButtonRect = Rect {
		.x = 640 / 2 - 75,
		.y = 480 / 2 - 25,
		.width = 150,
		.height = 50
	};

	bool hoveringStartButton = GRRLIB_PtInRect(startButtonRect.x, startButtonRect.y, startButtonRect.width, startButtonRect.height, mouseX, mouseY);
	int startButtonColor = hoveringStartButton ? GRRLIB_WHITE : GRRLIB_LIME;
	int startTextColor = hoveringStartButton ? GRRLIB_BLACK : GRRLIB_BLACK;
	GRRLIB_Rectangle(startButtonRect.x, startButtonRect.y, startButtonRect.width, startButtonRect.height, startButtonColor, true);
	GRRLIB_Printf(width / 2 - 36, height / 2 - 6, fontTexture, startTextColor, 1, "START");

	// Detect button click for Start
	if (mousePressed && hoveringStartButton) {
		inMainMenu = false;
		gameStarted = true;
		gameOver = false;
		gameWon = false;
		electrocutePlayed = false; // Reset electrocutePlayed flag when starting a new game
	}
}

void showGameOver()
{
	// Play the electrocute sound only once if the game is over
	// if (!electrocutePlayed) {
	//   electrocuteSound.play();
	  	electrocutePlayed = true;  // Mark electrocute sound as played
	// }

	// Scale the skeleton image to 70%
	float scaleFactor = 0.6;
	// Skeleton, draw scaled in the center, with transparency 
	GRRLIB_DrawImg(80, 50, skeleton_img, 0, scaleFactor, scaleFactor, GRRLIB_TRANSPARENT_WHITE);  // Draw a jpeg

        

	// Game Over text
	const char *menuText = "GAME OVER";
	int textX = 640 / 2; // Breedte van het scherm / 2
	int textY = 100;	 // Y-positie van de tekst
	GRRLIB_Printf(textX - (strlen(menuText) * 16), textY, fontTexture, GRRLIB_RED, 2, "%s", menuText);

	// Restart Button
	Rect restartButtonRect = Rect {
		.x = width / 2 - 75,
		.y = height / 2 + 25,
		.width = 150,
		.height = 50
	};

	bool hoveringRestartButton = GRRLIB_PtInRect(restartButtonRect.x, restartButtonRect.y, restartButtonRect.width, restartButtonRect.height, mouseX, mouseY);
	int restartButtonColor = hoveringRestartButton ? GRRLIB_WHITE : GRRLIB_ORANGE;
	int restartTextColor = hoveringRestartButton ? GRRLIB_BLACK : GRRLIB_BLACK;
	GRRLIB_Rectangle(restartButtonRect.x, restartButtonRect.y, restartButtonRect.width, restartButtonRect.height, restartButtonColor, true);
	GRRLIB_Printf(width / 2 - 55, height / 2 + 45, fontTexture, restartTextColor, 1, "RESTART");

	// Detect button click for Restart
	if (mousePressed && hoveringRestartButton) {
	  resetGame();
	}
}

void showGameWon()
{
	// fill(0, 255, 0);
	// textSize(32);
	// textAlign(CENTER, CENTER);
	// text("You Won!", width / 2, height / 2 - 50);

	// // Restart Button
	// fill(255, 165, 0);
	// rect(width / 2 - 75, height / 2 + 25, 150, 50);
	// fill(0);
	// textSize(24);
	// text("Restart", width / 2, height / 2 + 50);

	// // Detect button click for Restart
	// if (mousePressed && mouseX >= width / 2 - 75 && mouseX <= width / 2 + 75 && mouseY >= height / 2 + 25 && mouseY <= height / 2 + 75) {
	//   resetGame();
	// }
}

void playGame()
{
	for (int i = 0; i < PATH_COUNT; i++)
	{
		GRRLIB_Line(path[i][0], path[i][1], path[i][2], path[i][3], GRRLIB_SILVER);
	}
	
	// Green start block
	GRRLIB_Rectangle(100, 300, 50, 100, GRRLIB_LIME, true);
	// Blue end block
	GRRLIB_Rectangle(500, 300, 50, 100, GRRLIB_BLUE, true);

	// Draw stick if picked up
	if (stickPickedUp && shiftPressed && !gameOver) {
	  stickX = mouseX;
	  stickY = mouseY;
	}

	// The circle buzzer
	// Transparency for the circle
	GRRLIB_Circle(stickX, stickY, 25, GRRLIB_TRANSPARENT_WHITE, true);

	// Check if the stick is picked up
	if (!stickPickedUp && dist(stickX, stickY, mouseX, mouseY) < 20) {
	  stickPickedUp = true;
	}

	// Check if buzzer is hit (end block reached)
	if (stickPickedUp && (stickX > 500 && stickY > 300)) {
	  gameWon = true;
	  gameStarted = false;
	}

	// Check if the game is over (off path)
	if (!gameWon) {
	  checkGameOver();  // Only check game over if the game hasn't been won yet
	}

	// Display game over message if off path
	if (gameOver) {
	  gameStarted = false;
	}
}

int main()
{
	// Initialize GRRLIB and WiiMote
	GRRLIB_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	// Load font
	fontTexture = GRRLIB_LoadTexture(BMfont4_png);
	GRRLIB_InitTileSet(fontTexture, 16, 16, 32);

	// Load image
	skeleton_img = GRRLIB_LoadTexture(skeleton_jpg);

	// Initialize MP3 player and load sounds
	ASND_Init();
	MP3Player_Init();

	// Start playing background music in a loop
	MP3Player_PlayBuffer(chill_mp3, chill_mp3_size, NULL);

	while (true)
	{
		// Read input
		WPAD_SetVRes(0, 0, 0);
		WPAD_ScanPads();
		buttonsDown = WPAD_ButtonsDown(0);
		buttonsHeld = WPAD_ButtonsHeld(0);
		WPAD_IR(WPAD_CHAN_0, &ir1);

		// Cursor and mousePressed
		mouseX = ir1.sx - 190;
		mouseY = ir1.sy - 210;
		mousePressed = buttonsHeld & WPAD_BUTTON_A;
		shiftPressed = buttonsHeld & WPAD_BUTTON_B;

		// Clear the screen with black
		GRRLIB_FillScreen(GRRLIB_BLACK);

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

		// If the game is won, show the Win screen
		if (gameWon)
		{
			showGameWon();
		}

		if (buttonsDown & WPAD_BUTTON_HOME)
		{
			break;
		}

		// Draw cursor!
		GRRLIB_Circle(mouseX, mouseY, 25, GRRLIB_WHITE, true);

		GRRLIB_Render();
	}

	// GRRLIB_FreeTexture(fontTexture);
	GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB
	return 0;
}