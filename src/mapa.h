#ifndef MAPA_H
#define MAPA_H

#include "tipos.h"

// Carrega um nível específico (ex: 1 procura "mapa1.txt")
Mapa* CarregarMapaNivel(int nivel);

void LiberarMapa(Mapa *mapa);
void CarregarSprites(Sprites *sprites);
void DescarregarSprites(Sprites *sprites);

// Desenha o mapa e entidades
void DesenharMapa(Mapa *mapa, Sprites *sprites, int tamanhoBloco);

double CalcularDistancia(Posicao p1, Posicao p2);

// --- SISTEMA DE SAVE/LOAD ---
// Salva o estado completo do jogo em binário
bool SalvarJogo(Mapa *mapa, const char *nomeArquivo);

// Carrega o estado completo do jogo
Mapa* CarregarJogo(const char *nomeArquivo);

int ContarPastilhasRestantes(Mapa *mapa);

#endif