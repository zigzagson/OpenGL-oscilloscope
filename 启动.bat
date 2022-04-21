@ECHO OFF&PUSHD %~DP0 &TITLE 选择模式
mode con cols=36 lines=20
color E0
cls

echo.
echo 启动哪个模式
echo ==============================
echo.
echo 输入1，亮色模式
echo.
echo 输入2，暗色模式
echo.
echo ==============================
echo.
echo.
set /p user_input=请输入数字：
if %user_input% equ 1 (
	copy img\init_light.json init.json
)
if %user_input% equ 2 (
	copy img\init_dark.json init.json
)

start oscilloscope.exe
