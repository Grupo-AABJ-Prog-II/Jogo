#ifndef MENUS_H
#define MENUS_H

#include "tipos.h"

Tela tela_menu_principal();
Tela tela_resolucao_menu_principal(int *tamanho_bloco);
Tela tela_menu();
Tela tela_gameover(int pontuacaoFinal);
Tela tela_vitoria(int pontuacaoFinal);
Tela tela_vitoria_final(int pontuacaoFinal);

#endif