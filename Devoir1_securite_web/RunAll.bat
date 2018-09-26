start cmd /k "%~dp0Debug/Clement.exe 8812"
sleep 10
start cmd /k "%~dp0Debug/Agnesse.exe 127.0.0.1 8812"
sleep 500
start cmd /k "%~dp0Debug/Bob.exe 127.0.0.1 8812"