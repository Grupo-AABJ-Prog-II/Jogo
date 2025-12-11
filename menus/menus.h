#ifndef GAME_UTILS_H
#define GAME_UTILS_H

#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>

// --- Configurações Globais ---
#define SAVE_FILENAME "pacman_save.dat"

// --- Estados do Jogo (Unificado) ---
// Juntei os estados dos dois arquivos para você poder navegar entre todos
typedef enum GameScreen {
    SCREEN_TITLE,   // Tela Inicial (Menu_Inicial.c)
    SCREEN_MENU,    // Menu de Opções (Menu_Opcoes.c)
    SCREEN_GAME     // O Jogo em si
} GameScreen;

// --- Dados do Jogo ---
typedef struct GameState {
    int score;
    int lives;
    int currentLevel;
} GameState;

// --- Protótipos das Funções (Declarações) ---
// Apenas dizemos ao compilador que essas funções existem.
// Gerenciamento de Estado
void InitNewGame(GameState *state) {
    state->score = 0;
    state->lives = 3;
    state->currentLevel = 1;
    TraceLog(LOG_INFO, "Novo jogo iniciado.");
}

void SaveGameBinary(const GameState *state) {
    FILE *file = fopen(SAVE_FILENAME, "wb");
    if (file != NULL) {
        fwrite(state, sizeof(GameState), 1, file);
        fclose(file);
        TraceLog(LOG_INFO, "Jogo salvo com sucesso.");
    }
}

bool LoadGameBinary(GameState *state) {
    FILE *file = fopen(SAVE_FILENAME, "rb");
    if (file != NULL) {
        size_t readCount = fread(state, sizeof(GameState), 1, file);
        fclose(file);
        return (readCount == 1);
    }
    return false;
}

// --- SOLUÇÃO DO ERRO: Função em vez de Macro ---
// Esta função substitui o DRAW_MENU_ITEM. 
// Como é uma função, a variável 'isHovering' existe apenas aqui dentro,
// evitando o erro de "redefinição".
bool DrawClickableText(const char *text, int x, int y, int fontSize, Color colorNormal, Color colorHover) {
    int textWidth = MeasureText(text, fontSize);
    Rectangle bounds = { (float)x, (float)y, (float)textWidth, (float)fontSize };
    
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, bounds);
    
    // Sombra simples
    DrawText(text, x + 2, y + 2, fontSize, BLACK);
    
    if (isHovering) {
        DrawText(text, x, y, fontSize, colorHover);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return true;
    } else {
        DrawText(text, x, y, fontSize, colorNormal);
    }
    return false;
}

// ... (mantenha as funções InitNewGame, SaveGameBinary, etc. que já existiam)

// --- Novas Implementações ---

void SetGameResolution(int width, int height) {
    // 1. Aplica o novo tamanho
    SetWindowSize(width, height);
    
    // 2. Centraliza a janela no monitor do usuário
    int monitor = GetCurrentMonitor();
    int monitorW = GetMonitorWidth(monitor);
    int monitorH = GetMonitorHeight(monitor);
    
    SetWindowPosition((monitorW - width) / 2, (monitorH - height) / 2);
    
    TraceLog(LOG_INFO, "Resolucao alterada para: %dx%d", width, height);
}


// Função auxiliar para não precisar calcular X na mão toda vez
void DrawCenteredText(const char* text, int y, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    // Fórmula do centro: (LarguraDaTela - LarguraDoTexto) / 2
    int x = (GetScreenWidth() - textWidth) / 2; 
    DrawText(text, x, y, fontSize, color);
}

// Versão do botão que se auto-centraliza
bool DrawCenteredButton(const char* text, int y, int fontSize, Color normal, Color hover) {
    int textWidth = MeasureText(text, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2;
    
    // Reaproveita sua lógica de botão, mas com o X calculado
    return DrawClickableText(text, x, y, fontSize, normal, hover);
}
#endif // GAME_UTILS_H