#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <raylib.h>

#include "entidades.h"

enum Caminho {
    C_SEILA,
    C_UP,
    C_DOWN,
    C_LEFT,
    C_RIGHT
};

int PodeMover(int x, int y, Mapa *mapa) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return 0;
    if (mapa->grade[y][x] == '#') return 0;

    for (int i = 0; i < mapa->numero_fantasmas; i++)
        if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y)
            return 0;
    
    return 1;
}

enum Caminho SeguirJogador(Posicao atual, Mapa *mapa) {
    Posicao a_visitar[1000];
    int tamanho_visitar = 0;


    enum Caminho visitado[20][40];

    for (int y = 0; y < 20; y++)
        for (int x = 0; x < 40; x++)
            visitado[y][x] = C_SEILA;

    if (atual.x == mapa->pacman.pos.x && atual.y == mapa->pacman.pos.y)
        return C_UP;
    
    if (atual.y + 1 < 20 && mapa->grade[atual.y + 1][atual.x] != '#') {
        a_visitar[tamanho_visitar] = atual;
        a_visitar[tamanho_visitar++].y++;

        visitado[atual.y + 1][atual.x] = C_DOWN;
    }
    if (atual.x + 1 < 40 && mapa->grade[atual.y][atual.x + 1] != '#') {
        a_visitar[tamanho_visitar] = atual;
        a_visitar[tamanho_visitar++].x++;

        visitado[atual.y][atual.x + 1] = C_RIGHT;
    }
    if (atual.y > 0 && mapa->grade[atual.y - 1][atual.x] != '#') {
        a_visitar[tamanho_visitar] = atual;
        a_visitar[tamanho_visitar++].y--;

        visitado[atual.y - 1][atual.x] = C_UP;
    }
    if (PodeMover(atual.x - 1, atual.y, mapa)) {
        a_visitar[tamanho_visitar] = atual;
        a_visitar[tamanho_visitar++].x--;

        visitado[atual.y][atual.x - 1] = C_LEFT;
    }

    for (int i = 0; i < tamanho_visitar; i++) {
        atual = a_visitar[i];

        if (atual.x == mapa->pacman.pos.x && atual.y == mapa->pacman.pos.y)
            return visitado[atual.y][atual.x];

        if (atual.y + 1 < 20 && mapa->grade[atual.y + 1][atual.x] != '#' && visitado[atual.y + 1][atual.x] == C_SEILA) {
            a_visitar[tamanho_visitar] = atual;
            a_visitar[tamanho_visitar++].y++;

            visitado[atual.y + 1][atual.x] = visitado[atual.y][atual.x];
        }
        if (atual.x + 1 < 40 && mapa->grade[atual.y][atual.x + 1] != '#' && visitado[atual.y][atual.x + 1] == C_SEILA) {
            a_visitar[tamanho_visitar] = atual;
            a_visitar[tamanho_visitar++].x++;

            visitado[atual.y][atual.x + 1] = visitado[atual.y][atual.x];
        }
        if (atual.y > 0 && mapa->grade[atual.y - 1][atual.x] != '#' && visitado[atual.y - 1][atual.x] == C_SEILA) {
            a_visitar[tamanho_visitar] = atual;
            a_visitar[tamanho_visitar++].y--;

            visitado[atual.y - 1][atual.x] = visitado[atual.y][atual.x];
        }
        if (PodeMover(atual.x - 1, atual.y, mapa) && visitado[atual.y][atual.x - 1] == C_SEILA) {
            a_visitar[tamanho_visitar] = atual;
            a_visitar[tamanho_visitar++].x--;

            visitado[atual.y][atual.x - 1] = visitado[atual.y][atual.x];
        }
    }

    return C_UP;
}

Posicao ProcessarPortal(Posicao atual, Mapa *mapa) {
    for (int i = 0; i < mapa->qtdPortais; i++) {
        if (mapa->portais[i].x == atual.x && mapa->portais[i].y == atual.y) {
            int destinoIdx = mapa->conexoes[i];
            if (destinoIdx != -1) {
                return mapa->portais[destinoIdx]; 
            }
        }
    }
    return atual; 
}

void Respawn(Mapa *m) {
    // Reseta APENAS o Pac-Man para o início
    m->pacman.pos = m->pacman.spawnPos;
    m->pacman.dir = (Posicao){0, 0};
    m->pacman.proxDir = (Posicao){0, 0};
    m->pacman.moveTimer = 0;
}

GameState AtualizarJogo(Mapa *mapa) {
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) mapa->pacman.proxDir = (Posicao){1, 0};
    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  mapa->pacman.proxDir = (Posicao){-1, 0};
    else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  mapa->pacman.proxDir = (Posicao){0, 1};
    else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    mapa->pacman.proxDir = (Posicao){0, -1};

    Pacman *pac = &mapa->pacman;

    if (GetTime() - pac->moveTimer >= VELOCIDADE_NORMAL) {
        pac->moveTimer = GetTime();

        int nextX = pac->pos.x + pac->proxDir.x;
        int nextY = pac->pos.y + pac->proxDir.y;

        if (PodeMover(nextX, nextY, mapa)) {
            pac->dir = pac->proxDir;
            pac->pos.x = nextX;
            pac->pos.y = nextY;
        } else {
            nextX = pac->pos.x + pac->dir.x;
            nextY = pac->pos.y + pac->dir.y;
            if (PodeMover(nextX, nextY, mapa)) {
                pac->pos.x = nextX;
                pac->pos.y = nextY;
            }
        }
        
        if (mapa->grade[pac->pos.y][pac->pos.x] == 'T') {
            pac->pos = ProcessarPortal(pac->pos, mapa);
        }


        // Interação com itens
        char item = mapa->grade[pac->pos.y][pac->pos.x];
        if (item == '.') {
            pac->score += 10;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
        } else if (item == 'o') {
            pac->score += 50;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
            for(int i = 0; i < mapa->numero_fantasmas; i++) {
                mapa->fantasmas[i].estaVulneravel = true;
                mapa->fantasmas[i].tempoVulneravel = GetTime() + TEMPO_VULNERAVEL;
            }
        } else if (item == '+') {
            pac->score += 100;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
        }
    }

    for (int i = 0; i < mapa->numero_fantasmas; i++) {
        Fantasma *f = &mapa->fantasmas[i];

        if (f->estaVulneravel) {
            if (f->tempoVulneravel <= GetTime()) f->estaVulneravel = 0;
        }

        float velocidade = f->estaVulneravel ? VELOCIDADE_LENTA : VELOCIDADE_NORMAL;

        if (GetTime() - f->moveTimer >= velocidade) {
            f->moveTimer = GetTime();

            enum Caminho dir = SeguirJogador(f->pos, mapa);

            if (f->estaVulneravel) {
                if (dir == C_DOWN) dir = C_UP;
                if (dir == C_UP) dir = C_DOWN;
                if (dir == C_LEFT) dir = C_RIGHT;
                if (dir == C_RIGHT) dir = C_LEFT;
            }

            switch (dir) {
                case C_DOWN:
                    if (PodeMover(f->pos.x, f->pos.y + 1, mapa))
                        f->pos.y++;
                    break;
                case C_UP:
                    if (PodeMover(f->pos.x, f->pos.y - 1, mapa))
                        f->pos.y--;
                    break;
                case C_RIGHT:
                    if (PodeMover(f->pos.x + 1, f->pos.y, mapa))
                        f->pos.x++;
                    break;
                case C_LEFT:
                    if (PodeMover(f->pos.x - 1, f->pos.y, mapa))
                        f->pos.x--;
                    break;
            }
        }
        
        if (pac->pos.x == f->pos.x && pac->pos.y == f->pos.y) {
            if (f->estaVulneravel) {
                pac->score += 200;
                // TODO: matar fantasma
                //f->estaAtivo = 0; 
            } else {
                pac->vidas--;
                pac->score -= 200; 
                if (pac->score < 0) pac->score = 0;
                
                if (pac->vidas > 0) {
                    Respawn(mapa);
                    
                    break; 
                } else {
                    return ESTADO_GAMEOVER;
                }
            }
        }
    }

    // TODO: reviver
    //if (ContarPastilhasRestantes(mapa) == 0) return ESTADO_VITORIA;

    return ESTADO_JOGANDO;
}

