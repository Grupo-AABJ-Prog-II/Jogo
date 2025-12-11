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

    // Carrega o mapa inicial (apenas para ter algo na memória)
    Mapa *mapaAtual = CarregarMapa("mapa1.txt");
    if (!mapaAtual) {
        fprintf(stderr, "Erro critico: Nao foi possivel iniciar o jogo sem o mapa.\n");
        return 1;
    }

    EstadoJogo stats;
    InicializarEstadoJogo(&stats);
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
            {
                Tela novaTela = tela_menu_principal();
                
                // Se clicou em "Novo Jogo", reseta o mapa
                if (novaTela == TELA_JOGO && tela != TELA_JOGO) {
                    LiberarMapa(mapaAtual);
                    mapaAtual = CarregarMapa("mapa1.txt");
                    if (!mapaAtual) { tela = TELA_SAIR; break; }
                    
                    InicializarEstadoJogo(&stats);
                    stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);
                }
                
                tela = novaTela;
                break;
            }

            case TELA_RESOLUCAO_MENU_PRINCIPAL:
                tela = tela_resolucao_menu_principal(&tamanho_bloco);
                break;

            case TELA_MENU:
                tela = tela_menu();
                break;

            case TELA_JOGO:
                // Sincroniza dados do mapa com a HUD antes de desenhar
                stats.pontuacao = mapaAtual->pacman.score;
                stats.vidas = mapaAtual->pacman.vidas;
                stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);

                DesenharMapa(mapaAtual, &sprites, tamanho_bloco);
                DesenharHUD(&stats, GetScreenWidth(), GetScreenHeight(), tamanho_bloco * 1.5); // Altura da HUD ajustada

                tela = AtualizarJogo(mapaAtual);
                
                if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_P))
                    tela = TELA_MENU;
                break;

            case TELA_GAMEOVER:
                tela = tela_gameover(mapaAtual->pacman.score);
                break;

            case TELA_VITORIA:
                tela = tela_vitoria(mapaAtual->pacman.score);
                break;

            default:
                break;
        }

        EndDrawing();
    }

    DescarregarSprites(&sprites);
    LiberarMapa(mapaAtual);

    CloseWindow();
    return 0;
}