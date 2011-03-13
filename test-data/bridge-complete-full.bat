@echo off
echo %time%
..\bin\Release\image-completer-cmd.exe -ii bridge-input.png -im bridge-mask.png -io bridge-output.png
echo %time%