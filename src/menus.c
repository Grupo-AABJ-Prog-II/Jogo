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
    SetWindowSize(width, height);
    
    int monitor = GetCurrentMonitor();
    int monitorW = GetMonitorWidth(monitor);
    int monitorH = GetMonitorHeight(monitor);
    
    SetWindowPosition((monitorW - width) / 2, (monitorH - height) / 2);
    
    TraceLog(LOG_INFO, "Resolucao alterada para: %dx%d", width, height);
}

void DrawCenteredText(const char* text, int y, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2; 
    DrawText(text, x, y, fontSize, color);
}

bool DrawCenteredButton(const char* text, int y, int fontSize, Color normal, Color hover) {
    int textWidth = MeasureText(text, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2;
    return DrawClickableText(text, x, y, fontSize, normal, hover);
}


Tela tela_menu_principal() {
    DrawCenteredText("PEQUEMEN", 100, 40, YELLOW);
    if (IsKeyPressed(KEY_C))
        return CARREGAR;
    if (DrawCenteredButton("Carregar Jogo (C)", 300, 30, WHITE, GREEN))
        return CARREGAR;
    
    if (DrawCenteredButton("Novo Jogo", 250, 30, WHITE, GREEN))
        return TELA_JOGO;
    
    // Opção para Carregar Jogo (tecla C)
    if (DrawCenteredButton("Carregar Jogo (C)", 300, 30, WHITE, GREEN)) {
        // A lógica de carregar o binário fica no main loop (pequemen.c)
        // Aqui apenas indicamos visualmente, mas o input é tratado lá ou retornamos um estado específico se quisermos
        // Por enquanto, vamos manter simples e deixar o main tratar a tecla 'C' globalmente ou retornar TELA_JOGO se for novo
    }
    
    if (DrawCenteredButton("Opcoes de Video", 350, 30, WHITE, BLUE))
        return TELA_RESOLUCAO_MENU_PRINCIPAL;

    if (DrawCenteredButton("Sair", 400, 30, LIGHTGRAY, RED))
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

    if (DrawCenteredButton("Voltar", 450, 30, GRAY, WHITE))
        return TELA_MENU_PRINCIPAL;
    
    DrawText(TextFormat("%dx%d", GetScreenWidth(), GetScreenHeight()), 10, GetScreenHeight() - 20, 10, DARKGRAY);

    return TELA_RESOLUCAO_MENU_PRINCIPAL;
}

Tela tela_menu() {
    DrawCenteredText("PAUSA", 50, 40, YELLOW);

    int startY = 150;
    int spacing = 50;
    int fontSize = 20;
    
    // Atalhos de teclado
    if (IsKeyPressed(KEY_N)) return TELA_MENU_PRINCIPAL; 
    if (IsKeyPressed(KEY_Q)) return TELA_SAIR;
    if (IsKeyPressed(KEY_V)) return TELA_JOGO;
    if (IsKeyPressed(KEY_C)) return CARREGAR;
    if (IsKeyPressed(KEY_S)) return SALVAR;
    // Botões
    if (DrawCenteredButton("Voltar(V)", startY, fontSize, WHITE, GREEN))
        return TELA_JOGO;
    if(DrawCenteredButton("Salvar(S)", startY + spacing, 20, LIGHTGRAY, DARKGREEN))
        return SALVAR;
    
    if(DrawCenteredButton("Carregar Save(C)", startY + spacing * 2, 20, LIGHTGRAY, DARKGREEN))
        return CARREGAR;

    if (DrawCenteredButton("Menu Principal (N)", startY + spacing * 3, fontSize, WHITE, BLUE))
        return TELA_MENU_PRINCIPAL;

    if (DrawCenteredButton("Sair do Jogo (Q)", startY + spacing * 4, fontSize, WHITE, RED))
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
    DrawCenteredText("NIVEL CONCLUIDO!", 100, 60, GREEN);
    DrawCenteredText(TextFormat("Pontuacao: %d", pontuacaoFinal), 200, 30, WHITE);
    
    // Esta tela serve como transição, não precisa de botão se for automático,
    // mas se quiser pausar entre níveis:
    if (DrawCenteredButton("Continuar", 350, 30, WHITE, GREEN)) {
        // O retorno aqui não importa tanto se a lógica de troca de nível for automática no main loop
        // Mas podemos usar para voltar ao jogo
        return TELA_JOGO; 
    }

    return TELA_VITORIA;
}

Tela tela_vitoria_final(int pontuacaoFinal) {
    DrawCenteredText("PARABENS! VOCE ZEROU O JOGO!", 100, 60, GOLD);
    DrawCenteredText(TextFormat("Pontuacao Total: %d", pontuacaoFinal), 200, 30, WHITE);

    if (DrawCenteredButton("Voltar ao Menu", 350, 30, GRAY, WHITE)) {
        return TELA_MENU_PRINCIPAL;
    }

    return TELA_VITORIA_FINAL;

}



