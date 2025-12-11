#include <stdbool.h>
#include <raylib.h>
#include "tipos.h"

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

void DrawCenteredText(const char* text, int y, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    // Fórmula do centro: (LarguraDaTela - LarguraDoTexto) / 2
    int x = (GetScreenWidth() - textWidth) / 2; 
    DrawText(text, x, y, fontSize, color);
}

bool DrawCenteredButton(const char* text, int y, int fontSize, Color normal, Color hover) {
    int textWidth = MeasureText(text, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2;
    
    // Reaproveita sua lógica de botão, mas com o X calculado
    return DrawClickableText(text, x, y, fontSize, normal, hover);
}

Tela tela_menu_principal() {
    DrawCenteredText("MENU PRINCIPAL", 100, 40, YELLOW);

    if (DrawCenteredButton("Novo Jogo", 250, 30, WHITE, GREEN))
        return TELA_JOGO;
    
    // Botão que leva para as opções de vídeo
    if (DrawCenteredButton("Opcoes de Video", 300, 30, WHITE, BLUE))
        return TELA_RESOLUCAO_MENU_PRINCIPAL;

    if (DrawCenteredButton("Sair", 350, 30, LIGHTGRAY, RED))
        return TELA_SAIR;

    return TELA_MENU_PRINCIPAL;
}

Tela tela_resolucao_menu_principal(int *tamanho_bloco) {
    DrawCenteredText("SELECIONE A ESCALA", 100, 30, SKYBLUE);
    
    if (DrawCenteredButton("20", 200, 20, WHITE, GREEN)) {
        *tamanho_bloco = 20;
        SetGameResolution(*tamanho_bloco * 40, *tamanho_bloco * 21);
    }

    if (DrawCenteredButton("30", 240, 20, WHITE, GREEN)) {
        *tamanho_bloco = 30;
        SetGameResolution(*tamanho_bloco * 40, *tamanho_bloco * 21);
    }

    if (DrawCenteredButton("40", 280, 20, WHITE, GREEN)) {
        *tamanho_bloco = 40;
        SetGameResolution(*tamanho_bloco * 40, *tamanho_bloco * 21);
    }

    // Voltar
    if (DrawCenteredButton("Voltar", 450, 30, GRAY, WHITE))
        return TELA_MENU_PRINCIPAL;
    
    // Mostra resolução atual no rodapé
    DrawText(TextFormat("%dx%d", GetScreenWidth(), GetScreenHeight()), 10, GetScreenHeight() - 20, 10, DARKGRAY);

    return TELA_RESOLUCAO_MENU_PRINCIPAL;
}

