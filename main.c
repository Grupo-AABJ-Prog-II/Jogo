#include "raylib.h"
#include "mapa.h" 
#include "hud.h" 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned int)time(NULL));

    // setup do window
    SetConfigFlags(FLAG_WINDOW_HIDDEN); 
    InitWindow(800, 600, "Pac-Man - Jogo Principal");
    
    int monitor = GetCurrentMonitor();
    int monitorW = GetMonitorWidth(monitor);
    int monitorH = GetMonitorHeight(monitor);
    
    // Margens
    int maxW = (int)(monitorW * 0.95);
    int maxH = (int)(monitorH * 0.85) - ALTURA_HUD;

    int blocoW = maxW / COLUNAS_MAPA;
    int blocoH = maxH / LINHAS_MAPA;
    int tamanhoBloco = (blocoW < blocoH) ? blocoW : blocoH;

    if (tamanhoBloco > 40) tamanhoBloco = 40;
    if (tamanhoBloco < 15) tamanhoBloco = 15;

    int larguraJanela = COLUNAS_MAPA * tamanhoBloco;
    int alturaJanela = (LINHAS_MAPA * tamanhoBloco) + ALTURA_HUD;

    SetWindowSize(larguraJanela, alturaJanela);
    SetWindowPosition((monitorW - larguraJanela) / 2, (monitorH - alturaJanela) / 2);
    ClearWindowState(FLAG_WINDOW_HIDDEN);
    SetTargetFPS(60);

    // carregar o arquivo do mapa
    Mapa *mapaAtual = CarregarMapa("mapa_novo_teste.txt");
    
    if (!mapaAtual) {
        printf("Erro ao carregar mapa!\n");
        CloseWindow();
        return 1;
    }

    // Inicializa a HUD (Pontos, Vidas, etc)
    EstadoJogo stats;
    InicializarEstadoJogo(&stats);
    // Atualiza a quantidade total de pellets para controle de vitória
    stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);

    Sprites sprites;
    CarregarSprites(&sprites);

    // loop principal
    while (!WindowShouldClose()) {
        
        // botar aqui a logica do movimento
        // Exemplo: 
        // if (PacmanComeuPellet) { 
        //    AdicionarPontos(&stats, 10); 
        //    stats.pelletsRestantes--; 
        // }

        BeginDrawing();
        ClearBackground(BLACK);

        // Desenha o mapa usando a função do módulo
        DesenharMapa(mapaAtual, &sprites, tamanhoBloco);

        // Desenha a HUD usando o módulo dedicado
        DesenharHUD(&stats, larguraJanela, alturaJanela, ALTURA_HUD);

        EndDrawing();
    }

    // os free
    DescarregarSprites(&sprites);
    LiberarMapa(mapaAtual);
    CloseWindow();
    return 0;
}