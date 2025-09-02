#include <chrono>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <optional>
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

typedef struct Boss {
    Vector2 position;
    Color color;
    int health;
} Boss;

typedef struct Option {
    Rectangle def;
    Rectangle rect;
    Color color;
    std::optional<const char *> name;
    std::optional<const char *> description;
} Option;

typedef struct Ability {
    std::string name;
    std::string description;
    std::function<void()> whatItDoes;
} Ability;

int random(int min, int max) {
    return GetRandomValue(min, max);
}

long long getUnixTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool ColorsEqual(Color c1, Color c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

int main() {
    InitWindow(800, 450, "Nameless game...");
    Vector2 rectangle[2] = {{0.00f, (float)GetScreenHeight() - 150}, {(float)GetScreenWidth(), 10}};
    Vector2 cube = {(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};
    std::vector<Option> choice1 = {
        {{90, 60, 220, 320}, {90, 60, 220, 320}, BLACK, std::nullopt, std::nullopt},
        {{100, 70, 200, 300}, {100, 70, 200, 300}, WHITE, "Template", "If you read this then\nbad happened\nRarity: LEGENDARY (this is a bug not an ability)"}};
    std::vector<Option> choice2 = {
        {{500, 60, 220, 320}, {500, 60, 220, 320}, BLACK, std::nullopt, std::nullopt},
        {{510, 70, 200, 300}, {510, 70, 200, 300}, WHITE, "Template", "If you read this then\nbad happened\nRarity: LEGENDARY (this is a bug not an ability)"}};

    Camera2D cam = {0};
    cam.target = cube;
    cam.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Boss> bosses;

    Vector2 coin = { 900, 900 };

    InitAudioDevice();
    Sound opsound = LoadSound("./resources/op.mp3");
    Sound shotsound = LoadSound("./resources/shot.mp3");
    Sound emptySound = LoadSound("./resources/empty.mp3");
    Sound lose = LoadSound("./resources/lose.mp3");
    Sound coinsound = LoadSound("./resources/coin.mp3");

    char lastkey = 'D';
    float velocity = 0;
    bool died = false;
    int enemyCount = 1;
    bool rllydied = false;
    bool shot = false;
    long long lastUse = 0;
    long long lastOpUse = 0;
    long long playedLoseSound = 0;
    int kills = 0;
    int opCooldown = 20000;
    bool isPaused = false;
    bool giveOptions = false;
    int jump = 0.4;
    bool isCursorDisabled = false;
    int randomValue1 = random(0, 2);
    int randomValue2 = random(0, 2);
    int shotsShot = 0;
    bool hasTriple = false;
    int chances = 0;
    bool automatic = false;

    std::vector<Ability> abilities = {
        {"Faster Cannon", "Reload your Cannon faster by 1sec\nRarity: Common", [&opCooldown]() { opCooldown -= 1000; }},
        {"Higher Jump", "Jump Higher\nOnly 28374km left until touching \nthe sky\nRarity: Common", [&jump]() { jump += 0.005; }},
        {"Tenth Shot", "Every tenth shot is a cannon\nRarity: RARE", [&hasTriple, &abilities]() { hasTriple = true; abilities.erase(abilities.begin() + 2); }},
        {"Extra chance", "Gain an extra life coin\nRarity: Uncommon", [&chances]() { chances++; }},
        {"Automatic Gun", "Shoots bullets faster\nRarity: RARE", [&automatic, &abilities]() { automatic = true; abilities.erase(abilities.begin() + 4);}},
    };

    Color colors[] = {YELLOW, GREEN, BLUE, PURPLE, DARKBLUE, DARKBROWN, DARKGREEN, PINK, BEIGE};
    while (!WindowShouldClose()) {
        if (WindowShouldClose()) break;
        if (!isCursorDisabled && (!isPaused && !giveOptions)) {
            DisableCursor();
            isCursorDisabled = true;
        } else if (isCursorDisabled && (isPaused || giveOptions)) {
            EnableCursor();
            isCursorDisabled = false;
        }
        if (shot) std::this_thread::sleep_for(std::chrono::milliseconds(300));
        shot = false;

        if (kills % 10 == 0 && kills != 0) {
            isPaused = true;
            giveOptions = true;
            choice1[1].name = abilities[randomValue1].name.c_str();
            choice1[1].description = abilities[randomValue1].description.c_str();
            choice2[1].name = abilities[randomValue2].name.c_str();
            choice2[1].description = abilities[randomValue2].description.c_str();
        }
        ClearBackground(RAYWHITE);
        cam.offset = (Vector2){cube.x, cube.y};
        rectangle[0] = {0.00f, (float)GetScreenHeight() - 150};
        rectangle[1] = {(float)GetScreenWidth(), 10};
        if (IsKeyDown(KEY_D) && !died && !isPaused) {
            cube.x += 0.15;
            lastkey = 'D';
        }
        for (Option &choice : choice1) {
            if (giveOptions) {
                if (CheckCollisionPointRec({GetMousePosition().x, GetMousePosition().y}, choice.rect)) {
                    choice.rect.height = choice.def.height + 30;
                    choice.rect.width = choice.def.width + 30;
                    choice.rect.x = choice.def.x - 15;
                    choice.rect.y = choice.def.y - 15;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        abilities[randomValue1].whatItDoes();
                        giveOptions = false;
                        kills++;
                        randomValue1 = random(0, abilities.size() - 1);
                        isPaused = false;
                        do {
                            randomValue2 = random(0, abilities.size() - 1);
                        } while (randomValue2 == randomValue1 && abilities.size() > 1);
                    }
                } else {
                    choice.rect = choice.def;
                }
            }
        }
        for (Option &choice : choice2) {
            if (giveOptions) {
                if (CheckCollisionPointRec({GetMousePosition().x, GetMousePosition().y}, choice.rect)) {
                    choice.rect.height = choice.def.height + 30;
                    choice.rect.width = choice.def.width + 30;
                    choice.rect.x = choice.def.x - 15;
                    choice.rect.y = choice.def.y - 15;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        abilities[randomValue2].whatItDoes();
                        giveOptions = false;
                        kills++;
                        isPaused = false;
                        randomValue1 = random(0, abilities.size() - 1);
                        do {
                            randomValue2 = random(0, abilities.size() - 1);
                        } while (randomValue2 == randomValue1 && abilities.size() > 1);
                    }
                } else {
                    choice.rect = choice.def;
                }
            }
        }
        if (IsKeyDown(KEY_A) && !died && !isPaused) {
            cube.x -= 0.15;
            lastkey = 'A';
        }
        if (rectangle[0].y - 30 <= cube.y) {
            velocity = 0;
            coin.y += 0.45;
            coin.x += 0.1;
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) && !died && !isPaused) velocity = 0.4;
            if (died && rllydied == false) velocity = 0.4;
            rllydied = true;
        } else {
            if (!isPaused) velocity -= 0.001;
        }
        if (cube.x <= -30) {
            cube.x = GetScreenWidth() + 30;
        } else if (cube.x >= GetScreenWidth() + 30) {
            cube.x = -30;
        }
        if (enemies.size() + (bosses.size() * 2) < enemyCount && !isPaused) {
            if (random(0, 1000) == 0) {
                Enemy enemy;
                float side = random(0, 1) % 2 == 0 ? -30 : (float)GetScreenWidth() + 30;
                enemy.position = {side, (float)GetScreenHeight() - 180};
                enemy.color = colors[random(0, 5)];
                enemy.velocity = 0;
                enemies.push_back(enemy);
            }
        }
        if (kills > 100 && !isPaused) {
            if (random(0, 1000) == 3) {
                Boss boss;
                float side = random(0, 1) % 2 == 0 ? -30 : (float)GetScreenWidth() + 30;
                boss.position = {side, (float)GetScreenHeight() - 230};
                boss.color = colors[random(0, 5)];
                boss.health = 30;
                bosses.push_back(boss);
            }
        }
        if (!automatic && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !died && !isPaused) {
            if (getUnixTimeMs() - lastUse > 500) {
                Bullet bullet;
                bullet.position = {cube.x + 15, cube.y + 15};
                bullet.direction = lastkey;
                bullet.time = getUnixTimeMs();
                bullet.op = hasTriple && shotsShot % 10 == 0 ? true : false;
                shot = hasTriple && shotsShot % 10 == 0 ? true : false;
                bullets.push_back(bullet);
                if (hasTriple && shotsShot % 10 == 0) PlaySound(opsound); else PlaySound(shotsound);
                lastUse = getUnixTimeMs();
                shotsShot++;
            }
        }
        if (automatic && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !died && !isPaused) {
            if (getUnixTimeMs() - lastUse > 100) {
                Bullet bullet;
                bullet.position = {cube.x + 15, cube.y + 15};
                bullet.direction = lastkey;
                bullet.time = getUnixTimeMs();
                bullet.op = hasTriple && shotsShot % 10 == 0 ? true : false;
                shot = hasTriple && shotsShot % 10 == 0 ? true : false;
                bullets.push_back(bullet);
                if (hasTriple && shotsShot % 10 == 0) PlaySound(opsound); else PlaySound(shotsound);
                lastUse = getUnixTimeMs();
                shotsShot++;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !died && !isPaused) {
            if (getUnixTimeMs() - lastOpUse > opCooldown) {
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
            if (isPaused) continue;
            Bullet &bullet = bullets[j];
            if (bullet.direction == 'D') bullet.position.x += 0.75;
            if (bullet.direction == 'A') bullet.position.x -= 0.75;
            if (bullet.op) bullet.position.y -= 0.001;
            int timeToCheck = bullet.op ? 2000 : 500;
            if (getUnixTimeMs() - bullet.time > timeToCheck) {
                bullets.erase(bullets.begin() + j);
            }
            if (bullet.position.x <= -10 && !automatic)
                bullet.position.x = GetScreenWidth() + 10;
            else if (bullet.position.x >= GetScreenWidth() + 10 && !automatic)
                bullet.position.x = -10;
            for (int i = 0; i < enemies.size(); i++) {
                Enemy enemy = enemies[i];
                if (fabs(bullet.position.x - enemy.position.x) < 15 && fabs(bullet.position.y - (enemy.position.y + 15)) < 15) {
                    enemies.erase(enemies.begin() + i);
                    kills++;
                    enemyCount = automatic ? random(1, 100) : random(1, 30);
                    if (!bullet.op) bullets.erase(bullets.begin() + j);
                }
            }
            for (int i = 0; i < bosses.size(); i++) {
                Boss &enemy = bosses[i];
                if (fabs(bullet.position.x - enemy.position.x) < 70 && fabs(bullet.position.y - (enemy.position.y + 40)) < 70) {
                    if (enemy.health <= 0) {
                        bosses.erase(bosses.begin() + i);
                        kills++;
                    } else {
                        enemy.health--;
                    }
                    enemyCount = automatic ? random(1, 100) : random(1, 30);
                    bullets.erase(bullets.begin() + j);
                }
            }
        }
        for (Boss &enemy : bosses) {
            if (isPaused) continue;
            if (cube.x <= enemy.position.x) {
                enemy.position.x -= 0.05;
            } else {
                enemy.position.x += 0.05;
            }
            if (fabs(enemy.position.x - cube.x) < 15 && fabs(enemy.position.y - cube.y) < 15) {
                if (chances <= 0) {
                    died = true;
                } else {
                    chances--;
                    cube.x = GetScreenWidth()/2;
                    cube.y = -30;
                    coin = cube;
                    PlaySound(coinsound);
                }
            }
        }
        for (Enemy &enemy : enemies) {
            if (isPaused) continue;
            if (cube.x <= enemy.position.x) {
                enemy.position.x -= 0.05;
            } else {
                enemy.position.x += 0.05;
            }
            if (rectangle[0].y - 30 <= enemy.position.y) {
                enemy.velocity = 0;
                if (random(0, 1000) == 1) {
                    enemy.velocity = 0.3;
                }
            } else {
                enemy.velocity -= 0.001;
            }
            enemy.position.y -= enemy.velocity;
            if (fabs(enemy.position.x - cube.x) < 15 && fabs(enemy.position.y - cube.y) < 15) {
                if (chances <= 0) {
                    died = true;
                } else {
                    chances--;
                    cube.x = GetScreenWidth()/2;
                    cube.y = -30;
                    coin = cube;
                    PlaySound(coinsound);
                }
            }
        }
        if (IsKeyPressed(KEY_P)) isPaused = !isPaused;
        if (!isPaused) cube.y -= velocity;
        BeginDrawing();
        {
            if (shot) DrawRectangleV({0, 0}, {1000, 1000}, RED);
            else DrawRectangleV(rectangle[0], rectangle[1], RED);
            for (Enemy enemy : enemies) {
                DrawRectangleV(enemy.position, {30, 30}, enemy.color);
            }
            for (Boss enemy : bosses) {
                DrawRectangleV(enemy.position, {80, 80}, enemy.color);
                std::string health = "Health: " + std::to_string(enemy.health) + "/30";
                DrawText(health.c_str(), enemy.position.x, enemy.position.y - 40, 10, GRAY);
            }
            if (!died) {
                DrawRectangleV({cube.x + (float)random(-1, enemyCount / 2), cube.y + (float)random(0, 1)}, {30, 30}, GRAY);
                int direction = lastkey == 'D' ? 20 : -5;
                Color gunColor = shot ? ORANGE : BLACK;
                DrawRectangleV({cube.x + direction + (float)random(-1, enemyCount / 1.5), cube.y + 13 + (float)random(0, 1)}, {15, 10}, gunColor);
            } else {
                for (int i = 0; i < 10; i++) {
                    DrawRectangleV({cube.x + random(1, 5), cube.y + 15}, {30, 15}, GRAY);
                }
            }
            for (Bullet bullet : bullets) {
                if (bullet.op)
                    DrawRectangleV(bullet.position, {15, 10}, BLUE);
                else
                    DrawRectangleV(bullet.position, {10, 4}, BLUE);
            }
            DrawRectangleV(coin, {10, 10}, YELLOW);
            if (died) {
                DrawText("You died, click space to restart or ESC to close game", 100, 100, 20, BLACK);
                if (getUnixTimeMs() - playedLoseSound > 8000) {
                    PlaySound(lose);
                    playedLoseSound = getUnixTimeMs();
                }
                if (IsKeyPressed(KEY_SPACE)) {
                    UnloadSound(coinsound);
                    UnloadSound(opsound);
                    UnloadSound(shotsound);
                    UnloadSound(emptySound);
                    UnloadSound(lose);
                    CloseAudioDevice();
                    CloseWindow();
                    main();
                }
            }
            for (Option choice : choice1) {
                if (giveOptions) {
                    DrawRectangle(choice.rect.x, choice.rect.y, choice.rect.width, choice.rect.height, choice.color);
                    if (choice.name) {
                        DrawText(choice.name.value(), choice.rect.x + 15, choice.rect.y + 10, 20, BLACK);
                        DrawText(choice.description.value(), choice.rect.x + 15, choice.rect.y + 30, 10, GRAY);
                    }
                }
            }
            for (Option choice : choice2) {
                if (giveOptions) {
                    DrawRectangle(choice.rect.x, choice.rect.y, choice.rect.width, choice.rect.height, choice.color);
                    if (choice.name) {
                        DrawText(choice.name.value(), choice.rect.x + 15, choice.rect.y + 10, 20, BLACK);
                        DrawText(choice.description.value(), choice.rect.x + 15, choice.rect.y + 30, 10, GRAY);
                    }
                }
            }
            if (isPaused) {
                DrawText("Game paused", 360, 75, 10, GRAY);
            }
            std::string text = "Kills: " + std::to_string(kills);
            DrawText(text.c_str(), 0, 0, 20, BLACK);
            std::string xyz = "Click P to pause game, WAD to move,\nLMB to shoot, RMB to shoot cannon";
            DrawText(xyz.c_str(), 0, 20, 10, BLACK);
        }
        EndDrawing();
    }
    UnloadSound(coinsound);
    UnloadSound(opsound);
    UnloadSound(shotsound);
    UnloadSound(emptySound);
    UnloadSound(lose);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}