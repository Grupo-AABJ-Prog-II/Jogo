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

    /*Aqui inicia a logica do flood fill, a função recursiva está um pouco mais abaixo no codigo, mas basicamente
     ele é utilizado aqui para validar a jogabilidade do mapa. Ele simula o movimento do Pac-Man a partir de sua posição
     inicial, marcando todas as células alcançáveis (chão, portais, itens).
     para garantir que não existam itens isoladas ou itens cercados por paredes
     que tornariam o jogo impossível de ser completado. Se, ao final da inundação,
     restar alguma pellet não visitada, o mapa é considerado inválido.*/
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



// Save/Load Completo

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

/*
// Agora recebe a posição ATUAL do Pacman e a lista de Fantasmas para salvar no binário
int SalvarEstadoMapa(Mapa *mapa, const char *nomeArquivo, Posicao pacmanAtual, Posicao *fantasmas, int qtdFantasmas) {
    FILE *file = fopen(nomeArquivo, "wb");
    if (!file) return 0;

    // 1. Salva dados do Mapa (Grade, Spawns, Quantidades)
    fwrite(&mapa->linhas, sizeof(int), 1, file);
    fwrite(&mapa->colunas, sizeof(int), 1, file);
    // TODO: salvar pacmans e fantasmos
    fwrite(&mapa->qtdPortais, sizeof(int), 1, file);
    fwrite(&mapa->qtdPointPellets, sizeof(int), 1, file);

    // 2. Salva a grade (Contém quais pastilhas já foram comidas)
    for (int i = 0; i < mapa->linhas; i++) {
        fwrite(mapa->grade[i], sizeof(char), mapa->colunas, file);
    }

    // 3. Salva vetores dinâmicos (Portais e Conexões)
    if (mapa->qtdPortais > 0) {
        fwrite(mapa->portais, sizeof(Posicao), mapa->qtdPortais, file);
        fwrite(mapa->conexoes, sizeof(int), mapa->qtdPortais, file);
    }

    // 4. [NOVO] Salva o Estado Dinâmico (Posição Atual do Pacman e Fantasmas)
    fwrite(&pacmanAtual, sizeof(Posicao), 1, file);
    fwrite(&qtdFantasmas, sizeof(int), 1, file);
    if (qtdFantasmas > 0 && fantasmas != NULL) {
        fwrite(fantasmas, sizeof(Posicao), qtdFantasmas, file);
    }

    fclose(file);
    return 1;
}

// Carrega o jogo e retorna as posições salvas via ponteiros (outPacman, outFantasmas)
Mapa* CarregarEstadoMapa(const char *nomeArquivo, Posicao *outPacman, Posicao **outFantasmas, int *outQtdFantasmas) {
    FILE *file = fopen(nomeArquivo, "rb");
    if (!file) return NULL;

    Mapa *mapa = (Mapa*)malloc(sizeof(Mapa));
    
    // Lê dados básicos
    fread(&mapa->linhas, sizeof(int), 1, file);
    fread(&mapa->colunas, sizeof(int), 1, file);
    fread(&mapa->qtdPortais, sizeof(int), 1, file);
    fread(&mapa->qtdPointPellets, sizeof(int), 1, file);

    // Aloca e Lê a grade
    mapa->grade = (char**)malloc(mapa->linhas * sizeof(char*));
    for (int i = 0; i < mapa->linhas; i++) {
        mapa->grade[i] = (char*)malloc((mapa->colunas + 2) * sizeof(char));
        fread(mapa->grade[i], sizeof(char), mapa->colunas, file);
        mapa->grade[i][mapa->colunas] = '\0';
    }

    // Aloca e Lê Portais
    mapa->portais = NULL;
    mapa->conexoes = NULL;
    if (mapa->qtdPortais > 0) {
        mapa->portais = (Posicao *)malloc(mapa->qtdPortais * sizeof(Posicao));
        fread(mapa->portais, sizeof(Posicao), mapa->qtdPortais, file);

        mapa->conexoes = (int *)malloc(mapa->qtdPortais * sizeof(int));
        fread(mapa->conexoes, sizeof(int), mapa->qtdPortais, file);
    }

    // Lê as Posições Dinâmicas e retorna pelos ponteiros
    if (outPacman) {
        fread(outPacman, sizeof(Posicao), 1, file);
    } else {
        // Se o usuário não pediu (passou NULL), lê para descartar
        Posicao temp; 
        fread(&temp, sizeof(Posicao), 1, file);
    }

    if (outQtdFantasmas && outFantasmas) {
        fread(outQtdFantasmas, sizeof(int), 1, file);
        if (*outQtdFantasmas > 0) {
            *outFantasmas = (Posicao *)malloc((*outQtdFantasmas) * sizeof(Posicao));
            fread(*outFantasmas, sizeof(Posicao), *outQtdFantasmas, file);
        } else {
            *outFantasmas = NULL;
        }
    }

    fclose(file);
    return mapa;
}
*/

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

            if (mapa->pacman.pos.x == x && mapa->pacman.pos.y == y) {
                DesenharSpriteOuForma(sprites->pacman, px, py, YELLOW, 1, 1.0f, WHITE, tamanhoBloco);
                continue;
            }
            bool f = false;
            for (int i = 0; i < mapa->numero_fantasmas; i++) {
                if (mapa->fantasmas[i].pos.x == x && mapa->fantasmas[i].pos.y == y) {
                    DesenharSpriteOuForma(sprites->fantasma, px, py, RED, 1, 1.0f, WHITE, tamanhoBloco);
                    f = true;
                    break;
                }
            }
            if (f)
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

// TODO: essa função não está complexa demais?
/*Função para calcular a conexão dos portais. O numero de portais nunca pode ser ímpar. Caso haja mais de um par de portal, 
o portal ira se conectar aquele que estiver na linha paralela a ele, no caso 
a oposta, se tiver mais de um portal nessa linha, escolhe algum dos 2 aleatoriamente. 
Se não houver portal na linha paralela, escolhe o mais distante, que nao esteja na mesma linha. 
Se não há portal que esteja em outra linha a nao ser a que o proprio portal está, ele escolhe esse portal. 
Um portal nao pode se conectar a outro portal que ja tenha um outro portal conectar. 
Ou seja, se T1-T2, T3 nao poderá se conectar a T1 e nem a T2, pois eles ja tem um par
*/
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
        
        // Linha Paralela Oposta
        // Linha oposta em matriz 0-based é (Total - 1 - Y)
        int linhaOposta = (mapa->linhas - 1) - mapa->portais[i].y;
        
        // Busca candidatos na linha oposta
        int candidatosOpostos[mapa->qtdPortais];
        int qtdCandidatos = 0;

        for (int j = 0; j < mapa->qtdPortais; j++) {
            // Ignora a si mesmo e portais já conectados
            if (i == j || mapa->conexoes[j] != -1) continue;

            if (mapa->portais[j].y == linhaOposta) {
                candidatosOpostos[qtdCandidatos++] = j;
            }
        }

        // Se achou candidatos na linha oposta
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
                
                // Prioridade: Não estar na mesma linha
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

        // Realiza a conexão mútua
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

