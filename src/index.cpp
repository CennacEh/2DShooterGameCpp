#include <chrono>
#include <cmath>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "raylib.h"

typedef struct Bullet {
    Vector2 position;
    char direction;
    long long time;
    bool op;
} Bullet;

typedef struct Enemy {
    Vector2 position;
    Color color;
    float velocity;
} Enemy;

int random(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

long long getUnixTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

int main(int argc, char *argv[]) {
    InitWindow(800, 450, "Window");
    Vector2 rectangle[2] = {{0.00f, (float)GetScreenHeight() - 150}, {(float)GetScreenWidth(), 10}};
    Vector2 cube = {(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};

    Camera2D cam = {0};
    cam.target = cube;
    cam.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    InitAudioDevice();
    Sound opsound = LoadSound("./resources/op.mp3");
    Sound shotsound = LoadSound("./resources/shot.mp3");
    Sound emptySound = LoadSound("./resources/empty.mp3");

    char lastkey = 'D';
    float velocity = 0;
    bool died = false;
    int enemyCount = 1;
    bool rllydied = false;
    bool shot = false;
    long long lastUse = 0;
    long long lastOpUse = 0;
    Color colors[] = {YELLOW, GREEN, BLUE, PURPLE, DARKBLUE, DARKBROWN, DARKGREEN, PINK, BEIGE};
    while (!WindowShouldClose()) {
        if (shot) std::this_thread::sleep_for(std::chrono::milliseconds(300));
        shot = false;
        ClearBackground(RAYWHITE);
        cam.offset = (Vector2){cube.x, cube.y};
        rectangle[0] = {0.00f, (float)GetScreenHeight() - 150};
        rectangle[1] = {(float)GetScreenWidth(), 10};
        if (IsKeyDown(KEY_D) && !died) {
            cube.x += 0.15;
            lastkey = 'D';
        }
        if (IsKeyDown(KEY_A) && !died) {
            cube.x -= 0.15;
            lastkey = 'A';
        }
        if (rectangle[0].y - 30 <= cube.y) {
            velocity = 0;
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) && !died) velocity = 0.4;
            if (died && rllydied == false) velocity = 0.4;
            rllydied = true;
        } else {
            velocity -= 0.001;
        }
        if (cube.x <= -30) {
            cube.x = GetScreenWidth() + 30;
        } else if (cube.x >= GetScreenWidth() + 30) {
            cube.x = -30;
        }
        if (enemies.size() < enemyCount) {
            if (random(0, 1000) == 0) {
                Enemy enemy;
                float side = random(0, 1) % 2 == 0 ? -30 : (float)GetScreenWidth() + 30;
                enemy.position = {side, (float)GetScreenHeight() - 180};
                enemy.color = colors[random(0, 5)];
                enemy.velocity = 0;
                enemies.push_back(enemy);
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (getUnixTimeMs() - lastUse > 500) {
                Bullet bullet;
                bullet.position = {cube.x + 15, cube.y + 15};
                bullet.direction = lastkey;
                bullet.time = getUnixTimeMs();
                bullet.op = false;
                bullets.push_back(bullet);
                // shot = true;
                PlaySound(shotsound);
                lastUse = getUnixTimeMs();
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (getUnixTimeMs() - lastOpUse > 5000) {
                Bullet bullet;
                bullet.position = {cube.x + 15, cube.y + 15};
                bullet.direction = lastkey;
                bullet.time = getUnixTimeMs();
                bullet.op = true;
                bullets.push_back(bullet);
                shot = true;
                PlaySound(opsound);
                lastOpUse = getUnixTimeMs();
            } else {
                PlaySound(emptySound);
            }
        }
        for (int j = 0; j < bullets.size(); j++) {
            Bullet &bullet = bullets[j];
            if (bullet.direction == 'D') bullet.position.x += 0.75;
            if (bullet.direction == 'A') bullet.position.x -= 0.75;
            if (bullet.op) bullet.position.y -= 0.001;
            int timeToCheck = bullet.op ? 100000000000000 : 500;
            if (getUnixTimeMs() - bullet.time > timeToCheck) {
                bullets.erase(bullets.begin() + j);
            }
            if (bullet.position.x <= -10)
                bullet.position.x = GetScreenWidth() + 10;
            else if (bullet.position.x >= GetScreenWidth() + 10)
                bullet.position.x = -10;
            for (int i = 0; i < enemies.size(); i++) {
                Enemy enemy = enemies[i];
                if (fabs(bullet.position.x - enemy.position.x) < 15 && fabs(bullet.position.y - (enemy.position.y + 15)) < 15) {
                    enemies.erase(enemies.begin() + i);
                    enemyCount++;
                    bullets.erase(bullets.begin() + j);
                }
            }
        }
        for (Enemy &enemy : enemies) {
            if (cube.x <= enemy.position.x) {
                enemy.position.x -= 0.05;
            } else {
                enemy.position.x += 0.05;
            }
            if (rectangle[0].y - 30 <= enemy.position.y) {
                enemy.velocity = 0;
                if (random(0, 500) == 1) {
                    enemy.velocity = 0.4;
                }
            } else {
                enemy.velocity -= 0.001;
            }
            enemy.position.y -= enemy.velocity;
            if (fabs(enemy.position.x - cube.x) < 15 && fabs(enemy.position.y - cube.y) < 15) {
                died = true;
            }
        }
        cube.y -= velocity;
        BeginDrawing();
        {
            if (shot)
                DrawRectangleV({0, 0}, {1000, 1000}, RED);
            else
                DrawRectangleV(rectangle[0], rectangle[1], RED);
            for (Enemy enemy : enemies) {
                DrawRectangleV(enemy.position, {30, 30}, enemy.color);
            }
            if (!died) {
                DrawRectangleV({cube.x + (float)random(-1, enemyCount / 2), cube.y + (float)random(0, 1)}, {30, 30}, GRAY);
                int direction = lastkey == 'D' ? 20 : -5;
                Color gunColor = shot ? ORANGE : BLACK;
                DrawRectangleV({cube.x + direction + (float)random(-1, enemyCount / 1.5), cube.y + 13 + (float)random(0, 1)}, {15, 10}, gunColor);
            } else {
                for (int i = 0; i < 10; i++) {
                    DrawRectangleV({cube.x + (float)random(-10, 10), cube.y + 15}, {30, 15}, GRAY);
                }
            }
            for (Bullet bullet : bullets) {
                if (bullet.op)
                    DrawRectangleV(bullet.position, {15, 10}, BLUE);
                else
                    DrawRectangleV(bullet.position, {10, 4}, BLUE);
            }
            if (died) {
                DrawText("You died (close the app and reopen to restart)", 100, 100, 20, BLACK);
            }
        }
        EndDrawing();
    }
    UnloadSound(opsound);
    UnloadSound(shotsound);
    UnloadSound(emptySound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}