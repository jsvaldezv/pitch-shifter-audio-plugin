@echo off
title Build VS22 Project

echo ===========================
echo         Starting...
echo ===========================

:: ===================================================

set filename=plugin.cfg

for /f "tokens=2 delims==" %%a in ('findstr "PLUGIN_NAME" %filename%') do set plugin_name=%%a
for /f "tokens=2 delims==" %%a in ('findstr "VERSION" %filename%') do set plugin_version=%%a
for /f "tokens=2 delims==" %%a in ('findstr "CODE" %filename%') do set plugin_code=%%a

:: ===================================================

echo ===========================

echo Plug-in name: %plugin_name%
echo Plug-in version: %plugin_version%
echo Plug-in code: %plugin_code%

echo ===========================
echo Creating Visual Studio 2022 project...
cmake -G "Visual Studio 17 2022" -H. -B VS22 -T ClangCL

echo ===========================
echo Opening Visual Studio 2022...
echo ===========================
start VS22/%plugin_name%.sln