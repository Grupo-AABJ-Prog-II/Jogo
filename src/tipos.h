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
    TELA_VITORIA_FINAL,
    SALVAR,
    CARREGAR
} Tela;

typedef struct {
    int x;
    int y;
} Posicao;

typedef struct {
    Posicao pos;        
    Vector2 pixelPos;   
    Posicao dir;
    Posicao proxDir;
    Posicao spawnPos;
    double moveTimer;
    int score;
    int vidas;
} Pacman;

typedef struct {
    Posicao pos;
    Vector2 pixelPos;   
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
    
    int nivelAtual;
    bool jogoIniciado; 
} Mapa;

typedef struct {
    Texture2D parede;
    Texture2D pastilha;
    Texture2D superPastilha;
    Texture2D pacman;
    Texture2D fantasma;
    // Alterado para dois portais distintos
    Texture2D portal1; // Laranja
    Texture2D portal2; // Azul
} Sprites;

#endif