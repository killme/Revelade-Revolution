@ECHO OFF
set RR_BIN=bin
IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
set RR_BIN=bin64
)
IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
set RR_BIN=bin64
)
start %RR_BIN%\reveladerevolution.exe "-q$HOME\My Games\ReveladeRevolution" -gserver-log.txt -d %* 
