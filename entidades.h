#ifndef ENTIDADES_H
#define ENTIDADES_H

#include "raylib.h"
#include "mapa.h" // Importa as definições do seu mapa

// Configurações
#define QTD_FANTASMAS 4
#define TEMPO_VULNERAVEL 8.0f
#define VELOCIDADE_NORMAL 0.20f // Menor = Mais rápido (segundos por bloco)
#define VELOCIDADE_LENTA 0.30f  

// Estados do Jogo
typedef enum {
    ESTADO_JOGANDO,
    ESTADO_GAMEOVER,
    ESTADO_VITORIA
} GameState;

typedef struct {
    Posicao pos;        // Usando a struct Posicao do mapa.h
    Posicao dir;        // Direção atual (dx, dy)
    Posicao proxDir;    // Buffer de input
    Posicao spawnPos;   // Onde renasce
    float moveTimer;
    int score;
    int vidas;
} Pacman;

typedef struct {
    Posicao pos;
    Posicao dir;
    Posicao spawnPos;
    float moveTimer;
    float tempoVulneravel;
    int estaVulneravel; // 0 ou 1
    int estaAtivo;      // 0 = Morto (olhinhos voltando), 1 = Vivo
    int id;             // 0 a 3
    Color corOriginal;  // Para saber a cor dele quando não estiver azul
} Fantasma;

// --- Funções Principais ---

// Inicializa as entidades baseado nos dados carregados do mapa
void InicializarEntidades(Mapa *mapa, Pacman *pac, Fantasma *fantasmas);

// Atualiza a lógica (Movimento, Colisão, Pontuação)
// Retorna o estado atual do jogo (se morreu, se ganhou, etc)
GameState AtualizarJogo(Mapa *mapa, Pacman *pac, Fantasma *fantasmas, float deltaTime);

// Desenha as entidades SOBRE o mapa
void DesenharEntidades(Pacman *pac, Fantasma *fantasmas, Sprites *sprites, int tamanhoBloco);

#endif