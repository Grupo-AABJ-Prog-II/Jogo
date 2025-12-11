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

Tela tela_menu() {
    // --- TÍTULO CENTRALIZADO ---
    DrawCenteredText("PAUSA", 50, 40, YELLOW);

    int startY = 150;     // Altura inicial dos botões
    int spacing = 50;     // Espaço entre eles
    int fontSize = 20;
    
    //Lógica dos botões
    if (IsKeyPressed(KEY_N)) { return TELA_MENU_PRINCIPAL; } // Ajustado para voltar ao menu principal que reinicia
    if (IsKeyPressed(KEY_C)) { return TELA_JOGO; }
    if (IsKeyPressed(KEY_S)) { return TELA_SAIR; } // Salvar nao implementado aqui, entao sair
    if (IsKeyPressed(KEY_Q)) { return TELA_SAIR; }
    if (IsKeyPressed(KEY_V)) { return TELA_JOGO; }
    
    // --- BOTÕES CENTRALIZADOS ---
    if (DrawCenteredButton("Continuar Jogo (C)", startY, fontSize, WHITE, GREEN))
        return TELA_JOGO;

    // A opção "Novo Jogo" aqui poderia levar ao Menu Principal para resetar
    if (DrawCenteredButton("Menu Principal (N)", startY + spacing, fontSize, WHITE, BLUE))
        return TELA_MENU_PRINCIPAL;

    if (DrawCenteredButton("Sair do Jogo (Q)", startY + spacing * 2, fontSize, WHITE, RED))
        return TELA_SAIR;

    return TELA_MENU;
}

Tela tela_gameover(int pontuacaoFinal) {
    DrawCenteredText("GAME OVER", 100, 60, RED);
    
    DrawCenteredText(TextFormat("Pontuacao Final: %d", pontuacaoFinal), 200, 30, WHITE);

    if (DrawCenteredButton("Voltar ao Menu", 350, 30, GRAY, WHITE)) {
        return TELA_MENU_PRINCIPAL;
    }
    
    if (DrawCenteredButton("Sair", 400, 30, GRAY, RED)) {
        return TELA_SAIR;
    }

    return TELA_GAMEOVER;
}

Tela tela_vitoria(int pontuacaoFinal) {
    DrawCenteredText("VITORIA!", 100, 60, GREEN);
    
    DrawCenteredText(TextFormat("Pontuacao Final: %d", pontuacaoFinal), 200, 30, WHITE);

    if (DrawCenteredButton("Voltar ao Menu", 350, 30, GRAY, WHITE)) {
        return TELA_MENU_PRINCIPAL;
    }

    return TELA_VITORIA;
}