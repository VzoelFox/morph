@echo off
REM scripts/release_windows.bat
REM Release Build Script for Windows x86_64
REM ==============================================================================

setlocal EnableDelayedExpansion

set ASM_DIR=corelib\platform\x86_64\asm_win
set OUT_DIR=release\windows
set BIN_NAME=morph_v1.1.exe

echo [Release] Setting up directories...
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

REM Core Sources
set SOURCES=runtime alloc stack builtins executor control_flow context string symbol arena pool snapshot lexer vector parser compiler type scheduler net dns graphics font daemon_cleaner crypto_sha256 crypto_chacha20

set OBJECTS=

echo [Release] Assembling Core Sources...
for %%s in (%SOURCES%) do (
    echo   -^> %%s.asm
    nasm -f win64 -o "%OUT_DIR%\%%s.obj" "%ASM_DIR%\%%s.asm"
    if errorlevel 1 (
        echo [Error] Failed to assemble %%s.asm
        exit /b 1
    )
    set OBJECTS=!OBJECTS! "%OUT_DIR%\%%s.obj"
)

echo [Release] Assembling Morph CLI...
nasm -f win64 -o "%OUT_DIR%\morph.obj" "tools\morph.asm"

echo [Release] Linking morph.exe...
link /ENTRY:_start /SUBSYSTEM:CONSOLE /OUT:%OUT_DIR%\morph.exe "%OUT_DIR%\morph.obj" %OBJECTS% kernel32.lib ws2_32.lib user32.lib gdi32.lib

echo [Success] Binary created at %OUT_DIR%\morph.exe
