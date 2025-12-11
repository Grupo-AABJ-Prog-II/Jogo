#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

int PodeMoverF(int x, int y, Mapa *mapa) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return 0;
    if (mapa->grade[y][x] == '#') return 0;

    // Opcional: Impedir fantasmas de passarem uns pelos outros
    /*
    for (int i = 0; i < mapa->numero_fantasmas; i++)
        if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y)
            return 0;
    */
    
    return 1;
}

int PodeMoverP(int x, int y, Mapa *mapa) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return 0;
    if (mapa->grade[y][x] == '#') return 0;

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
    if (atual.x + 1 < 40 && mapa->grade[atual.y][atual.x + 1] != '#' && visitado[atual.y][atual.x + 1] != C_RIGHT) { 
         a_visitar[tamanho_visitar] = atual;
        a_visitar[tamanho_visitar++].x++;

        visitado[atual.y][atual.x + 1] = C_RIGHT;
    }
    if (atual.y > 0 && mapa->grade[atual.y - 1][atual.x] != '#') {
        a_visitar[tamanho_visitar] = atual;
        a_visitar[tamanho_visitar++].y--;

        visitado[atual.y - 1][atual.x] = C_UP;
    }
    if (PodeMoverF(atual.x - 1, atual.y, mapa)) {
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
        if (PodeMoverF(atual.x - 1, atual.y, mapa) && visitado[atual.y][atual.x - 1] == C_SEILA) {
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

void RespawnGeral(Mapa *m) {
    // 1. Reseta o Pac-Man para o início
    m->pacman.pos = m->pacman.spawnPos;
    m->pacman.dir = (Posicao){0, 0};
    m->pacman.proxDir = (Posicao){0, 0};
    m->pacman.moveTimer = 0;

    // 2. Reseta TODOS os Fantasmas para suas posições de spawn ORIGINAIS
    // Isso garante que o jogo volte ao estado inicial seguro
    for (int i = 0; i < m->numero_fantasmas; i++) {
        m->fantasmas[i].pos = m->fantasmas[i].spawnPosOriginal;
        m->fantasmas[i].moveTimer = 0;
        m->fantasmas[i].estaVulneravel = false;
        m->fantasmas[i].tempoVulneravel = 0;
    }
}

// Verifica se uma posição já está ocupada por algum fantasma
bool PosicaoOcupadaPorFantasma(Mapa *mapa, int x, int y) {
    for (int i = 0; i < mapa->numero_fantasmas; i++) {
        if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y) {
            return true;
        }
    }
    return false;
}

// Encontra um spawn livre perto do alvo (S mais distante)
Posicao EncontrarSpawnLivre(Mapa *mapa, Posicao alvo) {
    // Se o alvo está livre, usa ele
    if (!PosicaoOcupadaPorFantasma(mapa, alvo.x, alvo.y)) return alvo;

    // Se não, tenta vizinhos (Cima, Baixo, Esquerda, Direita)
    // Isso evita que fantasmas fiquem "encavalados"
    Posicao vizinhos[] = {{0,1}, {0,-1}, {1,0}, {-1,0}};
    
    for(int i=0; i<4; i++) {
        int nx = alvo.x + vizinhos[i].x;
        int ny = alvo.y + vizinhos[i].y;
        
        // Verifica limites, parede e se tem outro fantasma
        if (PodeMoverF(nx, ny, mapa) && !PosicaoOcupadaPorFantasma(mapa, nx, ny)) {
            return (Posicao){nx, ny};
        }
    }
    
    // Se tudo falhar, retorna o alvo mesmo (sobreposição é melhor que crash)
    return alvo;
}

bool Colisao(Mapa *mapa, int i) {
    if (mapa->fantasmas[i].estaVulneravel) {
        mapa->pacman.score += 200; 

        // Respawn Inteligente do Fantasma:
        // Tenta usar o spawn mais distante, mas verifica se está ocupado
        Posicao destino = EncontrarSpawnLivre(mapa, mapa->spawnFantasma);
        mapa->fantasmas[i].pos = destino;
        
        mapa->fantasmas[i].estaVulneravel = false;
        mapa->fantasmas[i].tempoVulneravel = 0;
        
        return false;
    }
    
    mapa->pacman.vidas--;
    
    // Se ainda tem vidas, faz respawn TOTAL (Pacman e Fantasmas voltam ao inicio)
    if (mapa->pacman.vidas > 0) {
        RespawnGeral(mapa);
        return false;
    }

    return true; // Game Over
}

Tela AtualizarJogo(Mapa *mapa) {
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) mapa->pacman.proxDir = (Posicao){1, 0};
    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  mapa->pacman.proxDir = (Posicao){-1, 0};
    else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  mapa->pacman.proxDir = (Posicao){0, 1};
    else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    mapa->pacman.proxDir = (Posicao){0, -1};

    Pacman *pac = &mapa->pacman;

    if (GetTime() - pac->moveTimer >= VELOCIDADE_NORMAL) {
        pac->moveTimer = GetTime();

        int nextX = pac->pos.x + pac->proxDir.x;
        int nextY = pac->pos.y + pac->proxDir.y;

        if (PodeMoverP(nextX, nextY, mapa)) {
            pac->dir = pac->proxDir;
            pac->pos.x = nextX;
            pac->pos.y = nextY;
        } else {
            nextX = pac->pos.x + pac->dir.x;
            nextY = pac->pos.y + pac->dir.y;
            if (PodeMoverP(nextX, nextY, mapa)) {
                pac->pos.x = nextX;
                pac->pos.y = nextY;
            }
        }
        
        if (mapa->grade[pac->pos.y][pac->pos.x] == 'T') {
            pac->pos = ProcessarPortal(pac->pos, mapa);
        }

        // Interação com itens
        char item = mapa->grade[pac->pos.y][pac->pos.x];
        int pontosGanhos = 0;

        if (item == '.') {
            pontosGanhos = 10;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
        } else if (item == 'o') {
            pontosGanhos = 100;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
            for(int i = 0; i < mapa->numero_fantasmas; i++) {
                mapa->fantasmas[i].estaVulneravel = true;
                mapa->fantasmas[i].tempoVulneravel = GetTime() + TEMPO_VULNERAVEL;
            }
        } else if (item == '+') {
            pontosGanhos = 300;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
        }

        if (pontosGanhos > 0) {
            int scoreAntes = pac->score;
            pac->score += pontosGanhos;

            if (pac->score / 1000 > scoreAntes / 1000) {
                pac->vidas++;
            }
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
                    if (PodeMoverF(f->pos.x, f->pos.y + 1, mapa))
                        f->pos.y++;
                    break;
                case C_UP:
                    if (PodeMoverF(f->pos.x, f->pos.y - 1, mapa))
                        f->pos.y--;
                    break;
                case C_RIGHT:
                    if (PodeMoverF(f->pos.x + 1, f->pos.y, mapa))
                        f->pos.x++;
                    break;
                case C_LEFT:
                    if (PodeMoverF(f->pos.x - 1, f->pos.y, mapa))
                        f->pos.x--;
                    break;
                default:
                    break;
            }
        }
        
        if (pac->pos.x == f->pos.x && pac->pos.y == f->pos.y) {
            if (Colisao(mapa, i))
                return TELA_GAMEOVER;
        }
    }

    int pellets = 0;

    for (int y = 0; y < 20; y++)
        for (int x = 0; x < 40; x++)
            if (mapa->grade[y][x] == '.' || mapa->grade[y][x] == 'o' || mapa->grade[y][x] == '+')
                pellets++;

    if (pellets == 0)
        return TELA_VITORIA;

    return TELA_JOGO;
}