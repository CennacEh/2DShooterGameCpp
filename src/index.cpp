#include <iostream>
#include <string>
#include <vector>

#include "raylib.h"

typedef struct Bullet {
    Vector2 position;
    char direction;
} Bullet;

int main(int argc, char* argv[]) {
    InitWindow(800, 450, "Window");
    Vector2 rectangle[2] = { { 0.00f, (float)GetScreenHeight() - 150 }, { (float)GetScreenWidth(), 10 } };
    Vector2 cube = { (float)GetScreenWidth()/2, (float)GetScreenHeight()/2 };
    
    Camera2D cam = { 0 };
    cam.target = cube;
    cam.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    std::vector<Bullet> bullets;

    char lastkey = 'D';
    float velocity = 0;
    while (!WindowShouldClose()) {
        ClearBackground(RAYWHITE);
        cam.offset = (Vector2){ cube.x, cube.y };
        rectangle[0] = { 0.00f, (float)GetScreenHeight() - 150 };
        rectangle[1] = { (float)GetScreenWidth(), 10 };
        if (IsKeyDown(KEY_D)) {cube.x += 0.15; lastkey = 'D';}
        if (IsKeyDown(KEY_A)) {cube.x -= 0.15; lastkey = 'A';}
        if (rectangle[0].y - 30 <= cube.y) {
            velocity = 0;
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W)) velocity = 0.25;
        } else {
            velocity -= 0.001;
        }
        if (cube.x <= -30) {
            cube.x = GetScreenWidth() + 30;
        } else if (cube.x >= GetScreenWidth() + 30) {
            cube.x = -30;
        }
        if (IsKeyPressed(KEY_I)) {
            Bullet bullet;
            bullet.position = { cube.x + 15, cube.y + 15};
            bullet.direction = lastkey;
            bullets.push_back(bullet);
        }
        for (Bullet &bullet : bullets) {
            if (bullet.direction == 'D') bullet.position.x += 0.75;
            if (bullet.direction == 'A') bullet.position.x -= 0.75;
        }
        cube.y -= velocity;
        BeginDrawing();
        {
            DrawRectangleV(rectangle[0], rectangle[1], RED);
            DrawRectangleV(cube, { 30, 30 }, ORANGE);
            for (Bullet bullet : bullets) {
                DrawRectangleV(bullet.position, { 10, 4 }, BLUE);
            }
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}