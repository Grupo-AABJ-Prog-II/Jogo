#include "menus.h"
// --- MAIN ---
// No seu arquivo principal .c

int main(void) {
    // Começa com um tamanho padrão
    InitWindow(800, 600, "Pac-Man Resizable");
    SetTargetFPS(60);
    SetWindowMinSize(320, 240); // Impede que diminuam demais a janela

    GameScreen currentScreen = SCREEN_TITLE; // Use SCREEN_TITLE se for o outro arquivo
    bool exitGame = false;

    // Variável para saber se estamos no menu de vídeo
    bool inVideoMenu = false; 

    while (!WindowShouldClose() && !exitGame) {
        
        BeginDrawing();
        ClearBackground(BLACK);

        if (!inVideoMenu) {
            // --- MENU PRINCIPAL ---
            DrawCenteredText("MENU PRINCIPAL", 100, 40, YELLOW);

            if (DrawCenteredButton("Novo Jogo", 250, 30, WHITE, GREEN)) {
                // Iniciar jogo...
            }
            
            // Botão que leva para as opções de vídeo
            if (DrawCenteredButton("Opcoes de Video", 300, 30, WHITE, BLUE)) {
                inVideoMenu = true;
            }

            if (DrawCenteredButton("Sair", 350, 30, LIGHTGRAY, RED)) {
                exitGame = true;
            }
        } 
        else {
            // --- MENU DE RESOLUÇÃO ---
            DrawCenteredText("SELECIONE A RESOLUCAO", 100, 30, SKYBLUE);
            
            // Opção 1: 800x600
            if (DrawCenteredButton("800 x 600", 200, 20, WHITE, GREEN)) {
                SetGameResolution(800, 600);
            }

            // Opção 2: HD (1280x720)
            if (DrawCenteredButton("1280 x 720 (HD)", 240, 20, WHITE, GREEN)) {
                SetGameResolution(1280, 720);
            }

            // Opção 3: Full HD (1920x1080)
            if (DrawCenteredButton("1920 x 1080 (FHD)", 280, 20, WHITE, GREEN)) {
                SetGameResolution(1920, 1080);
            }

            // Opção 4: Tela Cheia
            const char* textFull = IsWindowFullscreen() ? "Janela [X]" : "Tela Cheia [ ]";
            if (DrawCenteredButton(textFull, 340, 20, ORANGE, RED)) {
                ToggleFullscreen();
            }

            // Voltar
            if (DrawCenteredButton("Voltar", 450, 30, GRAY, WHITE)) {
                inVideoMenu = false;
            }
            
            // Mostra resolução atual no rodapé
            DrawText(TextFormat("%dx%d", GetScreenWidth(), GetScreenHeight()), 10, GetScreenHeight() - 20, 10, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
