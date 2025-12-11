#ifndef TIPOS_H
#define TIPOS_H

#include <raylib.h>
#include <stdbool.h>

typedef enum Tela {
    TELA_SAIR,
    TELA_MENU_PRINCIPAL,
    TELA_RESOLUCAO_MENU_PRINCIPAL,
    TELA_MENU,
    TELA_JOGO,
    TELA_GAMEOVER,
    TELA_VITORIA,
    TELA_VITORIA_FINAL
} Tela;

typedef struct {
    int x;
    int y;
} Posicao;

typedef struct {
    Posicao pos;        // Posição Lógica (Grade)
    Vector2 pixelPos;   // Posição Visual (Pixels)
    Posicao dir;
    Posicao proxDir;
    Posicao spawnPos;
    double moveTimer;
    int score;
    int vidas;
} Pacman;

typedef struct {
    Posicao pos;
    Vector2 pixelPos;   // Posição Visual
    Posicao spawnPosOriginal;
    double moveTimer;
    double tempoVulneravel;
    bool estaVulneravel;
} Fantasma;

typedef struct {
    char **grade;
    int linhas;
    int colunas;
    
    Posicao *portais;
    int *conexoes;
    int qtdPortais;

    Pacman pacman;
    Fantasma *fantasmas;
    int numero_fantasmas;
    
    Posicao spawnFantasma; 
    int temSpawn;
    int qtdPointPellets;
    
    // --- Controle de Estado ---
    int nivelAtual;
    bool jogoIniciado; // True se o player já moveu
} Mapa;

typedef struct {
    Texture2D parede;
    Texture2D pastilha;
    Texture2D superPastilha;
    Texture2D pacman;
    Texture2D fantasma;
    Texture2D portal;
} Sprites;

#endif