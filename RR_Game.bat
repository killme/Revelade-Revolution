@ECHO OFF

rem set SDL_VIDEO_WINDOW_POS=0,0
set APP_DIR=.
set APP_OPTIONS= "-q$HOME\My Games\ReveladeRevolution"
set APP_BIN=bin

IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
    set APP_BIN=bin64
)
IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
    set APP_BIN=bin64
)

:RETRY
IF EXIST %APP_BIN%\reveladerevolution.exe (
    start %APP_BIN%\reveladerevolution.exe %APP_OPTIONS% %*
) ELSE (
    IF EXIST %APP_DIR%\%APP_BIN%\reveladerevolution.exe (
        pushd %APP_DIR%
        start %APP_BIN%\reveladerevolution.exe %APP_OPTIONS% %*
        popd
    ) ELSE (
        IF %APP_BIN% == bin64 (
            set APP_BIN=bin
            goto RETRY
        )
        echo Unable to find the Revelade Revolution client
        pause
    )
)
