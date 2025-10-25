@echo off
echo === Building test_ac_hash ===
g++ -O2 -std=c++17 partie7.cpp -o partie7.exe
if %errorlevel% neq 0 (
    echo Compilation failed!
    pause
    exit /b 1
)
echo Running tests...
partie7.exe > test_results.txt
echo Results saved in test_results.txt
pause
