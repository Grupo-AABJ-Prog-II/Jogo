#ifndef TIPOS_H
#define TIPOS_H

#include <raylib.h>

typedef enum Tela {
    // Fecha o jogo
    TELA_SAIR,

    TELA_MENU_PRINCIPAL,
    TELA_RESOLUCAO_MENU_PRINCIPAL,

    TELA_MENU,

    TELA_JOGO
} Tela;

typedef struct {
    int x;
    int y;
} Posicao;

typedef struct {
    Posicao pos;        // Usando a struct Posicao do mapa.h
    Posicao dir;        // Direção atual (dx, dy)
    Posicao proxDir;    // Buffer de input
    Posicao spawnPos;   // Onde renasce
    double moveTimer;
    int score;
    int vidas;
} Pacman;

typedef struct {
    Posicao pos;
    double moveTimer;
    double tempoVulneravel;
    bool estaVulneravel;
} Fantasma;


typedef struct {
    char **grade;// Matriz dinâmica
    int linhas;
    int colunas;
    Posicao *portais; // Vetor dinâmico
    int *conexoes; // Vetor dinâmico de IDs
    int qtdPortais;

    Pacman pacman;
    Fantasma *fantasmas;
    int numero_fantasmas;
    
    // Estatísticas
    int qtdPointPellets;
    int totalPellets;// Total de itens para comer (. + o + +)
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
