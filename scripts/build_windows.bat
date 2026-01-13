@echo off
REM scripts/build_windows.bat
REM Build Script untuk Windows - Assemble dengan NASM dan Link
REM ==============================================================================

setlocal

set ASM_DIR=corelib\platform\x86_64\asm_win
set OUT_DIR=build\windows

echo [Build] Setting up directories...
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

REM List of Source Files (Core)
set SOURCES=runtime alloc stack builtins executor control_flow context string symbol arena pool snapshot lexer vector parser compiler type scheduler net dns graphics

set OBJECTS=

echo [Build] Assembling Core Sources...
for %%s in (%SOURCES%) do (
    echo   -^> %%s.asm
    nasm -f win64 -o "%OUT_DIR%\%%s.obj" "%ASM_DIR%\%%s.asm"
    if errorlevel 1 (
        echo [Error] Failed to assemble %%s.asm
        exit /b 1
    )
    set OBJECTS=!OBJECTS! "%OUT_DIR%\%%s.obj"
)

echo [Build] Assembling Tests...
nasm -f win64 -o "%OUT_DIR%\test_infinite_registers.obj" "%ASM_DIR%\test_infinite_registers.asm"
nasm -f win64 -o "%OUT_DIR%\test_pipeline_advanced.obj" "%ASM_DIR%\test_pipeline_advanced.asm"
nasm -f win64 -o "%OUT_DIR%\test_net.obj" "%ASM_DIR%\test_net.asm"
nasm -f win64 -o "%OUT_DIR%\test_dom.obj" "%ASM_DIR%\test_dom.asm"

echo [Build] Linking test_infinite_registers.exe...
link /ENTRY:_start /SUBSYSTEM:CONSOLE /OUT:%OUT_DIR%\test_infinite_registers.exe %OUT_DIR%\*.obj kernel32.lib ws2_32.lib

echo [Build] Linking test_pipeline_advanced.exe...
link /ENTRY:_start /SUBSYSTEM:CONSOLE /OUT:%OUT_DIR%\test_pipeline_advanced.exe %OUT_DIR%\*.obj kernel32.lib ws2_32.lib

echo [Build] Assembling Graphics Test...
nasm -f win64 -o "%OUT_DIR%\test_graphics.obj" "%ASM_DIR%\test_graphics.asm"

echo [Build] Linking test_net.exe...
link /ENTRY:_start /SUBSYSTEM:CONSOLE /OUT:%OUT_DIR%\test_net.exe %OUT_DIR%\*.obj kernel32.lib ws2_32.lib

echo [Build] Linking test_dom.exe...
link /ENTRY:_start /SUBSYSTEM:CONSOLE /OUT:%OUT_DIR%\test_dom.exe %OUT_DIR%\*.obj kernel32.lib ws2_32.lib

echo [Build] Linking test_graphics.exe...
link /ENTRY:_start /SUBSYSTEM:WINDOWS /OUT:%OUT_DIR%\test_graphics.exe %OUT_DIR%\*.obj kernel32.lib ws2_32.lib user32.lib gdi32.lib

echo --------------------------------------------------------
echo [Test 1] Running test_infinite_registers.exe...
"%OUT_DIR%\test_infinite_registers.exe"
if %ERRORLEVEL% EQU 0 (
    echo [Success] Registers Test Passed.
) else (
    echo [Failure] Registers Test Failed.
    exit /b 1
)

echo --------------------------------------------------------
echo [Test 2] Running test_pipeline_advanced.exe (Parser-Compiler-Executor)...
"%OUT_DIR%\test_pipeline_advanced.exe"
if %ERRORLEVEL% EQU 0 (
    echo [Success] Advanced Pipeline Test Passed.
) else (
    echo [Failure] Advanced Pipeline Test Failed.
    exit /b 1
)

echo --------------------------------------------------------
echo [Test 3] Running test_dom.exe (DOM Construction)...
"%OUT_DIR%\test_dom.exe"
if %ERRORLEVEL% EQU 0 (
    echo [Success] DOM Structure Verified.
) else (
    echo [Failure] DOM Structure Invalid.
    exit /b 1
)

echo --------------------------------------------------------
echo [All Tests Passed]
