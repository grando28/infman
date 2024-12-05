#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>

#define G 400.0f
#define PLAYER_HOR_SPD 350.0f
#define PLAYER_JUMP_SPD 200.0f
#define TILE_SIZE 16
#define MAP_WIDTH 50
#define MAP_HEIGHT 10
#define PLAYER_SIZE 24


typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
    int points;
    int lifes;
    bool direcao;
} PLAYER;


void UpdatePlayer(PLAYER *player, char map[MAP_HEIGHT][MAP_WIDTH], float delta);
void LoadMap(const char* filename, char map[MAP_HEIGHT][MAP_WIDTH]);
void DrawMap(char map[MAP_HEIGHT][MAP_WIDTH], Texture2D tileTexture, Texture2D playerTexture, PLAYER *player);

int main(void) {
    const int screenWidth = 1200;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "os horrores persistem, mas tambem devo eu");

    
    // sprite
    Texture2D tileTexture = LoadTexture("tile.png");
    Texture2D playerTexture = LoadTexture("player.png");
    Texture2D player2Texture = LoadTexture("player2.png");
    Texture2D playerAirTexture = LoadTexture("playerair.png");
    Texture2D player2AirTexture = LoadTexture("player2air.png");
    //Texture2D spikeTexture = LoadTexture("spike.png");
    
   
    PLAYER player = { 0 };

    Camera2D camera = { 0 };
    camera.target = (Vector2){player.position.x, player.position.y};
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;

    char map[MAP_HEIGHT][MAP_WIDTH] = { 0 };
    LoadMap("map.txt", map);
    
    // pos inicial do jogador
     for (int y = 0; y < MAP_HEIGHT; y++) {
         for (int x = 0; x < MAP_WIDTH; x++) {
             if (map[y][x] == 'P') {
                 player.position = (Vector2){ x * TILE_SIZE, y * TILE_SIZE };
                 break;
             }
         }
     }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Atualizar jogador
        UpdatePlayer(&player, map, deltaTime);
        
        camera.target = (Vector2){player.position.x, player.position.y-32};

        BeginDrawing();
        
        ClearBackground(RAYWHITE);
        
        BeginMode2D(camera);
        
        DrawMap(map, tileTexture, playerTexture, &player); 
        
        if(player.direcao == false){
            if(player.canJump == true) DrawTexture(playerTexture, player.position.x, player.position.y-8, WHITE);
            else DrawTexture(playerAirTexture, player.position.x, player.position.y-8, WHITE);
        }
        else{
            if(player.canJump == true) DrawTexture(player2Texture, player.position.x, player.position.y-8, WHITE);
            else DrawTexture(player2AirTexture, player.position.x, player.position.y-8, WHITE);
        }
        EndMode2D();
        
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void UpdatePlayer(PLAYER *player, char map[MAP_HEIGHT][MAP_WIDTH], float delta) {
    Vector2 oldPosition = player->position; // Armazenar posição anterior

    // mov
    if (player->canJump) { // no chao
        if (IsKeyDown(KEY_LEFT)){
            player->position.x -= PLAYER_HOR_SPD * delta;
            player->direcao = true;
        }
        if (IsKeyDown(KEY_RIGHT)){
            player->position.x += PLAYER_HOR_SPD * delta;
            player->direcao = false;
        }
    } else { // no ar
        if (IsKeyDown(KEY_LEFT)){
            player->position.x -= (PLAYER_HOR_SPD * 0.5f) * delta; // 50% da velocidade
            player->direcao = true;
        }
        if (IsKeyDown(KEY_RIGHT)){
            player->position.x += (PLAYER_HOR_SPD * 0.5f) * delta;
            player->direcao = false;
        }
    }

    // pular
    if (IsKeyDown(KEY_SPACE) && player->canJump) {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    // gravidade
    player->position.y += player->speed * delta;
    player->speed += G * delta;

    // colisoes
    int tileX = (int)((player->position.x + 1) / TILE_SIZE);
    int tileY = (int)((player->position.y + TILE_SIZE - 1) / TILE_SIZE);
    int tileXRight = (int)((player->position.x + PLAYER_SIZE - 3) / TILE_SIZE);  // Posição direita
    int tileYBottom = (int)((player->position.y + 10) / TILE_SIZE); // Posição inferior
    int tileXMiddle = (int)((player->position.x + 12) / TILE_SIZE);

    
    
    if (map[tileY][tileX] == 'B' || map[tileY][tileXRight] == 'B' || map[tileY][tileXMiddle] == 'B') {
        player->position.y = oldPosition.y;
        player->speed = 0;
        player->canJump = true;
    } else {
        //player->canJump = false;
    }
    
    if ((map[tileY][tileXRight] == 'B') && player->position.x > oldPosition.x) {
        player->position.x = oldPosition.x; // Reverter movimento horizontal (direita)
    }

    if ((map[tileY][tileX] == 'B') && player->position.x < oldPosition.x) {
        player->position.x = oldPosition.x; // Reverter movimento horizontal (esquerda)
    }
    
    if (map[tileYBottom][tileXMiddle] == 'O') {
        // Spike death
        //CloseWindow();
        DrawText("MOREEU", 300, 200, 40, RED);
        
    }
    
}

void LoadMap(const char* filename, char map[MAP_HEIGHT][MAP_WIDTH]) {
    FILE* file = fopen(filename, "r");
    

    // Lendo o arquivo linha por linha
    char line[MAP_WIDTH + 2];  // +2 para acomodar o '\n' e '\0'
    for (int y = 0; y < MAP_HEIGHT; y++) {
        if (fgets(line, sizeof(line), file)) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                // Garantir que o caractere seja válido para o mapa
                if (line[x] == 'B' || line[x] == 'P' || line[x] == 'O' || line[x] == ' ') {
                    map[y][x] = line[x];
                } else {
                    map[y][x] = ' '; // Caso algum caractere inesperado apareça
                }
            }
        } else {
            // Caso haja menos linhas do que o esperado no arquivo, preencher com espaços
            for (int x = 0; x < MAP_WIDTH; x++) {
                map[y][x] = ' ';
            }
        }
    }

    fclose(file);
}



void DrawMap(char map[MAP_HEIGHT][MAP_WIDTH], Texture2D tileTexture, Texture2D playerTexture, PLAYER *player) {
    Texture2D spikeTexture = LoadTexture("spike.png");
 for (int y = 0; y < MAP_HEIGHT; y++) {
     for (int x = 0; x < MAP_WIDTH; x++) {
         char tile = map[y][x];
         if (tile == 'B') {
             DrawTexture(tileTexture, x * TILE_SIZE, y * TILE_SIZE, WHITE);
         } else if (tile == 'O') {
             DrawTexture(spikeTexture, x * TILE_SIZE, y * TILE_SIZE, WHITE);
         } else if (tile == 'P') {
             DrawTexture(playerTexture, player->position.x, player->position.y, BLANK);
         }
     }
 }
}

