/*===========================================
        GRRLIB (GX Version)
        - Example Code -

        How To use Bitmap Fonts
============================================*/
#include <grrlib.h>

#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <fat.h>

#include "BMfont1_png.h"
#include "BMfont2_png.h"
#include "BMfont3_png.h"
#include "BMfont4_png.h"
#include "BMfont5_png.h"
#include "test_jpg_jpg.h"
#include "test_jpg_jpg.h"
#include "test_bmp_bmp.h"
#include "sprite_png.h"
#include "ocean_bmf.h"
#include "frontal_bmf.h"

#include "skeleton_jpg.h"

// Tile stuff
#define TILE_DELAY  10
#define TILE_UP     12 * 0
#define TILE_RIGHT  12 * 1
#define TILE_DOWN   12 * 2
#define TILE_LEFT   12 * 3
#define TILE_UP2    12 * 4 + 9
#define TILE_RIGHT2 12 * 5 + 9
#define TILE_DOWN2  12 * 6 + 9
#define TILE_LEFT2  12 * 7 + 9

// RGBA Colors
#define GRRLIB_BLACK   0x000000FF
#define GRRLIB_MAROON  0x800000FF
#define GRRLIB_GREEN   0x008000FF
#define GRRLIB_OLIVE   0x808000FF
#define GRRLIB_NAVY    0x000080FF
#define GRRLIB_PURPLE  0x800080FF
#define GRRLIB_TEAL    0x008080FF
#define GRRLIB_GRAY    0x808080FF
#define GRRLIB_SILVER  0xC0C0C0FF
#define GRRLIB_RED     0xFF0000FF
#define GRRLIB_LIME    0x00FF00FF
#define GRRLIB_YELLOW  0xFFFF00FF
#define GRRLIB_BLUE    0x0000FFFF
#define GRRLIB_FUCHSIA 0xFF00FFFF
#define GRRLIB_AQUA    0x00FFFFFF
#define GRRLIB_WHITE   0xFFFFFFFF

static u8 CalculateFrameRate(void);

int main() {
    s32 left = 0, top = 0, page = 0, frame = TILE_DOWN + 1;
    u32 wait = TILE_DELAY, direction = TILE_DOWN, direction_new = TILE_DOWN;
    u8 FPS = 0;

    ir_t ir1;
    guVector triangle[] = {{400,200,0.0f}, {500,400,0.0f}, {300,400,0.0f}};
    u32 trianglecolor[] = {GRRLIB_GREEN, GRRLIB_RED, GRRLIB_BLUE};

    GRRLIB_Init();

    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);


    GRRLIB_texImg *skeleton_img = GRRLIB_LoadTexture(skeleton_jpg);

    GRRLIB_texImg *tex_test_jpg = GRRLIB_LoadTexture(test_jpg_jpg);
    GRRLIB_texImg *tex_test_bmp = GRRLIB_LoadTexture(test_bmp_bmp);

    GRRLIB_bytemapFont *bmf_Font1 = GRRLIB_LoadBMF(ocean_bmf);
    GRRLIB_bytemapFont *bmf_Font2 = GRRLIB_LoadBMF(frontal_bmf);

    GRRLIB_texImg *tex_sprite_png = GRRLIB_LoadTexture(sprite_png);
    GRRLIB_InitTileSet(tex_sprite_png, 24, 32, 0);

    GRRLIB_texImg *tex_BMfont1 = GRRLIB_LoadTexture(BMfont1_png);
    GRRLIB_InitTileSet(tex_BMfont1, 32, 32, 32);

    GRRLIB_texImg *tex_BMfont2 = GRRLIB_LoadTexture(BMfont2_png);
    GRRLIB_InitTileSet(tex_BMfont2, 16, 16, 32);

    GRRLIB_texImg *tex_BMfont3 = GRRLIB_LoadTexture(BMfont3_png);
    GRRLIB_InitTileSet(tex_BMfont3, 32, 32, 32);

    GRRLIB_texImg *tex_BMfont4 = GRRLIB_LoadTexture(BMfont4_png);
    GRRLIB_InitTileSet(tex_BMfont4, 16, 16, 32);

    GRRLIB_texImg *tex_BMfont5 = GRRLIB_LoadTexture(BMfont5_png);
    GRRLIB_InitTileSet(tex_BMfont5, 8, 16, 0);


    
    
    // Stick
    bool stickPickedUp = false;
    int stickX = 125;
    int stickY = 300;
    
    bool buzzed = false;

    // Path 
    int path[3][2] = {
        { 125, 300, 125, 100 },
        { 125, 100, 525, 100 },

    };


    while(1) {
        WPAD_SetVRes(0, 0, 0);
        WPAD_ScanPads();
        const u32 wpaddown = WPAD_ButtonsDown(0);
        const u32 wpadheld = WPAD_ButtonsHeld(0);

        WPAD_IR(WPAD_CHAN_0, &ir1);

        GRRLIB_FillScreen(GRRLIB_BLACK);    // Clear the screen
        WPAD_Rumble(WPAD_CHAN_0, 0);


        // Cursor
        int x = ir1.sx - 190;
        int y = ir1.sy - 210;


        // Skeleton 
        GRRLIB_DrawImg(120, 100, skeleton_img, 0, .5, .5, GRRLIB_WHITE);  // Draw a jpeg



        GRRLIB_Printf(5, 25, tex_BMfont2, GRRLIB_WHITE, 1, "BUZZWIRE");


        // Start block
        GRRLIB_Rectangle(100, 300, 50, 100, GRRLIB_RED, 1);
        
        // Path
        GRRLIB_Line(125, 300, 125, 100, GRRLIB_SILVER);
        GRRLIB_Line(125, 100, 525, 100, GRRLIB_SILVER);
        GRRLIB_Line(525, 100, 525, 300, GRRLIB_SILVER);
        
        // End block
        GRRLIB_Rectangle(500, 300, 50, 100, GRRLIB_BLUE, 1);

        // Stick
        if (!stickPickedUp && abs(stickX - x) < 20 && abs(stickY - y) < 20) {
            stickPickedUp = true;
        }

        if (stickPickedUp) {
            stickX = x;
            stickY = y;
        }

        GRRLIB_Circle(stickX, stickY, 25, GRRLIB_AQUA, false);
        GRRLIB_Line(stickX + 35, stickY + 35, stickX + 150, stickY + 150, GRRLIB_AQUA);

        // Draw a yellow four pixel dot where the Wii Remote is pointing
        GRRLIB_Plot(x, y, GRRLIB_YELLOW);
        

        GRRLIB_Printf(500, 27, tex_BMfont5, GRRLIB_WHITE, 1, "Current FPSSSSS: %d", FPS);
        GRRLIB_Printf(500, 27*2, tex_BMfont5, GRRLIB_WHITE, 1, "Y: %d", y);
        GRRLIB_Printf(500, 27*3, tex_BMfont5, GRRLIB_WHITE, 1, "X: %d", x);

        if(wpaddown & WPAD_BUTTON_HOME) {
            break;
        }
        if(wpadheld & WPAD_BUTTON_LEFT) {
            if(wpadheld & WPAD_BUTTON_B || page == 1)
                left -= 2;
            else
                left--;
            direction_new = TILE_LEFT;  // for tile example
        }
        if(wpadheld & WPAD_BUTTON_RIGHT) {
            if(wpadheld & WPAD_BUTTON_B || page == 1)
                left += 2;
            else
                left++;
            direction_new = TILE_RIGHT;  // for tile example
        }
        if(wpadheld & WPAD_BUTTON_UP) {
            if(wpadheld & WPAD_BUTTON_B || page == 1)
                top -= 2;
            else
                top--;
            direction_new = TILE_UP;  // for tile example
        }
        if(wpadheld & WPAD_BUTTON_DOWN) {
            if(wpadheld & WPAD_BUTTON_B || page == 1)
                top += 2;
            else
                top++;
            direction_new = TILE_DOWN;  // for tile example
        }
        if(wpaddown & WPAD_BUTTON_MINUS) {
            page--;
            left = 0;
            top = 0;
            if(page < 0)
                page = 2;
        }
        if(wpaddown & WPAD_BUTTON_PLUS) {
            page++;
            left = 0;
            top = 0;
            if(page > 2)
                page = 0;
        }
        if(wpadheld & WPAD_BUTTON_1 && wpadheld & WPAD_BUTTON_2) {
            WPAD_Rumble(WPAD_CHAN_0, 1); // Rumble on
            GRRLIB_ScrShot("sd:/grrlib.png");
            WPAD_Rumble(WPAD_CHAN_0, 0); // Rumble off
        }

        GRRLIB_Render();
        FPS = CalculateFrameRate();
    }
    // Free some textures
    GRRLIB_FreeTexture(tex_test_jpg);
    GRRLIB_FreeTexture(tex_test_bmp);
    GRRLIB_FreeTexture(tex_sprite_png);
    GRRLIB_FreeTexture(tex_BMfont1);
    GRRLIB_FreeTexture(tex_BMfont2);
    GRRLIB_FreeTexture(tex_BMfont3);
    GRRLIB_FreeTexture(tex_BMfont4);
    GRRLIB_FreeTexture(tex_BMfont5);
    GRRLIB_FreeBMF(bmf_Font1);
    GRRLIB_FreeBMF(bmf_Font2);
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB
    return 0;
}

/**
 * This function calculates the number of frames we render each second.
 * @return The number of frames per second.
 */
static u8 CalculateFrameRate(void) {
    static u8 frameCount = 0;
    static u32 lastTime;
    static u8 FPS = 0;
    const u32 currentTime = ticks_to_millisecs(gettime());

    frameCount++;
    if(currentTime - lastTime > 1000) {
        lastTime = currentTime;
        FPS = frameCount;
        frameCount = 0;
    }
    return FPS;
}
