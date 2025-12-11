#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 
#include <time.h>
#include <raylib.h> // Necessário para GetTime()

#include "tipos.h"
#include "mapa.h"

// Declarações locais
void TentarGerarPointPellets(Mapa *mapa);
void CalcularSpawnDistante(Mapa *mapa);
void DesenharSpriteOuForma(Texture2D tex, int x, int y, Color cor, int ehCirculo, float escala, Color tint, int tamanhoBloco, float rotacao);
void ConectarPortais(Mapa *mapa);
void FloodFill(Mapa *mapa, int x, int y, int **visited); 

// Função auxiliar para carregar mapa pelo número
Mapa* CarregarMapaNivel(int nivel) {
    char nomeArquivo[50];
    sprintf(nomeArquivo, "mapa%d.txt", nivel);
    
    // Tenta abrir apenas para ver se existe
    FILE *teste = fopen(nomeArquivo, "r");
    if (!teste) return NULL; // Se não existir, retorna NULL (fim dos níveis)
    fclose(teste);

    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) return NULL;

    Mapa *mapa = (Mapa*)calloc(1, sizeof(Mapa));
    
    mapa->nivelAtual = nivel;
    mapa->jogoIniciado = false; 
    mapa->linhas = 20; 
    mapa->colunas = 40;
    mapa->pacman.vidas = 3; 
    
    mapa->grade = malloc(mapa->linhas * sizeof(char*));
    for (int i = 0; i < mapa->linhas; i++) {
        mapa->grade[i] = malloc((mapa->colunas + 2) * sizeof(char));
    }

    char linha[256]; 
    int l = 0;
    int pacmanCount = 0;

    while (l < mapa->linhas) {
        if (!fgets(linha, sizeof(linha), arquivo)) break;
        linha[strcspn(linha, "\r\n")] = 0;
        
        for (int c = 0; c < mapa->colunas; c++) {
            char ch = linha[c];
            mapa->grade[l][c] = ch;
            
            if (ch == 'P') { 
                mapa->pacman.spawnPos = (Posicao){c, l};
                mapa->pacman.pos = mapa->pacman.spawnPos;
                // Inicializa posição visual igual à lógica
                mapa->pacman.pixelPos = (Vector2){(float)c, (float)l};
                mapa->grade[l][c] = '.'; 
                pacmanCount++; 
            }
            if (ch == 'T') {
                mapa->qtdPortais++;
                mapa->portais = realloc(mapa->portais, mapa->qtdPortais * sizeof(Posicao));
                mapa->portais[mapa->qtdPortais - 1] = (Posicao){c, l};
            }
            if (ch == 'F') {
                mapa->grade[l][c] = '.'; 
                mapa->numero_fantasmas++;
                mapa->fantasmas = realloc(mapa->fantasmas, mapa->numero_fantasmas * sizeof(Fantasma));
                Fantasma *f = &mapa->fantasmas[mapa->numero_fantasmas - 1];
                f->pos = (Posicao){c, l};
                f->pixelPos = (Vector2){(float)c, (float)l}; // Inicializa visual
                f->spawnPosOriginal = f->pos;
                f->moveTimer = 0; // Inicializa timer zerado
                f->tempoVulneravel = 0;
                f->estaVulneravel = false;
            }
            if (ch == '+') mapa->qtdPointPellets++;
        }
        l++;
    }
    fclose(arquivo);

    if (pacmanCount != 1) { LiberarMapa(mapa); return NULL; }
    if (mapa->qtdPortais % 2 != 0) { LiberarMapa(mapa); return NULL; }

    if (mapa->qtdPortais > 0) ConectarPortais(mapa);
    TentarGerarPointPellets(mapa);
    CalcularSpawnDistante(mapa);
    
    return mapa;
}

// Wrapper para manter compatibilidade com código antigo se necessário
Mapa* CarregarMapa(const char *nomeArquivo) {
    // Tenta extrair o número do nível do nome do arquivo, senão assume 1
    int nivel = 1;
    if (sscanf(nomeArquivo, "mapa%d.txt", &nivel) != 1) {
        // Se não conseguiu ler o numero (ex: mapa_teste.txt), tenta carregar direto
        // Mas como nossa logica mudou, vamos forçar nivel 1 se falhar
        nivel = 1;
    }
    return CarregarMapaNivel(nivel);
}

// --- SISTEMA DE SAVE / LOAD ---

bool SalvarJogo(Mapa *mapa, const char *nomeArquivo) {
    FILE *f = fopen(nomeArquivo, "wb");
    if (!f) return false;

    // 1. Dados Básicos e Estado
    fwrite(&mapa->nivelAtual, sizeof(int), 1, f);
    fwrite(&mapa->linhas, sizeof(int), 1, f);
    fwrite(&mapa->colunas, sizeof(int), 1, f);
    
    // IMPORTANTE: Ao salvar, definimos jogoIniciado como false para que, ao carregar,
    // o jogador precise dar um input antes dos fantasmas se moverem.
    // Isso evita morte instantânea ao carregar se estiver cercado.
    bool iniciado = false; 
    fwrite(&iniciado, sizeof(bool), 1, f);
    
    // 2. Pacman (Score, Vidas, Posição)
    fwrite(&mapa->pacman, sizeof(Pacman), 1, f);

    // 3. Grade (Onde estão as pellets restantes)
    for (int i = 0; i < mapa->linhas; i++) {
        fwrite(mapa->grade[i], sizeof(char), mapa->colunas, f);
    }

    // 4. Fantasmas (Quantidade e Lista)
    fwrite(&mapa->numero_fantasmas, sizeof(int), 1, f);
    if (mapa->numero_fantasmas > 0) {
        fwrite(mapa->fantasmas, sizeof(Fantasma), mapa->numero_fantasmas, f);
    }

    // 5. Portais (Quantidade e Listas)
    fwrite(&mapa->qtdPortais, sizeof(int), 1, f);
    if (mapa->qtdPortais > 0) {
        fwrite(mapa->portais, sizeof(Posicao), mapa->qtdPortais, f);
        fwrite(mapa->conexoes, sizeof(int), mapa->qtdPortais, f);
    }
    
    // 6. Spawn Calculado
    fwrite(&mapa->spawnFantasma, sizeof(Posicao), 1, f);
    fwrite(&mapa->temSpawn, sizeof(int), 1, f);

    fclose(f);
    printf("Jogo salvo com sucesso em %s\n", nomeArquivo);
    return true;
}

Mapa* CarregarJogo(const char *nomeArquivo) {
    FILE *f = fopen(nomeArquivo, "rb");
    if (!f) return NULL;

    Mapa *m = (Mapa*)calloc(1, sizeof(Mapa));

    // 1. Lê dados básicos
    fread(&m->nivelAtual, sizeof(int), 1, f);
    fread(&m->linhas, sizeof(int), 1, f);
    fread(&m->colunas, sizeof(int), 1, f);
    fread(&m->jogoIniciado, sizeof(bool), 1, f);

    // 2. Pacman
    fread(&m->pacman, sizeof(Pacman), 1, f);
    
    // [CORREÇÃO CRÍTICA] Resetar o timer de movimento ao carregar!
    // O valor salvo em 'moveTimer' é referente ao tempo de execução da sessão anterior.
    // Se não resetarmos para GetTime(), o jogo pode achar que precisa esperar horas (ou o contrário).
    m->pacman.moveTimer = 0; // Isso força o movimento a ser processado imediatamente no próximo loop se o tempo bater, ou sincroniza.
    // Melhor ainda: definir como GetTime() atual para reiniciar o ciclo
    // Mas como a lógica de movimento verifica (GetTime() - moveTimer >= delay), 
    // se moveTimer for 0 e GetTime() for alto, ele move instantaneamente.
    // Vamos deixar 0 para garantir resposta imediata no primeiro frame.

    // Recalcula pixelPos para evitar glitch visual ao carregar (teleporte suave)
    m->pacman.pixelPos.x = (float)m->pacman.pos.x;
    m->pacman.pixelPos.y = (float)m->pacman.pos.y;
    
    // Reseta direção próxima para evitar movimento indesejado imediato
    m->pacman.proxDir = (Posicao){0, 0};

    // 3. Grade
    m->grade = malloc(m->linhas * sizeof(char*));
    for (int i = 0; i < m->linhas; i++) {
        m->grade[i] = malloc((m->colunas + 1) * sizeof(char));
        fread(m->grade[i], sizeof(char), m->colunas, f);
        m->grade[i][m->colunas] = '\0';
    }

    // 4. Fantasmas
    fread(&m->numero_fantasmas, sizeof(int), 1, f);
    if (m->numero_fantasmas > 0) {
        m->fantasmas = malloc(m->numero_fantasmas * sizeof(Fantasma));
        fread(m->fantasmas, sizeof(Fantasma), m->numero_fantasmas, f);
        
        // Atualiza pixelPos dos fantasmas e reseta timers
        for(int i=0; i<m->numero_fantasmas; i++) {
            m->fantasmas[i].pixelPos.x = (float)m->fantasmas[i].pos.x;
            m->fantasmas[i].pixelPos.y = (float)m->fantasmas[i].pos.y;
            m->fantasmas[i].moveTimer = 0; // Resetar timer também!
            
            // Se o jogo foi salvo com fantasmas vulneráveis, precisamos ajustar o tempo relativo
            // O tempoVulneravel salvo é absoluto da sessão anterior.
            // Se quisermos manter a vulnerabilidade, teríamos que salvar "quanto tempo falta" e somar ao GetTime() atual.
            // Para simplificar e evitar bugs, vamos remover a vulnerabilidade ao carregar.
            m->fantasmas[i].estaVulneravel = false;
            m->fantasmas[i].tempoVulneravel = 0;
        }
    }

    // 5. Portais
    fread(&m->qtdPortais, sizeof(int), 1, f);
    if (m->qtdPortais > 0) {
        m->portais = malloc(m->qtdPortais * sizeof(Posicao));
        fread(m->portais, sizeof(Posicao), m->qtdPortais, f);
        
        m->conexoes = malloc(m->qtdPortais * sizeof(int));
        fread(m->conexoes, sizeof(int), m->qtdPortais, f);
    }

    // 6. Spawn Point
    fread(&m->spawnFantasma, sizeof(Posicao), 1, f);
    fread(&m->temSpawn, sizeof(int), 1, f);

    fclose(f);
    return m;
}

int ContarPastilhasRestantes(Mapa *mapa) {
    int count = 0;
    for (int y = 0; y < mapa->linhas; y++) {
        for (int x = 0; x < mapa->colunas; x++) {
            char c = mapa->grade[y][x];
            if (c == '.' || c == 'o' || c == '+') count++;
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
    // Desenha o cenário (estático)
    for (int y = 0; y < mapa->linhas; y++) {
        for (int x = 0; x < mapa->colunas; x++) {
            int px = x * tamanhoBloco;
            int py = y * tamanhoBloco;
            char tile = mapa->grade[y][x];

            switch (tile) {
                case '#': DesenharSpriteOuForma(sprites->parede, px, py, BLUE, 0, 1.0f, WHITE, tamanhoBloco, 0.0f); break;
                case '.': DesenharSpriteOuForma(sprites->pastilha, px, py, WHITE, 1, 0.2f, WHITE, tamanhoBloco, 0.0f); break;
                case 'o': DesenharSpriteOuForma(sprites->superPastilha, px, py, GREEN, 1, 0.6f, WHITE, tamanhoBloco, 0.0f); break;
                case '+': DesenharSpriteOuForma(sprites->superPastilha, px, py, GOLD, 1, 0.6f, GOLD, tamanhoBloco, 0.0f); break;
                case 'T': DesenharSpriteOuForma(sprites->portal, px, py, PINK, 0, 1.0f, WHITE, tamanhoBloco, 0.0f); break;
            }
        }
    }

    // Desenha Pacman (Usando pixelPos para movimento fluido)
    int pacX = (int)(mapa->pacman.pixelPos.x * tamanhoBloco);
    int pacY = (int)(mapa->pacman.pixelPos.y * tamanhoBloco);
    
    float angulo = 0.0f;
    if (mapa->pacman.dir.x == 1) angulo = 0.0f;
    else if (mapa->pacman.dir.x == -1) angulo = 180.0f;
    else if (mapa->pacman.dir.y == 1) angulo = 90.0f;
    else if (mapa->pacman.dir.y == -1) angulo = 270.0f;
    
    DesenharSpriteOuForma(sprites->pacman, pacX, pacY, YELLOW, 1, 1.0f, WHITE, tamanhoBloco, angulo);

    // Desenha Fantasmas (Usando pixelPos para movimento fluido)
    for (int i = 0; i < mapa->numero_fantasmas; i++) {
        int fantX = (int)(mapa->fantasmas[i].pixelPos.x * tamanhoBloco);
        int fantY = (int)(mapa->fantasmas[i].pixelPos.y * tamanhoBloco);
        
        Color cor = RED;
        Color tint = WHITE;
        if (mapa->fantasmas[i].estaVulneravel) {
            if (fmod(GetTime(), 0.4) < 0.2) { cor = BLUE; tint = BLUE; }
            else { cor = WHITE; tint = WHITE; }
        }
        DesenharSpriteOuForma(sprites->fantasma, fantX, fantY, cor, 1, 1.0f, tint, tamanhoBloco, 0.0f);
    }
}

// Implementações auxiliares
double CalcularDistancia(Posicao p1, Posicao p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

void FloodFill(Mapa *mapa, int x, int y, int **visited) {
    if (x < 0 || x >= mapa->colunas || y < 0 || y >= mapa->linhas) return;
    if (visited[y][x] || mapa->grade[y][x] == '#') return;
    visited[y][x] = 1;
    FloodFill(mapa, x + 1, y, visited);
    FloodFill(mapa, x - 1, y, visited);
    FloodFill(mapa, x, y + 1, visited);
    FloodFill(mapa, x, y - 1, visited);
    if (mapa->grade[y][x] == 'T') {
        for(int i = 0; i < mapa->qtdPortais; i++) {
            if (mapa->portais[i].x == x && mapa->portais[i].y == y) {
                int destIdx = mapa->conexoes[i];
                if (destIdx != -1) {
                    Posicao dest = mapa->portais[destIdx];
                    FloodFill(mapa, dest.x, dest.y, visited);
                }
                break;
            }
        }
    }
}

void DesenharSpriteOuForma(Texture2D tex, int x, int y, Color cor, int ehCirculo, float escala, Color tint, int tamanhoBloco, float rotacao) {
    float meio = tamanhoBloco / 2.0f;
    if (tex.id > 0) {
        Rectangle fonte = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle dest = { (float)x + meio, (float)y + meio, (float)tamanhoBloco, (float)tamanhoBloco };
        DrawTexturePro(tex, fonte, dest, (Vector2){meio, meio}, rotacao, tint);
    } else {
        int cx = x + (int)meio;
        int cy = y + (int)meio;
        if (ehCirculo) DrawCircle(cx, cy, (meio-2)*escala, cor);
        else {
            DrawRectangle(x, y, tamanhoBloco, tamanhoBloco, cor);
            if (cor.r == BLUE.r) DrawRectangleLines(x, y, tamanhoBloco, tamanhoBloco, DARKBLUE);
        }
    }
}

void ConectarPortais(Mapa *mapa) {
    mapa->conexoes = malloc(mapa->qtdPortais * sizeof(int));
    for(int i=0; i<mapa->qtdPortais; i++) mapa->conexoes[i] = -1;

    for(int i=0; i<mapa->qtdPortais; i++) {
        if(mapa->conexoes[i] != -1) continue;
        int melhorPar = -1;
        int linhaOposta = (mapa->linhas - 1) - mapa->portais[i].y;
        
        int *candidatos = malloc(mapa->qtdPortais * sizeof(int));
        int qtd = 0;

        for(int j=0; j<mapa->qtdPortais; j++) {
            if(i == j || mapa->conexoes[j] != -1) continue;
            if(mapa->portais[j].y == linhaOposta) candidatos[qtd++] = j;
        }

        if(qtd > 0) {
            melhorPar = candidatos[rand() % qtd];
        } else {
            double maxDist = -1.0;
            for(int j=0; j<mapa->qtdPortais; j++) {
                if(i == j || mapa->conexoes[j] != -1) continue;
                double d = CalcularDistancia(mapa->portais[i], mapa->portais[j]);
                if(d > maxDist) { maxDist = d; melhorPar = j; }
            }
        }
        free(candidatos);
        if(melhorPar != -1) {
            mapa->conexoes[i] = melhorPar;
            mapa->conexoes[melhorPar] = i;
        }
    }
}

void TentarGerarPointPellets(Mapa *mapa) {
    if (mapa->qtdPointPellets > 0) return;
    for (int y=0; y<mapa->linhas; y++) {
        for (int x=0; x<mapa->colunas; x++) {
            if (mapa->grade[y][x] == 'o') {
                if (((double)rand() / RAND_MAX) * 100.0 <= 2.54345) {
                    mapa->grade[y][x] = '+';
                    mapa->qtdPointPellets++;
                }
            }
        }
    }
}

void CalcularSpawnDistante(Mapa *mapa) {
    double distMax = -1.0;
    Posicao melhorPos = {1, 1};
    int achou = 0;
    for (int y=0; y<mapa->linhas; y++) {
        for (int x=0; x<mapa->colunas; x++) {
            if (mapa->grade[y][x] == 'S') {
                Posicao pos = {x, y};
                double dist = CalcularDistancia(mapa->pacman.spawnPos, pos);
                if (dist > distMax) { distMax = dist; melhorPos = pos; }
                achou = 1;
            }
        }
    }
    mapa->temSpawn = achou;
    if (achou) mapa->spawnFantasma = melhorPos;
    else if (mapa->numero_fantasmas > 0) mapa->spawnFantasma = mapa->fantasmas[0].pos;
}