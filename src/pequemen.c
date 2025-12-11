#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

#include "tipos.h"
#include "menus.h"
#include "mapa.h"
#include "hud.h"
#include "entidades.h"


int main(void) {
    srand((unsigned int)time(NULL));

    Mapa *mapaAtual = CarregarMapa("mapa1.txt");
    if (!mapaAtual) {
        fprintf(stderr, "Erro ao carregar mapa!\n");
        return 1;
    }

    EstadoJogo stats;
    InicializarEstadoJogo(&stats);
    // Atualiza a quantidade total de pellets para controle de vitória
    stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);

    int tamanho_bloco = 20;

    InitWindow(tamanho_bloco * 40, tamanho_bloco * 21, "Pequemen");
    SetTargetFPS(60);

    Sprites sprites;
    CarregarSprites(&sprites);

    Tela tela = TELA_MENU_PRINCIPAL;

    while (!WindowShouldClose()) {
        
        BeginDrawing();
        ClearBackground(BLACK);

        if (tela == TELA_SAIR)
            break;

        switch (tela) {
        case TELA_MENU_PRINCIPAL:
            tela = tela_menu_principal();
            break;

        case TELA_RESOLUCAO_MENU_PRINCIPAL:
            tela = tela_resolucao_menu_principal(&tamanho_bloco);
            break;

        case TELA_MENU:
            tela = tela_menu();
            break;

        case TELA_JOGO:
            DesenharMapa(mapaAtual, &sprites, tamanho_bloco);
            DesenharHUD(&stats, tamanho_bloco * 40, tamanho_bloco * 21, tamanho_bloco);

            AtualizarJogo(mapaAtual);
            if (IsKeyPressed(KEY_TAB))
                tela = TELA_MENU;
            break;
        }

        EndDrawing();
    }

    DescarregarSprites(&sprites);
    LiberarMapa(mapaAtual);

    CloseWindow();
    return 0;
}
