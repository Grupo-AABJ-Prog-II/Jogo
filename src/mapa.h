#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"

// Estruturas
typedef struct {
    int x;
    int y;
} Posicao;

typedef struct {
    char **grade;// Matriz dinâmica
    int linhas;
    int colunas;
    Posicao inicioPacman;
    Posicao spawnFantasma;
    int temSpawn;
    Posicao *portais; // Vetor dinâmico
    int *conexoes; // Vetor dinâmico de IDs
    int qtdPortais;
    
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

//mapa no geral
Mapa* CarregarMapa(const char *nomeArquivo);

// Limpa memória (importante!)
void LiberarMapa(Mapa *mapa);

// Carrega as imagens
void CarregarSprites(Sprites *sprites);
void DescarregarSprites(Sprites *sprites);

// Função de desenho que se adapta ao tamanho do bloco
void DesenharMapa(Mapa *mapa, Sprites *sprites, int tamanhoBloco);

// Auxiliares úteis para a movimentação
double CalcularDistancia(Posicao p1, Posicao p2);

// Funções de Save/Load e Utilitários (adicionar mais depois?)

// Salva o jogo completo
int SalvarEstadoMapa(Mapa *mapa, const char *nomeArquivo, Posicao pacmanAtual, Posicao *fantasmas, int qtdFantasmas);

// Carrega o jogo salvo e retorna posições dinâmicas via ponteiros
Mapa* CarregarEstadoMapa(const char *nomeArquivo, Posicao *outPacman, Posicao **outFantasmas, int *outQtdFantasmas);

// Verifica condição de vitória
int ContarPastilhasRestantes(Mapa *mapa);

#endif
