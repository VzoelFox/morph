@echo off
REM scripts/build_daemon_windows.bat
REM Build MorphFox Daemon for Windows
REM ==============================================================================

setlocal

set ASM_DIR=corelib\platform\x86_64\asm_win
set OUT_DIR=build\windows

echo [Build] Building MorphFox Daemon Cleaner for Windows...
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

REM Assemble daemon components
echo [Build] Assembling daemon_cleaner.asm...
nasm -f win64 -o "%OUT_DIR%\daemon_cleaner.obj" "%ASM_DIR%\daemon_cleaner.asm"
if errorlevel 1 (
    echo [Error] Failed to assemble daemon_cleaner.asm
    exit /b 1
)

echo [Build] Assembling morph_daemon_main.asm...
nasm -f win64 -o "%OUT_DIR%\morph_daemon_main.obj" "%ASM_DIR%\morph_daemon_main.asm"
if errorlevel 1 (
    echo [Error] Failed to assemble morph_daemon_main.asm
    exit /b 1
)

REM Link with Microsoft Linker
echo [Build] Linking morph_daemon.exe...
link /ENTRY:_start /SUBSYSTEM:CONSOLE /OUT:"%OUT_DIR%\morph_daemon.exe" ^
     "%OUT_DIR%\daemon_cleaner.obj" "%OUT_DIR%\morph_daemon_main.obj" ^
     kernel32.lib

if errorlevel 1 (
    echo [Error] Failed to link morph_daemon.exe
    exit /b 1
)

echo [Build] Success! Binary: %OUT_DIR%\morph_daemon.exe
echo.
echo Usage:
echo   %OUT_DIR%\morph_daemon.exe start   - Start daemon
echo   %OUT_DIR%\morph_daemon.exe stop    - Stop daemon
echo   %OUT_DIR%\morph_daemon.exe status  - Check status
