@ECHO OFF&PUSHD %~DP0 &TITLE ѡ��ģʽ
mode con cols=36 lines=20
color E0
cls

echo.
echo �����ĸ�ģʽ
echo ==============================
echo.
echo ����1����ɫģʽ
echo.
echo ����2����ɫģʽ
echo.
echo ==============================
echo.
echo.
set /p user_input=���������֣�
if %user_input% equ 1 (
	copy img\init_light.json init.json
)
if %user_input% equ 2 (
	copy img\init_dark.json init.json
)

start oscilloscope.exe
