@echo off
@setlocal

set CUR_PATH=%~dp0
set DXC_PATH=%CUR_PATH%..\Source\External\DirectXShaderCompiler\bin\x64
set OUT_PATH=%1

robocopy %DXC_PATH% %OUT_PATH% dxcompiler.dll
if not %ERRORLEVEL%==1 ( goto :Error )

robocopy %DXC_PATH% %OUT_PATH% dxil.dll
if not %ERRORLEVEL%==1 ( goto :Error )

exit 0

:Error
exit /b %ERRORLEVEL%