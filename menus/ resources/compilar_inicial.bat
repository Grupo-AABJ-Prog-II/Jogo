@echo off
setlocal enabledelayedexpansion
:: ================================================================
:: SCRIPT DE COMPILAÇÃO PARA PROJETO MODULAR (MAIN + MAPA)
:: ================================================================

:: AQUI ESTÁ A MUDANÇA: Coloque TODOS os arquivos .c aqui, separados por espaço
set ARQUIVO_FONTE=Menu_Inicial.c
set ARQUIVO_SAIDA=menu_inicial.exe

echo.
echo === 1. PREPARANDO AMBIENTE ===

if exist "C:\raylib\w64devkit\bin\gcc.exe" (
    set "PATH=C:\raylib\w64devkit\bin;%PATH%"
    set "COMPILADOR=gcc"
    echo [SUCESSO] Ambiente isolado configurado para Raylib 64-bits.
) else (
    set "COMPILADOR=gcc"
    echo [AVISO] Kit w64devkit nao encontrado. Tentando gcc do sistema...
)

echo.
echo === 2. PROCURANDO BIBLIOTECA RAYLIB ===

set "LOCAIS[0]=C:\raylib\raylib"
set "LOCAIS[1]=C:\raylib"
set "LOCAIS[2]=C:\raylib\src"

set "CAMINHO_FINAL="
set "LIB_ENCONTRADA="

for /L %%i in (0,1,2) do (
    set "PASTA_TESTE=!LOCAIS[%%i]!"
    
    REM 1. Testa subpasta 'lib'
    if exist "!PASTA_TESTE!\lib\libraylib.a" (
        set "CAMINHO_FINAL=!PASTA_TESTE!"
        set "LIB_DIR=!PASTA_TESTE!\lib"
        set "INC_DIR=!PASTA_TESTE!\src"
        if exist "!PASTA_TESTE!\include\raylib.h" set "INC_DIR=!PASTA_TESTE!\include"
        set "LIB_ENCONTRADA=1"
        goto :ENCONTRADO
    )

    REM 2. Testa subpasta 'src'
    if exist "!PASTA_TESTE!\src\libraylib.a" (
        set "CAMINHO_FINAL=!PASTA_TESTE!"
        set "LIB_DIR=!PASTA_TESTE!\src"
        set "INC_DIR=!PASTA_TESTE!\src"
        set "LIB_ENCONTRADA=1"
        goto :ENCONTRADO
    )

    REM 3. Testa pasta raiz
    if exist "!PASTA_TESTE!\libraylib.a" (
        set "CAMINHO_FINAL=!PASTA_TESTE!"
        set "LIB_DIR=!PASTA_TESTE!"
        set "INC_DIR=!PASTA_TESTE!"
        set "LIB_ENCONTRADA=1"
        goto :ENCONTRADO
    )
)

:NAO_ENCONTRADO
echo.
echo [ERRO FATAL] Nao encontrei 'libraylib.a'.
pause
exit /b 1

:ENCONTRADO
echo [SUCESSO] Raylib encontrada em: !CAMINHO_FINAL!
echo.
echo ---------------------------------------
echo Compilando Projeto Modular (%ARQUIVO_FONTE%)...
echo ---------------------------------------

:: O comando agora inclui ambos os arquivos .c definidos no início
"%COMPILADOR%" %ARQUIVO_FONTE% -o %ARQUIVO_SAIDA% -O1 -Wall -std=c99 -Wno-missing-braces -I "!INC_DIR!" -L "!LIB_DIR!" -lraylib -lopengl32 -lgdi32 -lwinmm -static

if %errorlevel% neq 0 (
    echo.
    echo [ERRO] Falha na compilacao.
    pause
    exit /b %errorlevel%
)

echo.
echo [SUCESSO] Jogo compilado! Iniciando...
echo.

%ARQUIVO_SAIDA%