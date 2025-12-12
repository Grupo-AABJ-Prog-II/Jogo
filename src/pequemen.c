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

    // Carrega nível 1 por padrão
    int nivelAtual = 1;
    Mapa *mapaAtual = CarregarMapaNivel(nivelAtual);
    if (!mapaAtual) {
        fprintf(stderr, "Erro critico: mapa1.txt nao encontrado.\n");
        return 1;
    }

    EstadoJogo stats;
    InicializarEstadoJogo(&stats);
    stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);
    stats.nivel = mapaAtual->nivelAtual;

    int tamanho_bloco = 20;
    InitWindow(tamanho_bloco * 40, tamanho_bloco * 21, "Pequemen");
    SetTargetFPS(60);

    Sprites sprites;
    CarregarSprites(&sprites);

    Tela tela = TELA_MENU_PRINCIPAL;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (tela == TELA_SAIR) break;

        switch (tela) {
            case TELA_MENU_PRINCIPAL:
            {
                Tela acao = tela_menu_principal();
                if (acao == TELA_JOGO) {
                    // Novo Jogo: Reseta
                    LiberarMapa(mapaAtual);
                    mapaAtual = CarregarMapaNivel(1); // Reinicia do mapa 1
                    if (!mapaAtual) { tela = TELA_SAIR; break; }
                    
                    InicializarEstadoJogo(&stats);
                    stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);
                    stats.nivel = 1;
                    tela = TELA_JOGO;
                } 
                else if (acao == TELA_SAIR) tela = TELA_SAIR;
                else if (acao == TELA_RESOLUCAO_MENU_PRINCIPAL) tela = TELA_RESOLUCAO_MENU_PRINCIPAL;
                else if (acao == CARREGAR){
                    Mapa *carregado = CarregarJogo("savegame.bin");
                    if (carregado) {
                        LiberarMapa(mapaAtual);
                        mapaAtual = carregado;
                        tela = TELA_JOGO;
                    }
                }
                break;
            }

            case TELA_RESOLUCAO_MENU_PRINCIPAL:
                tela = tela_resolucao_menu_principal(&tamanho_bloco);
                break;

            case TELA_MENU:
                Tela acao2 = tela_menu();
                if (acao2 == SALVAR) SalvarJogo(mapaAtual, "savegame.bin"); tela = TELA_MENU;
                if (acao2 == CARREGAR) {
                    Mapa *c = CarregarJogo("savegame.bin");
                    if(c) { LiberarMapa(mapaAtual); mapaAtual=c; tela=TELA_JOGO; }
                }
                else{tela = acao2;}
                break;

            case TELA_JOGO:
                // Sincroniza HUD
                stats.pontuacao = mapaAtual->pacman.score;
                stats.vidas = mapaAtual->pacman.vidas;
                stats.pelletsRestantes = ContarPastilhasRestantes(mapaAtual);
                stats.nivel = mapaAtual->nivelAtual;

                DesenharMapa(mapaAtual, &sprites, tamanho_bloco);
                DesenharHUD(&stats, GetScreenWidth(), GetScreenHeight(), tamanho_bloco * 1.5);

                Tela resultado = AtualizarJogo(mapaAtual);
                
                if (resultado == TELA_GAMEOVER) {
                    tela = TELA_GAMEOVER;
                } 
                else if (resultado == TELA_VITORIA) {
                    // Passou de nível! Tenta carregar o próximo.
                    int proxNivel = mapaAtual->nivelAtual + 1;
                    int scoreSalvo = mapaAtual->pacman.score;
                    int vidasSalvas = mapaAtual->pacman.vidas;

                    Mapa *proxMapa = CarregarMapaNivel(proxNivel);
                    
                    if (proxMapa) {
                        // Existe próximo nível
                        LiberarMapa(mapaAtual);
                        mapaAtual = proxMapa;
                        // Restaura status do jogador no novo mapa
                        mapaAtual->pacman.score = scoreSalvo;
                        mapaAtual->pacman.vidas = vidasSalvas;
                        tela = TELA_JOGO; 
                    } else {
                        // Não existe mais mapas -> Zerou o jogo!
                        tela = TELA_VITORIA_FINAL;
                    }
                }
                
                if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_P)) tela = TELA_MENU;
                break;

            case TELA_GAMEOVER:
                tela = tela_gameover(mapaAtual->pacman.score);
                break;

            case TELA_VITORIA:
                // Pode exibir uma tela intermediária ou já ter sido tratada na lógica do TELA_JOGO
                // Aqui é só desenhar se cair neste estado
                tela = tela_vitoria(mapaAtual->pacman.score);
                break;

            case TELA_VITORIA_FINAL:
                tela = tela_vitoria_final(mapaAtual->pacman.score);
                break;
                
            case TELA_SAIR:
                // Apenas para o switch não reclamar, o break do while já trata isso
                break;
        }

        EndDrawing();
    }

    DescarregarSprites(&sprites);
    LiberarMapa(mapaAtual);
    CloseWindow();
    return 0;

}

