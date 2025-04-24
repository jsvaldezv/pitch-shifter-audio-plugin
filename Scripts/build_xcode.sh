#!/bin/bash

echo "==========================="
echo "         Starting        "
echo "==========================="

# ==================================================================

filename="plugin.cfg"
CUSTOM_PATH="default"

plugin_name=$(grep 'PLUGIN_NAME' $filename | awk -F '=' '{print $2}')
plugin_version=$(grep 'VERSION' $filename | awk -F '=' '{print $2}')
plugin_code=$(grep 'CODE' $filename | awk -F '=' '{print $2}')

# ==================================================================

echo "==========================="
echo "Plug-in name: $plugin_name"
echo "Plug-in version: $plugin_version"
echo "Plug-in code: $plugin_code"

echo "==========================="
echo "Creating Xcode project..."
echo "==========================="
cmake -G Xcode -H. -B Xcode

echo "==========================="
echo "Opening Xcode..."
echo "==========================="
open Xcode/${plugin_name}.xcodeproj