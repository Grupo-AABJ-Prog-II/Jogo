#include "entidades.h"
#include <stdlib.h>
#include <math.h>

// --- Funções Auxiliares Internas ---

int PodeMover(int x, int y, Mapa *mapa) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return 0;
    if (mapa->grade[y][x] == '#') return 0;
    return 1;
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

// --- AQUI ESTÁ A MUDANÇA ---
void Respawn(Pacman *pac, Fantasma *fantasmas) {
    // Reseta APENAS o Pac-Man para o início
    pac->pos = pac->spawnPos;
    pac->dir = (Posicao){0, 0};
    pac->proxDir = (Posicao){0, 0};
    pac->moveTimer = 0;

    // OBS: Os fantasmas NÃO são tocados aqui. 
    // Eles mantêm posição, direção e estado (vulnerável ou não).
    // O parâmetro *fantasmas é mantido apenas para compatibilidade com o header.
    (void)fantasmas; // Evita warning de "unused parameter"
}

// --- Funções Principais ---

void InicializarEntidades(Mapa *mapa, Pacman *pac, Fantasma *fantasmas) {
    pac->spawnPos = mapa->inicioPacman;
    pac->pos = mapa->inicioPacman;
    pac->dir = (Posicao){0, 0};
    pac->proxDir = (Posicao){0, 0};
    pac->score = 0;
    pac->vidas = 3;
    pac->moveTimer = 0;

    Color cores[4] = {RED, PINK, SKYBLUE, ORANGE};

    for (int i = 0; i < QTD_FANTASMAS; i++) {
        fantasmas[i].id = i;
        fantasmas[i].corOriginal = cores[i % 4];
        
        if (mapa->temSpawn) {
            fantasmas[i].spawnPos = mapa->spawnFantasma;
        } else {
            fantasmas[i].spawnPos = (Posicao){mapa->colunas/2, mapa->linhas/2}; 
        }
        
        fantasmas[i].pos = fantasmas[i].spawnPos;
        fantasmas[i].dir = (Posicao){0, 0}; 
        fantasmas[i].estaAtivo = 1;
        fantasmas[i].estaVulneravel = 0;
    }
}

GameState AtualizarJogo(Mapa *mapa, Pacman *pac, Fantasma *fantasmas, float deltaTime) {
    // 1. INPUT
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) pac->proxDir = (Posicao){1, 0};
    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  pac->proxDir = (Posicao){-1, 0};
    else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  pac->proxDir = (Posicao){0, 1};
    else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    pac->proxDir = (Posicao){0, -1};

    // 2. MOVIMENTO PACMAN
    pac->moveTimer += deltaTime;
    if (pac->moveTimer >= VELOCIDADE_NORMAL) {
        pac->moveTimer = 0;

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
            for(int i=0; i<QTD_FANTASMAS; i++) {
                if(fantasmas[i].estaAtivo) {
                    fantasmas[i].estaVulneravel = 1;
                    fantasmas[i].tempoVulneravel = TEMPO_VULNERAVEL;
                }
            }
        } else if (item == '+') {
            pac->score += 100;
            mapa->grade[pac->pos.y][pac->pos.x] = ' ';
        }
    }

    // 3. MOVIMENTO FANTASMAS
    Posicao direcoes[4] = {{0,-1}, {0,1}, {-1,0}, {1,0}};

    for (int i = 0; i < QTD_FANTASMAS; i++) {
        Fantasma *f = &fantasmas[i];
        if (!f->estaAtivo) continue; 

        if (f->estaVulneravel) {
            f->tempoVulneravel -= deltaTime;
            if (f->tempoVulneravel <= 0) f->estaVulneravel = 0;
        }

        float velocidade = f->estaVulneravel ? VELOCIDADE_LENTA : VELOCIDADE_NORMAL;
        f->moveTimer += deltaTime;

        if (f->moveTimer >= velocidade) {
            f->moveTimer = 0;
            
            int nx = f->pos.x + f->dir.x;
            int ny = f->pos.y + f->dir.y;
            
            int caminhosLivres = 0;
            for(int k=0; k<4; k++) {
                if(PodeMover(f->pos.x + direcoes[k].x, f->pos.y + direcoes[k].y, mapa)) 
                    caminhosLivres++;
            }

            int bateu = !PodeMover(nx, ny, mapa);
            
            if (bateu || caminhosLivres > 2) {
                Posicao opcoes[4];
                int qtdOpcoes = 0;
                for(int k=0; k<4; k++) {
                    if (direcoes[k].x == -f->dir.x && direcoes[k].y == -f->dir.y && caminhosLivres > 1) continue;
                    
                    if (PodeMover(f->pos.x + direcoes[k].x, f->pos.y + direcoes[k].y, mapa)) {
                        opcoes[qtdOpcoes++] = direcoes[k];
                    }
                }
                if (qtdOpcoes > 0) f->dir = opcoes[rand() % qtdOpcoes];
            }

            f->pos.x += f->dir.x;
            f->pos.y += f->dir.y;

            if (mapa->grade[f->pos.y][f->pos.x] == 'T') {
                f->pos = ProcessarPortal(f->pos, mapa);
            }
        }
        
        // 4. COLISÃO
        if (pac->pos.x == f->pos.x && pac->pos.y == f->pos.y) {
            if (f->estaVulneravel) {
                pac->score += 200;
                f->estaAtivo = 0; 
                f->pos = (Posicao){-1,-1};
            } else {
                pac->vidas--;
                pac->score -= 200; 
                if (pac->score < 0) pac->score = 0;
                
                if (pac->vidas > 0) {
                    // Chama o Respawn modificado (só Pac-Man volta)
                    Respawn(pac, fantasmas);
                    
                    // Opcional: Break para garantir que não processe colisão 
                    // com outro fantasma no mesmo frame antes do teleporte
                    break; 
                } else {
                    return ESTADO_GAMEOVER;
                }
            }
        }
    }

    if (ContarPastilhasRestantes(mapa) == 0) return ESTADO_VITORIA;

    return ESTADO_JOGANDO;
}

void DesenharEntidades(Pacman *pac, Fantasma *fantasmas, Sprites *sprites, int tamanhoBloco) {
    (void)sprites; // Sprites não usados (versão geométrica)

    // Pacman
    int px = pac->pos.x * tamanhoBloco + tamanhoBloco/2;
    int py = pac->pos.y * tamanhoBloco + tamanhoBloco/2;
    float raio = (tamanhoBloco / 2.0f) - 2;
    
    DrawCircle(px, py, raio, YELLOW);
    
    // Olho
    int olhoX = px + (pac->dir.x * 10);
    int olhoY = py + (pac->dir.y * 10);
    if (pac->dir.x != 0 || pac->dir.y != 0) {
        DrawCircle(olhoX, olhoY, 3, BLACK);
    }

    // Fantasmas
    for (int i = 0; i < QTD_FANTASMAS; i++) {
        if (!fantasmas[i].estaAtivo) continue;

        int fx = fantasmas[i].pos.x * tamanhoBloco + tamanhoBloco/2;
        int fy = fantasmas[i].pos.y * tamanhoBloco + tamanhoBloco/2;
        
        Color cor = fantasmas[i].corOriginal;

        if (fantasmas[i].estaVulneravel) {
            cor = BLUE; 
            if (fantasmas[i].tempoVulneravel < 2.0f) {
                 if (((int)(fantasmas[i].tempoVulneravel * 5)) % 2 == 0) cor = WHITE;
            }
        }

        DrawCircle(fx, fy, raio, cor);
    }
    
    // HUD
    int hudY = GetScreenHeight() - 30;
    DrawText(TextFormat("SCORE: %06d", pac->score), 20, hudY, 20, WHITE);
    DrawText(TextFormat("LIVES: %d", pac->vidas), 250, hudY, 20, WHITE);
}