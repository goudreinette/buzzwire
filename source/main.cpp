#include <grrlib.h>
#include <ogc/lwp_watchdog.h> // Needed for gettime and ticks_to_millisecs
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <math.h>

#include "BMfont2_png.h"
#include "BMfont4_png.h"


// Colors
#define GRRLIB_MAROON 0x800000FF
#define GRRLIB_WHITE 0xFFFFFFFF
#define GRRLIB_BLACK 0x000000FF
#define GRRLIB_LIME 0x00FF00FF

// Font
GRRLIB_texImg *fontTexture;

// WiiMote
ir_t ir1; // infrared
int mouseX;
int mouseY;
bool mousePressed;

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


int path[3][4] = {
	{125, 300, 125, 100},
	{125, 100, 525, 100},
	{525, 100, 525, 300}
};

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
	// fill(255);
	GRRLIB_FillScreen(GRRLIB_BLACK);
	// textSize(32);
	// textAlign(CENTER, CENTER);
	// text("Main Menu", width / 2, 100);

	const char *menuText = "MAIN MENU";
	int textX = 640 / 2; // Breedte van het scherm / 2
	int textY = 100;	 // Y-positie van de tekst
	GRRLIB_Printf(textX - (strlen(menuText) * 16), textY, fontTexture, GRRLIB_WHITE, 2, "%s", menuText);

	// // Start Button
	// fill(0, 255, 0);
	// rect(width / 2 - 75, height / 2 - 25, 150, 50);
	// fill(0);
	// textSize(24);
	// text("Start", width / 2, height / 2);
	GRRLIB_Rectangle(640 / 2 - 75, 480 / 2 - 25, 150, 50, GRRLIB_LIME, true);
	GRRLIB_Printf(640 / 2 - 36, 480 / 2 - 6, fontTexture, GRRLIB_BLACK, 1, "START");

	// // Detect button click for Start
	// if (mousePressed && mouseX >= width / 2 - 75 && mouseX <= width / 2 + 75 && mouseY >= height / 2 - 25 && mouseY <= height / 2 + 25) {
	//   inMainMenu = false;
	//   gameStarted = true;
	//   gameOver = false;
	//   gameWon = false;
	//   electrocutePlayed = false;  // Reset electrocutePlayed flag when starting a new game
	// }
}

void showGameOver()
{
	// // Play the electrocute sound only once if the game is over
	// if (!electrocutePlayed) {
	//   electrocuteSound.play();
	//   electrocutePlayed = true;  // Mark electrocute sound as played
	// }

	// // Scale the skeleton image to 70%
	// float scaleFactor = 0.6;
	// float imgWidth = skeletonImage.width * scaleFactor;
	// float imgHeight = skeletonImage.height * scaleFactor;

	// // Apply a transparency effect (80% opacity)
	// tint(255, 204);  // 255 for full color, 204 for 80% opacity (255 * 0.8)

	// // Display the scaled skeleton image in the center
	// image(skeletonImage, width / 2 - imgWidth / 2, height / 2 - imgHeight / 2, imgWidth, imgHeight);

	// // Reset tint for other elements
	// noTint();

	// // Game Over text
	// fill(255, 0, 0);
	// textSize(64);
	// textAlign(CENTER, CENTER);
	// text("Game Over", width / 2, height / 2 - 50);

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
	// stroke(192);
	// line(path[0][0], path[0][1], path[0][2], path[0][3]);
	// line(path[1][0], path[1][1], path[1][2], path[1][3]);
	// line(path[2][0], path[2][1], path[2][2], path[2][3]);

	// fill(0, 255, 0);  // Green start block
	// rect(100, 300, 50, 100);  // Start block
	// fill(0, 0, 255);  // Blue end block
	// rect(500, 300, 50, 100);  // End block

	// // Draw stick if picked up
	// if (stickPickedUp && shiftPressed && !gameOver) {
	//   stickX = mouseX;
	//   stickY = mouseY;
	// }

	// // The circle buzzer
	// fill(0, 255, 255, 127);  // Transparency for the circle
	// ellipse(stickX, stickY, 50, 50);

	// // Check if the stick is picked up
	// if (!stickPickedUp && dist(stickX, stickY, mouseX, mouseY) < 20) {
	//   stickPickedUp = true;
	// }

	// // Check if buzzer is hit (end block reached)
	// if (stickPickedUp && (stickX > 500 && stickY > 300)) {
	//   gameWon = true;
	//   gameStarted = false;
	// }

	// // Check if the game is over (off path)
	// if (!gameWon) {
	//   checkGameOver();  // Only check game over if the game hasn't been won yet
	// }

	// // Display game over message if off path
	// if (gameOver) {
	//   gameStarted = false;
	// }
}

int main()
{
	// Initialize GRRLIB and WiiMote
	GRRLIB_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	fontTexture = GRRLIB_LoadTexture(BMfont4_png);
	GRRLIB_InitTileSet(fontTexture, 16, 16, 32);

	// size(600, 400);  // Set window size
	// skeletonImage = loadImage("skeleton.png");  // Load the image
	// electrocuteSound = new SoundFile(this, "electrocute.wav");  // Load the electrocute sound
	// backgroundMusic = new SoundFile(this, "Chill.wav");  // Load the background music

	// // Start playing background music in a loop
	// backgroundMusic.loop();


	while (true)
	{
		// Read input
		WPAD_SetVRes(0, 0, 0);
		WPAD_ScanPads();
		const int buttonsDown = WPAD_ButtonsDown(0);
		const int buttonsHeld = WPAD_ButtonsHeld(0);
		WPAD_IR(WPAD_CHAN_0, &ir1);

		// Cursor
		mouseX = ir1.sx - 190;
		mouseY = ir1.sy - 210;

		// Clear the screen with black
		GRRLIB_FillScreen(GRRLIB_MAROON);

		// GRRLIB_Rectangle(100, 300, 50, 100, GRRLIB_MAROON, 1);

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