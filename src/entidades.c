#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h> 

#include "entidades.h"

enum Caminho { C_SEILA, C_UP, C_DOWN, C_LEFT, C_RIGHT };

// [REMOVIDO] A função Vector2Lerp causava conflito.
// Agora usamos a nativa da Raylib incluída em raymath.h

int PodeMoverF(int x, int y, Mapa *mapa) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return 0;
    if (mapa->grade[y][x] == '#') return 0;
    // Fantasmas colidem entre si
    for (int i = 0; i < mapa->numero_fantasmas; i++)
        if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y) return 0;
    return 1;
}

int PodeMoverP(int x, int y, Mapa *mapa) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return 0;
    if (mapa->grade[y][x] == '#') return 0;
    return 1;
}

enum Caminho SeguirJogador(Posicao atual, Mapa *mapa) {
    if(atual.x < mapa->pacman.pos.x && PodeMoverF(atual.x+1, atual.y, mapa)) return C_RIGHT;
    if(atual.x > mapa->pacman.pos.x && PodeMoverF(atual.x-1, atual.y, mapa)) return C_LEFT;
    if(atual.y < mapa->pacman.pos.y && PodeMoverF(atual.x, atual.y+1, mapa)) return C_DOWN;
    if(atual.y > mapa->pacman.pos.y && PodeMoverF(atual.x, atual.y-1, mapa)) return C_UP;
    
    int r = rand() % 4;
    if(r==0 && PodeMoverF(atual.x+1, atual.y, mapa)) return C_RIGHT;
    if(r==1 && PodeMoverF(atual.x-1, atual.y, mapa)) return C_LEFT;
    if(r==2 && PodeMoverF(atual.x, atual.y+1, mapa)) return C_DOWN;
    if(r==3 && PodeMoverF(atual.x, atual.y-1, mapa)) return C_UP;
    return C_SEILA;
}

Posicao ProcessarPortal(Posicao atual, Mapa *mapa) {
    for (int i = 0; i < mapa->qtdPortais; i++) {
        if (mapa->portais[i].x == atual.x && mapa->portais[i].y == atual.y) {
            int idx = mapa->conexoes[i];
            if (idx != -1) return mapa->portais[idx];
        }
    }
    return atual;
}

void AtivarPoder(Mapa *mapa) {
    for(int i = 0; i < mapa->numero_fantasmas; i++) {
        mapa->fantasmas[i].estaVulneravel = true;
        mapa->fantasmas[i].tempoVulneravel = GetTime() + TEMPO_VULNERAVEL;
    }
}

void RespawnGeral(Mapa *m) {
    m->pacman.pos = m->pacman.spawnPos;
    m->pacman.pixelPos = (Vector2){(float)m->pacman.spawnPos.x, (float)m->pacman.spawnPos.y};
    m->pacman.dir = (Posicao){0,0};
    m->pacman.proxDir = (Posicao){0,0};
    m->pacman.moveTimer = 0;
    m->jogoIniciado = false; 

    for (int i = 0; i < m->numero_fantasmas; i++) {
        m->fantasmas[i].pos = m->fantasmas[i].spawnPosOriginal;
        m->fantasmas[i].pixelPos = (Vector2){(float)m->fantasmas[i].spawnPosOriginal.x, (float)m->fantasmas[i].spawnPosOriginal.y};
        m->fantasmas[i].moveTimer = 0;
        m->fantasmas[i].estaVulneravel = false;
    }
}

bool PosicaoOcupadaPorFantasma(Mapa *mapa, int x, int y) {
    for (int i = 0; i < mapa->numero_fantasmas; i++) {
        if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y) return true;
    }
    return false;
}

Posicao EncontrarSpawnLivre(Mapa *mapa, Posicao alvo) {
    if (!PosicaoOcupadaPorFantasma(mapa, alvo.x, alvo.y)) return alvo;
    Posicao vizinhos[] = {{0,1}, {0,-1}, {1,0}, {-1,0}};
    for(int i=0; i<4; i++) {
        int nx = alvo.x + vizinhos[i].x;
        int ny = alvo.y + vizinhos[i].y;
        if (PodeMoverF(nx, ny, mapa) && !PosicaoOcupadaPorFantasma(mapa, nx, ny)) return (Posicao){nx, ny};
    }
    return alvo;
}

bool Colisao(Mapa *mapa, int i) {
    if (mapa->fantasmas[i].estaVulneravel) {
        mapa->pacman.score += 200;
        Posicao destino = EncontrarSpawnLivre(mapa, mapa->spawnFantasma);
        mapa->fantasmas[i].pos = destino;
        mapa->fantasmas[i].pixelPos = (Vector2){(float)destino.x, (float)destino.y};
        mapa->fantasmas[i].estaVulneravel = false;
        return false;
    }
    mapa->pacman.vidas--;
    if (mapa->pacman.vidas > 0) {
        RespawnGeral(mapa);
        return false;
    }
    return true;
}

Tela AtualizarJogo(Mapa *mapa) {
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) mapa->pacman.proxDir = (Posicao){1, 0};
    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  mapa->pacman.proxDir = (Posicao){-1, 0};
    else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  mapa->pacman.proxDir = (Posicao){0, 1};
    else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    mapa->pacman.proxDir = (Posicao){0, -1};

    if (!mapa->jogoIniciado && (mapa->pacman.proxDir.x != 0 || mapa->pacman.proxDir.y != 0)) {
        if (PodeMoverP(mapa->pacman.pos.x + mapa->pacman.proxDir.x, mapa->pacman.pos.y + mapa->pacman.proxDir.y, mapa)) {
            mapa->jogoIniciado = true;
        }
    }

    Pacman *pac = &mapa->pacman;

    if (GetTime() - pac->moveTimer >= VELOCIDADE_NORMAL) {
        pac->moveTimer = GetTime();
        int nextX = pac->pos.x + pac->proxDir.x;
        int nextY = pac->pos.y + pac->proxDir.y;

        if (PodeMoverP(nextX, nextY, mapa)) {
            pac->dir = pac->proxDir;
            pac->pos.x = nextX; pac->pos.y = nextY;
        } else {
            nextX = pac->pos.x + pac->dir.x;
            nextY = pac->pos.y + pac->dir.y;
            if (PodeMoverP(nextX, nextY, mapa)) { pac->pos.x = nextX; pac->pos.y = nextY; }
        }
        
        if (mapa->grade[pac->pos.y][pac->pos.x] == 'T') {
            Posicao saida = ProcessarPortal(pac->pos, mapa);
            pac->pos = saida;
            pac->pixelPos = (Vector2){(float)saida.x, (float)saida.y};
        }

        char item = mapa->grade[pac->pos.y][pac->pos.x];
        int pontos = 0;
        if (item == '.') { pontos = 10; mapa->grade[pac->pos.y][pac->pos.x] = ' '; }
        else if (item == 'o') { pontos = 100; mapa->grade[pac->pos.y][pac->pos.x] = ' '; AtivarPoder(mapa); }
        else if (item == '+') { pontos = 300; mapa->grade[pac->pos.y][pac->pos.x] = ' '; AtivarPoder(mapa); }

        if (pontos > 0) {
            int scoreAntes = pac->score;
            pac->score += pontos;
            if (pac->score / 1000 > scoreAntes / 1000) pac->vidas++;
        }
    }

    // Usa a função nativa da Raylib agora
    pac->pixelPos = Vector2Lerp(pac->pixelPos, (Vector2){(float)pac->pos.x, (float)pac->pos.y}, 0.25f);

    if (mapa->jogoIniciado) {
        for (int i = 0; i < mapa->numero_fantasmas; i++) {
            Fantasma *f = &mapa->fantasmas[i];
            if (f->estaVulneravel && f->tempoVulneravel <= GetTime()) f->estaVulneravel = false;
            float vel = f->estaVulneravel ? VELOCIDADE_LENTA : VELOCIDADE_NORMAL;

            if (GetTime() - f->moveTimer >= vel) {
                f->moveTimer = GetTime();
                enum Caminho dir = SeguirJogador(f->pos, mapa);
                if (f->estaVulneravel) { 
                    if (dir == C_DOWN) dir = C_UP; else if (dir == C_UP) dir = C_DOWN;
                    else if (dir == C_LEFT) dir = C_RIGHT; else if (dir == C_RIGHT) dir = C_LEFT;
                }
                switch (dir) {
                    case C_DOWN: if (PodeMoverF(f->pos.x, f->pos.y + 1, mapa)) f->pos.y++; break;
                    case C_UP: if (PodeMoverF(f->pos.x, f->pos.y - 1, mapa)) f->pos.y--; break;
                    case C_RIGHT: if (PodeMoverF(f->pos.x + 1, f->pos.y, mapa)) f->pos.x++; break;
                    case C_LEFT: if (PodeMoverF(f->pos.x - 1, f->pos.y, mapa)) f->pos.x--; break;
                    default: break;
                }
            }
            f->pixelPos = Vector2Lerp(f->pixelPos, (Vector2){(float)f->pos.x, (float)f->pos.y}, 0.25f);

            if (pac->pos.x == f->pos.x && pac->pos.y == f->pos.y) {
                if (Colisao(mapa, i)) return TELA_GAMEOVER;
            }
        }
    }

    int pellets = 0;
    for (int y=0; y<20; y++) for (int x=0; x<40; x++)
        if (mapa->grade[y][x] == '.' || mapa->grade[y][x] == 'o' || mapa->grade[y][x] == '+') pellets++;

    if (pellets == 0) return TELA_VITORIA;

    return TELA_JOGO;
}