#ifndef ENTIDADES_H
#define ENTIDADES_H

#include <stdbool.h>
#include <raylib.h>

#include "tipos.h"

#define TEMPO_VULNERAVEL 8.0f
#define VELOCIDADE_NORMAL 0.25f // Menor = Mais rápido (segundos por bloco)
#define VELOCIDADE_LENTA 0.33f  

typedef enum {
    ESTADO_JOGANDO,
    ESTADO_GAMEOVER,
    ESTADO_VITORIA
} GameState;

GameState AtualizarJogo(Mapa *mapa);

#endif
