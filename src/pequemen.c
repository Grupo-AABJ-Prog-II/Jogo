#include <raylib.h>

#include "tipos.h"
#include "menus.h"


int main(void) {
    int tamanho_bloco = 20;

    InitWindow(tamanho_bloco * 40, tamanho_bloco * 21, "Pequemen");
    SetTargetFPS(60);

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
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
