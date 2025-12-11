#ifndef HUD_H
#define HUD_H

#include "raylib.h"


typedef struct {
    int pontuacao;
    int vidas;
    int nivel;
    int pelletsRestantes;
    int gameover;
} EstadoJogo;

// Inicializa os dados com valores padrão (0 pontos, 3 vidas...)
void InicializarEstadoJogo(EstadoJogo *estado);

// Desenha a barra inferior com as informações
void DesenharHUD(EstadoJogo *estado, int larguraJanela, int alturaJanela, int alturaHUD);

// Função auxiliar para somar pontos
void AdicionarPontos(EstadoJogo *estado, int pontos);

#endif