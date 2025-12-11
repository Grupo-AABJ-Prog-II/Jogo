#include "menus.h" // Certifique-se que DrawCenteredButton esta definido aqui

int main(void) {
    // Aumentei um pouco a altura para caber tudo confortavelmente
    const int screenWidth = 1100;
    const int screenHeight = 700;

    // Adicionei a flag de redimensionamento caso queira testar a centralização
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Pac-Man - Menu de Opcoes");
    SetTargetFPS(60);

    Texture2D menuBackground = LoadTexture("FachadaCCMN.jpeg");
    if (menuBackground.id == 0) TraceLog(LOG_WARNING, "Fundo nao encontrado.");

    GameState gameState;
    InitNewGame(&gameState);

    GameScreen currentScreen = SCREEN_MENU;
    bool exitGameRequested = false;

    while (!WindowShouldClose() && !exitGameRequested) {
        
        // Input de Teclado (Mantido)
        if (currentScreen == SCREEN_MENU) {
             if (IsKeyPressed(KEY_N)) { InitNewGame(&gameState); currentScreen = SCREEN_GAME; }
             if (IsKeyPressed(KEY_C)) { if (LoadGameBinary(&gameState)) currentScreen = SCREEN_GAME; }
             if (IsKeyPressed(KEY_S)) { SaveGameBinary(&gameState); }
             if (IsKeyPressed(KEY_Q)) { exitGameRequested = true; }
             if (IsKeyPressed(KEY_V)) { currentScreen = SCREEN_GAME; }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentScreen) {
            case SCREEN_MENU: {
                // Desenha o fundo esticado para caber na tela se ela mudar de tamanho
                DrawTexturePro(menuBackground, 
                               (Rectangle){0, 0, menuBackground.width, menuBackground.height},
                               (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
                               (Vector2){0, 0}, 0.0f, WHITE);

                // --- TÍTULO CENTRALIZADO ---
                // Não usamos mais X=50, apenas a altura Y=50
                DrawCenteredText("2.10. Menu de Opcoes:", 50, 40, YELLOW);

                int startY = 150;     // Altura inicial dos botões
                int spacing = 50;     // Espaço entre eles
                int fontSize = 20;

                // --- BOTÕES CENTRALIZADOS ---
                // Substituímos DrawClickableText por DrawCenteredButton
                // Note que o argumento 'X' sumiu.
                
                if (DrawCenteredButton("- Novo Jogo (N)", startY, fontSize, WHITE, GREEN)) {
                    InitNewGame(&gameState);
                    currentScreen = SCREEN_GAME;
                }

                if (DrawCenteredButton("- Carregar jogo (C)", startY + spacing, fontSize, WHITE, GREEN)) {
                    if (LoadGameBinary(&gameState)) currentScreen = SCREEN_GAME;
                }

                if (DrawCenteredButton("- Salvar jogo (S)", startY + spacing * 2, fontSize, WHITE, GREEN)) {
                    SaveGameBinary(&gameState);
                }

                if (DrawCenteredButton("- Sair do jogo (Q)", startY + spacing * 3, fontSize, WHITE, GREEN)) {
                    exitGameRequested = true;
                }

                if (DrawCenteredButton("- Voltar (V)", startY + spacing * 4, fontSize, WHITE, GREEN)) {
                    currentScreen = SCREEN_GAME;
                }

                DrawCenteredText("(Mouse ou Teclado)", startY + spacing * 6, 20, LIGHTGRAY);
            } break;

            case SCREEN_GAME: {
                ClearBackground(DARKBLUE);
                gameState.score++; 
                
                // --- JOGO CENTRALIZADO ---
                DrawCenteredText("JOGO EM ANDAMENTO...", 200, 30, WHITE);
                
                // Para textos formatados, usamos o TextFormat dentro da função
                DrawCenteredText(TextFormat("Score: %d", gameState.score), 250, 30, YELLOW);
                
                if (DrawCenteredButton("Voltar ao Menu [TAB]", 500, 20, WHITE, GREEN)) {
                    currentScreen = SCREEN_MENU;
                }
                
                if (IsKeyPressed(KEY_TAB)) currentScreen = SCREEN_MENU;
            } break;
        }

        EndDrawing();
    }

    UnloadTexture(menuBackground);
    CloseWindow();
    return 0;
}