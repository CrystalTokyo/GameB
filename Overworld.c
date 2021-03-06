// Filename: Overworld.c
// Contains code for the functions that are specific to the overworld game state.
//
// Project Codename: GameB
// TODO: Come up with a better name later.
// 2020 Joseph Ryan Ries <ryanries09@gmail.com>
// My YouTube series where we program an entire video game from scratch in C.
// Watch it on YouTube:    https://www.youtube.com/watch?v=3zFFrBSdBvA
// Follow along on GitHub: https://github.com/ryanries/GameB
// Find me on Twitter @JosephRyanRies 
// # License
// ----------
// The source code in this project is licensed under the MIT license.
// The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
// A copy of that license can be found in the 'Assets' directory.
// stb_vorbis by Sean Barrett is public domain and a copy of its license can be found in the stb_vorbis.c file.

#include "Main.h"

#include "Overworld.h"

void DrawOverworld(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;

    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        memset(&TextColor, 0, sizeof(PIXEL32));
    }

    if (LocalFrameCounter == 60)
    {
        PlayGameMusic(&gMusicOverworld01);
    }

    BlitBackgroundToBuffer(&gOverworld01.GameBitmap);

    // TODO: if debug turned on, draw only the tile numbers adjacent to the player
    //for (uint16_t Row = 0; Row < GAME_RES_HEIGHT / 16; Row++)
    //{
    //    for (uint16_t Column = 0; Column < GAME_RES_WIDTH / 16; Column++)
    //    {
    //        char Buffer[8] = { 0 };

    //        _itoa_s(gOverworld01.TileMap.Map[Row][Column], Buffer, sizeof(Buffer), 10);

    //        BlitStringToBuffer(Buffer, &g6x7Font, &(PIXEL32) { 0xFF, 0xFF, 0xFF, 0xFF }, (Column * 16) + 5, (Row * 16) + 4);
    //        
    //    }
    //}

    Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.SpriteIndex + gPlayer.Direction],
        gPlayer.ScreenPos.x,
        gPlayer.ScreenPos.y);


    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

// Process Player Input for the Overworld game state.
// This typically involves the player moving around on the screen.
// TODO: Make a gCurrentMap so that this works for any map we if choose to swap maps.
void PPI_Overworld(void)
{
    // TODO remove this - it is just for debugging
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    // If the player has no movement remaining, it means the player is standing still.    

    if (!gPlayer.MovementRemaining)
    {
        // If the player wants to move downward, we need to consult the tilemap to see
        // if the destination tile can be stepped on - e.g., is it grass or is it water?
        // We do this for all four directions - just check the adjacent tile to see if it's passable
        // before we allow the player to move there. Careful about index out of bounds errors if the
        // player is near the edge of the map.
        
        if (gGameInput.DownKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
            {
                if (gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) + 1][gPlayer.WorldPos.x / 16] == gPassableTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 16)
                {
                    gPlayer.Direction = DOWN;

                    gPlayer.MovementRemaining = 16;
                }
            }         

        }
        else if (gGameInput.LeftKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
            {
                if (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) - 1] == gPassableTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.x > 0)
                {
                    gPlayer.Direction = LEFT;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
            {
                if (gOverworld01.TileMap.Map[gPlayer.WorldPos.y / 16][(gPlayer.WorldPos.x / 16) + 1] == gPassableTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 16)
                {
                    gPlayer.Direction = RIGHT;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
        else if (gGameInput.UpKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            if (gPlayer.ScreenPos.y > 0)
            {
                for (uint8_t Counter = 0; Counter < _countof(gPassableTiles); Counter++)
                {
                    if (gOverworld01.TileMap.Map[(gPlayer.WorldPos.y / 16) - 1][gPlayer.WorldPos.x / 16] == gPassableTiles[Counter])
                    {
                        CanMoveToDesiredTile = TRUE;

                        break;
                    }
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.y > 0)
                {
                    gPlayer.Direction = UP;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
    }
    else 
    {       
        // gPlayer.MovementRemaining was greater than 0, which means the player is currently in motion.
        // The player must move exactly 16 pixels (1 tile) to complete a full movement. You cannot 
        // cancel a movement in progress or change directions during the middle of a movement.
        // If player is near the center of the screen, then adjust only player's screen position.
        // If player is near the edge of the screen, then pan the camera instead of changing player's screen position.
        // (Unless player is near the edge of the map, thus the camera cannot pan any further.)

        gPlayer.MovementRemaining--;

        if (gPlayer.Direction == DOWN)
        {
            if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 64)
            {
                gPlayer.ScreenPos.y++;
            }
            else
            {
                gCamera.y++;
            }

            gPlayer.WorldPos.y++;
        }
        else if (gPlayer.Direction == LEFT)
        {
            if (gPlayer.ScreenPos.x > 64)
            {
                gPlayer.ScreenPos.x--;
            }
            else
            {
                if (gCamera.x > 0)
                {
                    gCamera.x--;
                }
                else
                {
                    gPlayer.ScreenPos.x--;
                }
            }

            gPlayer.WorldPos.x--;
        }
        else if (gPlayer.Direction == RIGHT)
        {
            if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 64)
            {
                gPlayer.ScreenPos.x++;
            }
            else
            {
                gCamera.x++;
            }

            gPlayer.WorldPos.x++;
        }
        else if (gPlayer.Direction == UP)
        {
            if (gPlayer.ScreenPos.y > 64)
            {
                gPlayer.ScreenPos.y--;
            }
            else
            {
                if (gCamera.y > 0)
                {
                    gCamera.y--;
                }
                else
                {
                    gPlayer.ScreenPos.y--;
                }
            }

            gPlayer.WorldPos.y--;
        }

        // During the course of the player's 16 pixel motion, we are changing player's sprite index
        // 4 times. One foot forward, neutral, other foot forward, neutral. This achieves a walking
        // animation effect as the player is moving. You want to end on neutral/standing still.

        switch (gPlayer.MovementRemaining)
        {
            case 16:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            case 12:
            {
                gPlayer.SpriteIndex = 1;

                break;
            }
            case 8:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            case 4:
            {
                gPlayer.SpriteIndex = 2;

                break;
            }
            case 0:
            {
                gPlayer.SpriteIndex = 0;

                break;
            }
            default:
            {

            }
        }
    }
}