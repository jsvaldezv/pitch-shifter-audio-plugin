@echo off
title Build VS22 Project

:::  ______              _____                _         _______        _     
::: |  ____|            / ____|              | |       |__   __|      | |    
::: | |__   __ _ _ __  | |     __ _ _ __   __| |_   _     | | ___  ___| |__  
::: |  __| / _  |  __| | |    / _  |  _ \ / _  | | | |    | |/ _ \/ __|  _ \ 
::: | |___| (_| | |    | |___| (_| | | | | (_| | |_| |    | |  __/ (__| | | |
::: |______\__,_|_|     \_____\__,_|_| |_|\__,_|\__, |    |_|\___|\___|_| |_|
:::                                              __/ |                       
:::                                             |___/                             
for /f "delims=: tokens=*" %%A in ('findstr /b ::: "%~f0"') do @echo(%%A

echo ===========================
echo         Starting...
echo ===========================

:: ===================================================

set CUSTOM_PATH=""
set filename=plugin.cfg

for /f "tokens=2 delims==" %%a in ('findstr "PLUGIN_NAME" %filename%') do set plugin_name=%%a
for /f "tokens=2 delims==" %%a in ('findstr "VERSION" %filename%') do set plugin_version=%%a
for /f "tokens=2 delims==" %%a in ('findstr "CODE" %filename%') do set plugin_code=%%a

:: ======================= Path =======================

echo 1. Custom
echo 2. Default
choice /C 12 /M "Select path:"

if errorlevel 2 goto DefaultPath
if errorlevel 1 goto CustomPath

:DefaultPath
set CUSTOM_PATH=default
goto Clean

:CustomPath
echo ===========================
set /p "CUSTOM_PATH=Enter your custom path:"
goto Clean

:: ======================= Clean =======================

:Clean
echo ===========================

echo 1. Yes
echo 2. No
choice /C 12 /M "Do you wanna clean the project?:"

if errorlevel 2 goto Configuration
if errorlevel 1 goto CleanProject

:CleanProject
echo ===========================
echo Cleaning VS22 project...
rd /s /q "VS22\"
goto Configuration

:: ================== Configuration ====================

:Configuration
echo ===========================

echo Plug-in name: %plugin_name%
echo Plug-in version: %plugin_version%
echo Plug-in code: %plugin_code%
echo Path to copy: %CUSTOM_PATH%

echo ===========================
echo Creating Visual Studio 2022 project...
cmake -G "Visual Studio 17 2022" -H. -B VS22 -T ClangCL -DCUSTOM_PATH=%CUSTOM_PATH% -DUSER_DIR=""

echo ===========================
echo Opening Visual Studio 2022...
echo ===========================
start VS22/%plugin_name%.sln