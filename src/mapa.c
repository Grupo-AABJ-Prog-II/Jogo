#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // Para strlen e strcspn
#include <time.h>   // Para o rand()

#include "tipos.h"

// Declarações locais
void TentarGerarPointPellets(Mapa *mapa);
void CalcularSpawnDistante(Mapa *mapa);
void DesenharSpriteOuForma(Texture2D tex, int x, int y, Color cor, int ehCirculo, float escala, Color tint, int tamanhoBloco);
void ConectarPortais(Mapa *mapa);
void FloodFill(Mapa *mapa, int x, int y, int **visited); 
void LiberarMapa(Mapa *mapa);


Mapa* CarregarMapa(const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) return NULL;

    Mapa *mapa = calloc(1, sizeof(Mapa));
    mapa->linhas = 20; 
    mapa->colunas = 40;
    
    mapa->portais = NULL;
    mapa->conexoes = NULL; 

    mapa->grade = malloc(mapa->linhas * sizeof(char*));
    for (int i = 0; i < mapa->linhas; i++) {
        mapa->grade[i] = malloc((mapa->colunas + 2) * sizeof(char));
    }

    mapa->pacman.dir.x = 0;
    mapa->pacman.dir.y = 0;
    mapa->pacman.proxDir = mapa->pacman.dir;
    mapa->pacman.score = 0;
    mapa->pacman.vidas = 3;
    mapa->pacman.moveTimer = 0;

    mapa->numero_fantasmas = 0;
    mapa->fantasmas = NULL;

    char linha[256]; 
    int l = 0;
    int pacmanCount = 0;

    // Loop de leitura
    while (l < mapa->linhas) {
        if (fgets(linha, sizeof(linha), arquivo) == NULL) {
            printf("Mapa invalido: Numero de linhas menor que o exigido (%d)!\n", mapa->linhas);
            LiberarMapa(mapa);
            fclose(arquivo);
            return NULL;
        }

        // Remove quebra de linha \n ou \r para contar caracteres reais
        linha[strcspn(linha, "\r\n")] = 0;
        
        size_t tamanhoLinha = strlen(linha);

        // Validação rigorosa de colunas
        if (tamanhoLinha != (size_t)mapa->colunas) {
            printf("Mapa invalido na linha %d: Tem %zu caracteres, mas deve ter exatamente %d.\n", l + 1, tamanhoLinha, mapa->colunas);
            LiberarMapa(mapa);
            fclose(arquivo);
            return NULL;
        }

        for (int c = 0; c < mapa->colunas; c++) {
            char ch = linha[c];
            mapa->grade[l][c] = ch;
            
            if (ch == 'P') { 
                mapa->pacman.spawnPos.x = c;
                mapa->pacman.spawnPos.y = l;
                mapa->pacman.pos = mapa->pacman.spawnPos;

                mapa->grade[l][c] = '.'; 

                pacmanCount++; 
            }
            if (ch == 'T') {
                mapa->qtdPortais++;
                mapa->portais = (Posicao *)realloc(mapa->portais, mapa->qtdPortais * sizeof(Posicao));
                mapa->portais[mapa->qtdPortais - 1].x = c;
                mapa->portais[mapa->qtdPortais - 1].y = l;
            }
            if (ch == 'F') {
                mapa->grade[l][c] = '.'; 

                mapa->numero_fantasmas++;
                mapa->fantasmas = realloc(mapa->fantasmas, mapa->numero_fantasmas * sizeof(Fantasma));

                mapa->fantasmas[mapa->numero_fantasmas - 1].pos.x = c;
                mapa->fantasmas[mapa->numero_fantasmas - 1].pos.y = l;
                
                // [NOVO] Salva a posição original para resetar a fase depois
                mapa->fantasmas[mapa->numero_fantasmas - 1].spawnPosOriginal.x = c;
                mapa->fantasmas[mapa->numero_fantasmas - 1].spawnPosOriginal.y = l;

                mapa->fantasmas[mapa->numero_fantasmas - 1].moveTimer = 0;
                mapa->fantasmas[mapa->numero_fantasmas - 1].tempoVulneravel = 0;
                mapa->fantasmas[mapa->numero_fantasmas - 1].estaVulneravel = false;
            }
            if (ch == '+') mapa->qtdPointPellets++;
        }
        l++;
    }

    // Validação de linhas extras
    char extraCheck[10];
    if (fgets(extraCheck, sizeof(extraCheck), arquivo) != NULL) {
        if (strlen(extraCheck) > 0 && extraCheck[0] != '\n' && extraCheck[0] != '\r') {
            printf("Mapa invalido: O arquivo tem mais linhas do que o permitido (%d)!\n", mapa->linhas);
            LiberarMapa(mapa);
            fclose(arquivo);
            return NULL;
        }
    }

    fclose(arquivo);

    // Tem que ter exatos 1 pacman
    if (pacmanCount != 1) {
        printf("Mapa invalido: Quantidade de Pac-Man ('P') incorreta: %d. Deve haver exatamente 1!\n", pacmanCount);
        LiberarMapa(mapa);
        return NULL;
    }
    // Portais devem ser pares
    if (mapa->qtdPortais % 2 != 0) {
        printf("Mapa invalido: Numero impar de portais: %d\n", mapa->qtdPortais);
        LiberarMapa(mapa); 
        return NULL; 
    }

    // Processa lógica de conexões e pellets
    if (mapa->qtdPortais > 0) {
        ConectarPortais(mapa);
    }
    
    TentarGerarPointPellets(mapa);
    
    // Calcula o spawn global "S" mais distante para respawn
    CalcularSpawnDistante(mapa);

    /*Aqui inicia a logica do flood fill*/
    int **visited = (int**)malloc(mapa->linhas * sizeof(int*));
    for(int i=0; i<mapa->linhas; i++) {
        visited[i] = (int*)calloc(mapa->colunas, sizeof(int));
    }

    // Inicia a "tinta" a partir do Pac-Man
    FloodFill(mapa, mapa->pacman.spawnPos.x, mapa->pacman.spawnPos.y, visited);

    int mapaImpossivel = 0;
    
    // Procura por itens que não foram pintados (ou seja, inalcançáveis)
    for(int y=0; y<mapa->linhas; y++) {
        for(int x=0; x<mapa->colunas; x++) {
            char c = mapa->grade[y][x];
            // Verifica pellets, power pellets e point pellets
            if ((c == '.' || c == 'o' || c == '+') && !visited[y][x]) {
                // Imprime a posição exata do erro
                printf("Mapa inválido: Item inalcançável encontrado na Linha %d, Coluna %d (Caractere '%c')\n", y+1, x+1, c);
                mapaImpossivel = 1;
            }
        }
    }

    // Limpeza da matriz auxiliar
    for(int i=0; i<mapa->linhas; i++) free(visited[i]);
    free(visited);

    if (mapaImpossivel) {
        printf("FALHA CRITICA: O mapa contem areas que o Pac-Man nao consegue acessar.\n");
        LiberarMapa(mapa);
        return NULL;
    }
    
    return mapa;
}



// --- Novas Funções para Suporte ao Jogo (Save/Load Completo) ---

// Conta quantas pastilhas restam no mapa para verificar vitoria
int ContarPastilhasRestantes(Mapa *mapa) {
    int count = 0;
    for (int y = 0; y < mapa->linhas; y++) {
        for (int x = 0; x < mapa->colunas; x++) {
            char c = mapa->grade[y][x];
            if (c == '.' || c == 'o' || c == '+') {
                count++;
            }
        }
    }
    return count;
}

void LiberarMapa(Mapa *mapa) {
    if (!mapa) return;
    for (int i = 0; i < mapa->linhas; i++) free(mapa->grade[i]);
    free(mapa->grade);
    if (mapa->portais) free(mapa->portais);
    if (mapa->conexoes) free(mapa->conexoes);
    if (mapa->fantasmas) free(mapa->fantasmas);
    free(mapa);
}

void CarregarSprites(Sprites *sprites) {
    sprites->parede = LoadTexture("assets/wall.png");
    sprites->pastilha = LoadTexture("assets/pellet.png");
    sprites->superPastilha = LoadTexture("assets/power.png");
    sprites->pacman = LoadTexture("assets/pacman.png");
    sprites->fantasma = LoadTexture("assets/ghost.png");
    sprites->portal = LoadTexture("assets/portal.png");
}

void DescarregarSprites(Sprites *sprites) {
    UnloadTexture(sprites->parede);
    UnloadTexture(sprites->pastilha);
    UnloadTexture(sprites->superPastilha);
    UnloadTexture(sprites->pacman);
    UnloadTexture(sprites->fantasma);
    UnloadTexture(sprites->portal);
}

void DesenharMapa(Mapa *mapa, Sprites *sprites, int tamanhoBloco) {
    for (int y = 0; y < mapa->linhas; y++) {
        for (int x = 0; x < mapa->colunas; x++) {
            int px = x * tamanhoBloco;
            int py = y * tamanhoBloco;
            char tile = mapa->grade[y][x];

            // 1. Desenha Pacman
            if (mapa->pacman.pos.x == x && mapa->pacman.pos.y == y) {
                DesenharSpriteOuForma(sprites->pacman, px, py, YELLOW, 1, 1.0f, WHITE, tamanhoBloco);
                continue; 
            }
            
            // 2. Desenha Fantasmas
            bool temFantasma = false;
            for (int i = 0; i < mapa->numero_fantasmas; i++) {
                if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y) {
                    
                    Color corFantasma = RED;
                    Color tintSprite = WHITE;

                    // Lógica de Piscar (Vulnerável)
                    if (mapa->fantasmas[i].estaVulneravel) {
                        // Pisca a cada 0.2 segundos (5x por segundo)
                        if (fmod(GetTime(), 0.4) < 0.2) {
                            corFantasma = BLUE; // Azul vulnerável
                            tintSprite = BLUE;
                        } else {
                            corFantasma = WHITE; // Branco piscando
                            tintSprite = WHITE;
                        }
                    }

                    DesenharSpriteOuForma(sprites->fantasma, px, py, corFantasma, 1, 1.0f, tintSprite, tamanhoBloco);
                    temFantasma = true;
                    break; 
                }
            }
            if (temFantasma)
                continue;


            switch (tile) {
                case '#': DesenharSpriteOuForma(sprites->parede, px, py, BLUE, 0, 1.0f, WHITE, tamanhoBloco); break;
                case '.': DesenharSpriteOuForma(sprites->pastilha, px, py, WHITE, 1, 0.2f, WHITE, tamanhoBloco); break;
                case 'o': DesenharSpriteOuForma(sprites->superPastilha, px, py, GREEN, 1, 0.6f, WHITE, tamanhoBloco); break;
                case '+': DesenharSpriteOuForma(sprites->superPastilha, px, py, GOLD, 1, 0.6f, GOLD, tamanhoBloco); break;
                case 'T': DesenharSpriteOuForma(sprites->portal, px, py, PINK, 0, 1.0f, WHITE, tamanhoBloco); break;
                case 'S': 
                    DrawText("s", px + (tamanhoBloco/4), py + (tamanhoBloco/4), tamanhoBloco/2, DARKGRAY);
                    break;
            }
        }
    }
}

double CalcularDistancia(Posicao p1, Posicao p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

// --- Implementações Auxiliares ---

// Algoritmo Flood Fill Recursivo
void FloodFill(Mapa *mapa, int x, int y, int **visited) {
    // Limites do mapa
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return;
    
    // Já visitado ou Parede
    if (visited[y][x] || mapa->grade[y][x] == '#') return;

    // Marca como visitado
    visited[y][x] = 1;

    // Espalha para vizinhos (Cima, Baixo, Esquerda, Direita)
    FloodFill(mapa, x + 1, y, visited);
    FloodFill(mapa, x - 1, y, visited);
    FloodFill(mapa, x, y + 1, visited);
    FloodFill(mapa, x, y - 1, visited);

    // Lógica de Portais
    if (mapa->grade[y][x] == 'T') {
        for(int i = 0; i < mapa->qtdPortais; i++) {
            if (mapa->portais[i].x == x && mapa->portais[i].y == y) {
                int destinoIdx = mapa->conexoes[i];
                if (destinoIdx != -1) {
                    Posicao destino = mapa->portais[destinoIdx];
                    FloodFill(mapa, destino.x, destino.y, visited);
                }
                break;
            }
        }
    }
}

void DesenharSpriteOuForma(Texture2D tex, int x, int y, Color cor, int ehCirculo, float escala, Color tint, int tamanhoBloco) {
    if (tex.id > 0) {
        Rectangle fonte = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle destino = { (float)x, (float)y, (float)tamanhoBloco, (float)tamanhoBloco };
        // Se tiver tint (cor diferente de WHITE), aplica na textura
        DrawTexturePro(tex, fonte, destino, (Vector2){0,0}, 0.0f, tint);
    } else {
        int cx = x + tamanhoBloco / 2;
        int cy = y + tamanhoBloco / 2;
        if (ehCirculo) {
            DrawCircle(cx, cy, (tamanhoBloco/2.0f - 2)*escala, cor);
        } else {
            DrawRectangle(x, y, tamanhoBloco, tamanhoBloco, cor);
            if (cor.r == BLUE.r) DrawRectangleLines(x, y, tamanhoBloco, tamanhoBloco, DARKBLUE);
        }
    }
}

void ConectarPortais(Mapa *mapa) {
    // Aloca vetor de conexões (indice -> indice)
    mapa->conexoes = (int *)malloc(mapa->qtdPortais * sizeof(int));
    
    // Inicializa tudo como -1 (sem conexão)
    for (int i = 0; i < mapa->qtdPortais; i++) {
        mapa->conexoes[i] = -1;
    }

    for (int i = 0; i < mapa->qtdPortais; i++) {
        // Se este portal já tem par, pula
        if (mapa->conexoes[i] != -1) continue;

        int melhorPar = -1;
        int linhaOposta = (mapa->linhas - 1) - mapa->portais[i].y;
        
        // CORREÇÃO AQUI: Usar malloc, não VLA
        int *candidatosOpostos = (int *)malloc(mapa->qtdPortais * sizeof(int));
        int qtdCandidatos = 0;

        for (int j = 0; j < mapa->qtdPortais; j++) {
            // Ignora a si mesmo e portais já conectados
            if (i == j || mapa->conexoes[j] != -1) continue;

            if (mapa->portais[j].y == linhaOposta) {
                candidatosOpostos[qtdCandidatos++] = j;
            }
        }

        if (qtdCandidatos > 0) {
            // Escolhe aleatoriamente
            int r = rand() % qtdCandidatos;
            melhorPar = candidatosOpostos[r];
        } 
        else {
            // Mais distante (não na mesma linha)
            double maxDist = -1.0;
            int candidatoDistante = -1;

            for (int j = 0; j < mapa->qtdPortais; j++) {
                if (i == j || mapa->conexoes[j] != -1) continue;
                if (mapa->portais[j].y != mapa->portais[i].y) {
                    double dist = CalcularDistancia(mapa->portais[i], mapa->portais[j]);
                    if (dist > maxDist) {
                        maxDist = dist;
                        candidatoDistante = j;
                    }
                }
            }

            if (candidatoDistante != -1) {
                melhorPar = candidatoDistante;
            } else {
                // Sobrou apenas a mesma linha
                // Escolhe o mais distante na mesma linha
                maxDist = -1.0;
                for (int j = 0; j < mapa->qtdPortais; j++) {
                    if (i == j || mapa->conexoes[j] != -1) continue;
                    double dist = CalcularDistancia(mapa->portais[i], mapa->portais[j]);
                    if (dist > maxDist) {
                        maxDist = dist;
                        melhorPar = j;
                    }
                }
            }
        }
        
        // Agora o free funcionará corretamente
        free(candidatosOpostos);

        if (melhorPar != -1) {
            mapa->conexoes[i] = melhorPar;
            mapa->conexoes[melhorPar] = i;
        }
    }
}

void TentarGerarPointPellets(Mapa *mapa) {
    if (mapa->qtdPointPellets > 0) return;
    for (int y = 0; y < mapa->linhas; y++) {
        for (int x = 0; x < mapa->colunas; x++) {
            if (mapa->grade[y][x] == 'o') {
                double chance = ((double)rand() / RAND_MAX) * 100.0;
                if (chance <= 2.54345) {
                    mapa->grade[y][x] = '+';
                    mapa->qtdPointPellets++;
                }
            }
        }
    }
}

// Calcula o spawn global "S" mais distante para respawn dos fantasmas
// Se não houver 'S' no mapa, usa uma posição padrão (ex: 1,1) ou a posição inicial do fantasma
void CalcularSpawnDistante(Mapa *mapa) {
    double distMax = -1.0;
    Posicao melhorPos = {1, 1}; // Default seguro caso nao tenha S
    int achou = 0;
    
    // Varre o mapa procurando por 'S'
    for (int y = 0; y < mapa->linhas; y++) {
        for (int x = 0; x < mapa->colunas; x++) {
            if (mapa->grade[y][x] == 'S') {
                Posicao posAtual = {x, y};
                double dist = CalcularDistancia(mapa->pacman.spawnPos, posAtual); // Distancia do spawn original do Pacman
                if (dist > distMax) { 
                    distMax = dist; 
                    melhorPos = posAtual; 
                }
                achou = 1;
            }
        }
    }
    
    mapa->temSpawn = achou;
    if (achou) {
        mapa->spawnFantasma = melhorPos;
    } else {
        // Se nao tem S definido no mapa, usa o spawn do primeiro fantasma encontrado ou 1,1
        if (mapa->numero_fantasmas > 0) {
            mapa->spawnFantasma = mapa->fantasmas[0].pos; // Usa posição do F original
        } else {
            mapa->spawnFantasma = melhorPos;
        }
    }
}