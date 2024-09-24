@echo off
title Test plug-ins

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
echo          Starting...
echo ===========================

:: ====================================================================================

set filename=plugin.cfg

for /f "tokens=2 delims==" %%a in ('findstr "PLUGIN_NAME" %filename%') do set PLUGIN_NAME=%%a
for /f "tokens=2 delims==" %%a in ('findstr "VERSION" %filename%') do set VERSION=%%a
for /f "tokens=2 delims==" %%a in ('findstr "CODE" %filename%') do set PLUGIN_CODE=%%a

:: ======================= Path =======================

echo 1. Release
echo 2. PreRelease
choice /C 12 /M "Select build configuration:"

if errorlevel 2 goto PreReleaseConf
if errorlevel 1 goto ReleaseConf

:PreReleaseConf
set TYPE=PreRelease
goto Continue

:ReleaseConf
set TYPE=Release
goto Continue

:: ====================================================================================

:Continue
set CUSTOM_PATH="default"
set DEFAULT_PLUGIN_PATH=%TYPE%/Source/%PLUGIN_NAME%_artefacts/%TYPE%
set PLUGINS_INSTALLER_PATH="Installers\Plugins"
set TESTING_SCRIPT="Scripts\windows\validate.bat"

echo Plug-in name: %PLUGIN_NAME%
echo Plug-in version: %VERSION%
echo Plug-in code: %PLUGIN_CODE%
echo Path to copy: %CUSTOM_PATH%
echo Building %TYPE% configuration

:: ====================================================================================

echo ===========================
echo      Setting project...

mkdir %PLUGINS_INSTALLER_PATH%

:: ====================================================================================

echo ===========================
echo     Building plug-ins...

cmake -G "Visual Studio 17 2022" -H. -B %TYPE% -T ClangCL -DCUSTOM_PATH=%CUSTOM_PATH% -DUSER_DIR="" -DCMAKE_BUILD_TYPE="%TYPE%"
cmake --build %TYPE% --config %TYPE%

:: ====================================================================================

echo ===========================
echo       Move plug-ins...

robocopy "%DEFAULT_PLUGIN_PATH%/VST3/%PLUGIN_NAME%.vst3" "%PLUGINS_INSTALLER_PATH%/%PLUGIN_NAME%.vst3" /E
copy /Y "%DEFAULT_PLUGIN_PATH%\VST\%PLUGIN_NAME%.dll" "%PLUGINS_INSTALLER_PATH%\%PLUGIN_NAME%.dll"
robocopy "%DEFAULT_PLUGIN_PATH%/AAX/%PLUGIN_NAME%.aaxplugin" "%PLUGINS_INSTALLER_PATH%/AAX/%PLUGIN_NAME%.aaxplugin" /E

:: ====================================================================================

echo ===========================
echo     Validate plug-ins...

call %TESTING_SCRIPT% %PLUGINS_INSTALLER_PATH% %PLUGIN_NAME%

:: ====================================================================================

echo ===========================
echo         Cleaning...

rd /s /q "%PLUGINS_INSTALLER_PATH%"
rd /s /q "%TYPE%\"

:: ====================================================================================

echo ===========================
echo             Done
echo ===========================