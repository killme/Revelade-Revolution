@ECHO OFF

set TESS_BIN=bin

IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
    set TESS_BIN=bin
)
IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
    set TESS_BIN=bin
)

start %TESS_BIN%\RR_GAME.exe
