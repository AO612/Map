#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// clang -o main main.c libraylib.a -framework IOKit -framework Cocoa -framework OpenGL
// ./main

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

typedef struct Province
{
    char name[15];
    Color colour;
    int ownerID;
    char ownerName[15];
} Province;

typedef enum Country
{
    SPAIN = 0,
    UK = 1,
    PORTUGAL = 2
} Country;

int main ()
{
    // Initialisation
    //--------------------------------------------------------------------------------------
    int windowedScreenWidth = 800;
    int windowedScreenHeight = 500;

    int screenWidth = windowedScreenWidth;
    int screenHeight = windowedScreenHeight;

    InitWindow(screenWidth, screenHeight, "Map");
    
    int provinceCount = 53;
    Province provinces[provinceCount];

    FILE* fp = fopen("information.csv", "r");
    
    if (!fp)
        printf("Can't open file\n");
 
    else {

        char buffer[1024];
 
        int row = 0;
        int column = 0;
 
        while (fgets(buffer, 1024, fp)) 
        { 
            column = 0;

            char* value = strtok(buffer, ",");

            provinces[row].colour.a = 255;
 
            while (value)
            {
                
                if (column == 0)
                {
                    strcpy(provinces[row].name, value);
                }
 
                if (column == 1)
                {
                    provinces[row].colour.r = atoi(value);
                }
 
                if (column == 2)
                {
                    provinces[row].colour.g = atoi(value);
                }

                if (column == 3)
                {
                    provinces[row].colour.b = atoi(value);
                }

                if (column == 4)
                {
                    provinces[row].ownerID = atoi(value);
                }

                if (column == 5)
                {
                    strcpy(provinces[row].ownerName, value);
                }
 
                value = strtok(NULL, ",");
                column++;
            }
            row++;
        }
 
        // Close the file
        fclose(fp);
    }

    // Country colours
    Color colours[3] = {{212, 182, 82, 255}, {177, 87, 73, 255}, {100, 137, 103, 255}};

    Image playerMap = LoadImage("spain.png");     // Loaded in CPU memory (RAM)

    for (int i = 0; i < provinceCount; i++) // // Create map with country colours for player
    {
        ImageColorReplace(&playerMap, provinces[i].colour, colours[provinces[i].ownerID]);
    }

    Texture2D spainTexture = LoadTextureFromImage(playerMap);          // Image converted to texture, GPU memory (VRAM) 
    
    Image valueMap = LoadImage("spain.png"); // Map with unique province colours

    Image highlightMap; // Map to draw province highlight on cursor hover

    Color colour;

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;
    camera.offset.x = screenWidth/2;
    camera.offset.y = screenHeight/2;
    camera.target.x = screenWidth/2;
    camera.target.y = screenHeight/2;

    SetTargetFPS(60);

    SetMousePosition(screenWidth/2,screenHeight/2);

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        if ( IsKeyPressed(KEY_F))
        {

            int display = GetCurrentMonitor();
            
            if (IsWindowFullscreen())
            {
                // If we are full screen, then go back to the windowed size
                screenWidth = windowedScreenWidth;
                screenHeight = windowedScreenHeight;
                SetWindowSize(screenWidth, screenHeight);
                
            }
            else
            {
                // If we are not full screen, set the window size to match the monitor we are on
                screenWidth = GetMonitorWidth(display);
                screenHeight = GetMonitorHeight(display);
                SetWindowSize(screenWidth, screenHeight);
            }
            // Toggle the state
            ToggleFullscreen();
            camera.offset.x = screenWidth/2;
            camera.offset.y = screenHeight/2;
            camera.target.x = screenWidth/2;
            camera.target.y = screenHeight/2;
        }
        
        // Pan by moving cursor closer to edge

        Vector2 delta = {0};
        Vector2 mousePosition = GetMousePosition();
        Vector2 mouseWorldPos =  GetScreenToWorld2D(GetMousePosition(), camera);

        int moveSpeed = 10;

        if (mousePosition.x < (screenWidth/10))
        {
            delta.x = moveSpeed;
        }
        else if (mousePosition.x > (screenWidth*9/10))
        {
            delta.x = -moveSpeed;
        }
        else
        {
            delta.x = 0;
        }

        if (mousePosition.y < (screenHeight/10))
        {
            delta.y = moveSpeed;
        }
        else if (mousePosition.y > (screenHeight*9/10))
        {
            delta.y = -moveSpeed;
        }
        else
        {
            delta.y = 0;
        }
        
        printf("get mouse position: %.6f %.6f\n", mousePosition.x, mousePosition.y);
        printf("delta: %.6f %.6f\n", delta.x, delta.y);
        
        delta = Vector2Scale(delta, -1.0f/camera.zoom);
        camera.target = Vector2Add(camera.target, delta);

        // Limit game borders

        camera.target.x = Clamp(camera.target.x, 0 , screenWidth);
        camera.target.y = Clamp(camera.target.y, 0 , screenHeight);
        
        printf("Camera offset x:%.6f\nCamera offset y:%.6f\nCamera target x:%.6f\nCamera target y:%.6f\nCamera zoom:%.6f\n", camera.offset.x, camera.offset.y, camera.target.x, camera.target.y, camera.zoom);

        float wheel = GetMouseWheelMove();

        if (wheel != 0)
        {

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point 
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0)
            {
                scaleFactor = 1.0f/scaleFactor;
            }
            camera.zoom = Clamp(camera.zoom*scaleFactor, 1.0f, 5.0f);
        }

        float getImageColorx = mouseWorldPos.x/screenWidth*valueMap.width;
        float getImageColory = mouseWorldPos.y/screenHeight*valueMap.height;

        colour = GetImageColor(valueMap, getImageColorx, getImageColory);

        // Find cursor position, map to image and retrieve province colour

        Image highlightMap = ImageCopy(valueMap);

        for (int i = 0; i < provinceCount; i++)
        {
            if (ColorIsEqual(colour, provinces[i].colour)) // Highlight province with transparent white
            {
                ImageColorReplace(&highlightMap, provinces[i].colour, (Color){255,255,255,60});
            }
            else // Remove everything else
            {
                ImageColorReplace(&highlightMap, provinces[i].colour, (Color){0,0,0,0});
            }
        }

        Texture2D highlightTexture = LoadTextureFromImage(highlightMap);


        // Draw

        BeginDrawing();

        ClearBackground((Color){88, 109, 139, 255});

        BeginMode2D(camera);

        Rectangle spainRectangle = {0,0,spainTexture.width,spainTexture.height};
        Vector2 spainCentre = {screenWidth/2, screenHeight/2};
        Rectangle screenRectangle = {screenWidth/2, screenHeight/2, screenWidth, screenHeight};
        
        DrawTexturePro(spainTexture, spainRectangle, screenRectangle, spainCentre, 0, WHITE); // Draw full map
        DrawText("+",camera.target.x, camera.target.y, 20, RED);
        DrawText("+",camera.offset.x, camera.offset.y, 20, RED);

        DrawTexturePro(highlightTexture, spainRectangle, screenRectangle, spainCentre, 0, WHITE); // Draw highlight

        EndMode2D();

        int rectangleHeight = screenHeight/6;
        int rectangleWidth = screenWidth/6;
        DrawRectangle(0, screenHeight-rectangleHeight, rectangleWidth, rectangleHeight, BEIGE);
        DrawText(TextFormat("%d, %d, %d", colour.r, colour.g, colour.b), 0, screenHeight-rectangleHeight, 20, WHITE);
        DrawCircle(screenWidth/12, screenHeight-rectangleHeight/3, 20, colour); // Information in bottom left

        for (int i = 0; i < provinceCount; i++)
        {
            if (ColorIsEqual(colour, provinces[i].colour))
            {
                DrawText(TextFormat("%s %d", provinces[i].name, provinces[i].ownerID), 0, screenHeight-rectangleHeight/2, 20, WHITE);
                break;
            }
        }

        UnloadImage(highlightMap);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    UnloadImage(playerMap);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM
    UnloadImage(valueMap);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}
