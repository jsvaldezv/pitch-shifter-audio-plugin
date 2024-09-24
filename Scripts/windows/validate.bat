@echo off
title Validate plug-ins

set PLUGIN_PATH=%1
set PLUGIN_NAME=%2

set PLUGIN_PATH=%PLUGIN_PATH:"=%
set PLUGIN_NAME=%PLUGIN_NAME:"=%

:: ====================================================================================
:: Pluginval

echo ===========================
echo Validating with pluginval

powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Windows.zip -OutFile pluginval.zip"
powershell -Command "Expand-Archive pluginval.zip -DestinationPath ."

echo ===========================
echo Validating VST3

pluginval.exe ^
    --strictness-level 5 ^
    --validate-in-process ^
    --validate %PLUGIN_PATH%/%PLUGIN_NAME%.vst3 ^
    --output-dir pluginval-logs ^
    --timeout-ms 500000;

echo ===========================
echo Validating VST

pluginval.exe ^
    --strictness-level 5 ^
    --validate-in-process ^
    --validate %PLUGIN_PATH%/%PLUGIN_NAME%.dll ^
    --output-dir pluginval-logs ^
    --timeout-ms 500000;

:: ====================================================================================

echo ===========================
echo         Cleaning...

del "pluginval.zip"
del "pluginval.exe"
rd /s /q "pluginval-logs"

if %ERRORLEVEL% neq 0 exit /b 1