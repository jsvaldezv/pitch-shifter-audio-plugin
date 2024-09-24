#!/bin/bash

echo '
  ______              _____                _         _______        _     
 |  ____|            / ____|              | |       |__   __|      | |    
 | |__   __ _ _ __  | |     __ _ _ __   __| |_   _     | | ___  ___| |__  
 |  __| / _  |  __| | |    / _  |  _ \ / _  | | | |    | |/ _ \/ __|  _ \ 
 | |___| (_| | |    | |___| (_| | | | | (_| | |_| |    | |  __/ (__| | | |
 |______\__,_|_|     \_____\__,_|_| |_|\__,_|\__, |    |_|\___|\___|_| |_|
                                              __/ |                       
                                             |___/                             
'

echo "==========================="
echo "         Starting..."
echo "==========================="

# ====================================================================================

filename="plugin.cfg"

PLUGIN_NAME=$(grep 'PLUGIN_NAME' $filename | awk -F '=' '{print $2}')
VERSION=$(grep 'VERSION' $filename | awk -F '=' '{print $2}')
PLUGIN_CODE=$(grep 'CODE' $filename | awk -F '=' '{print $2}')

TYPE="PreRelease"
CUSTOM_PATH="default"
PLUGINS_INSTALLER_PATH="Installers/Plugins"
VALIDATE_SCRIPT="Scripts/macOS/validate.sh"

# ====================================================================================

echo "Plug-in name: $PLUGIN_NAME"
echo "Plug-in version: $VERSION"
echo "Plug-in code: $PLUGIN_CODE"
echo "Compiling in $TYPE mode"

# ====================================================================================

echo "==========================="
echo "     Setting project..."

mkdir -p $TYPE
mkdir -p "$PLUGINS_INSTALLER_PATH"
cd $TYPE

# ====================================================================================

echo "==========================="
echo "   Building plug-ins..."

cmake -DCUSTOM_PATH=$CUSTOM_PATH -DUSER_DIR=$HOME -DCMAKE_BUILD_TYPE=$TYPE ..
make

# ====================================================================================

echo "==========================="
echo "     Move plug-ins..."

cd ..
mv "$TYPE/Source/${PLUGIN_NAME}_artefacts/$TYPE/VST3/${PLUGIN_NAME}.vst3" "$PLUGINS_INSTALLER_PATH"
mv "$TYPE/Source/${PLUGIN_NAME}_artefacts/$TYPE/AU/${PLUGIN_NAME}.component" "$PLUGINS_INSTALLER_PATH"
mv "$TYPE/Source/${PLUGIN_NAME}_artefacts/$TYPE/VST/${PLUGIN_NAME}.vst" "$PLUGINS_INSTALLER_PATH"
mv "$TYPE/Source/${PLUGIN_NAME}_artefacts/$TYPE/AAX//${PLUGIN_NAME}.aaxplugin" "$PLUGINS_INSTALLER_PATH"

# ====================================================================================

echo "==========================="
echo "   Validate plug-ins..."

sh $VALIDATE_SCRIPT "$PLUGINS_INSTALLER_PATH" "$PLUGIN_NAME"

# ====================================================================================

echo "==========================="
echo "   	 Cleaning..."

rm -rf "$PLUGINS_INSTALLER_PATH"
rm -rf $TYPE

# ====================================================================================

echo "==========================="
echo "          Done"
echo "==========================="