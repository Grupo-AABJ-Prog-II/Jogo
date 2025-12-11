#include "hud.h"
#include <stdio.h>

void InicializarEstadoJogo(EstadoJogo *estado) {
    estado->pontuacao = 0;
    estado->vidas = 3;
    estado->nivel = 1;
    estado->pelletsRestantes = 0; // Será atualizado ao carregar o mapa
    estado->gameover = 0;
}

void AdicionarPontos(EstadoJogo *estado, int pontos) {
    estado->pontuacao += pontos;
    // Adicionar ganhar vida a cada 10.000 pontos?
    // talvez um: if (estado->pontuacao % 10000 == 0) estado->vidas++;?
}

void DesenharHUD(EstadoJogo *estado, int larguraJanela, int alturaJanela, int alturaHUD) {
    // A HUD fica na parte inferior da tela, ocupando a altura definida
    int yHUD = alturaJanela - alturaHUD;
    
    DrawRectangle(0, yHUD, larguraJanela, alturaHUD, (Color){ 20, 20, 20, 255 }); // Cinza escuro quase preto
    DrawLine(0, yHUD, larguraJanela, yHUD, GRAY); // Linha separadora

    // Configurações de Texto
    int tamanhoTexto = 20;
    if (alturaHUD < 40) tamanhoTexto = 10; // Ajuste para resoluções pequenas

    int margemEsquerda = 20;
    int centroY = yHUD + (alturaHUD - tamanhoTexto) / 2;

    // Desenha as Informações
    
    // potos(meu Deus é mt dificil colocar til com letra maiuscula)
    DrawText(TextFormat("PONTUAÇÃO: %06d", estado->pontuacao), 
             margemEsquerda, centroY, tamanhoTexto, WHITE);

    // Nível (Centralizado aprox)
    DrawText(TextFormat("LEVEL: %d", estado->nivel), 
             larguraJanela / 3, centroY, tamanhoTexto, YELLOW);

    // PELLETS (Mais para a direita)
    DrawText(TextFormat("PELLETS: %d", estado->pelletsRestantes), 
             (larguraJanela / 3) * 2, centroY, tamanhoTexto, GREEN);

    // VIDAS (Canto Direito)
    // Pode desenhar bolinhas amarelas ou ícones do Pacman para representar vidas
    int xVidas = larguraJanela - 100;
    DrawText("VIDAS:", xVidas, centroY, tamanhoTexto, RED);
    
    for (int i = 0; i < estado->vidas; i++) {
        DrawCircle(xVidas + 70 + (i * 15), centroY + (tamanhoTexto/2), 5, YELLOW);
    }

    // Mensagem de Game Over
    if (estado->gameover) {
        const char* msg = "Fim de Jogo - Aperte 'R' para reiniciar";
        int textWidth = MeasureText(msg, 40);
        DrawText(msg, (larguraJanela - textWidth)/2, alturaJanela/2 - 20, 40, RED);
    }
}