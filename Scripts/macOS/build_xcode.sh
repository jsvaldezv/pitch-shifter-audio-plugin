#!/bin/bash

echo "==========================="
echo "         Starting        "
echo "==========================="

# ==================================================================

PATH_OPTIONS="default custom"
CLEAN_OPTIONS="true false"

# ==================================================================

filename="plugin.cfg"

plugin_name=$(grep 'PLUGIN_NAME' $filename | awk -F '=' '{print $2}')
plugin_version=$(grep 'VERSION' $filename | awk -F '=' '{print $2}')
plugin_code=$(grep 'CODE' $filename | awk -F '=' '{print $2}')

# ==================================================================

echo "Select path:\n"
select CUSTOM_PATH_PRE in $PATH_OPTIONS; 
do
	if [ "$CUSTOM_PATH_PRE" = "custom" ]; then
		echo "\n"
		read -p "Type your custom path: " CUSTOM_PATH
	else
		CUSTOM_PATH=default
	fi
	break
done

echo "==========================="
echo "Do you wanna clean the project?:\n"
select CLEAN in $CLEAN_OPTIONS; 
do
	break
done

# ==================================================================

echo "==========================="
echo "Plug-in name: $plugin_name"
echo "Plug-in version: $plugin_version"
echo "Plug-in code: $plugin_code"
echo "Path to copy: $CUSTOM_PATH"

if $CLEAN; then
	echo "==========================="
	echo "Cleaning Xcode project..."
	rm -rf Xcode
fi

echo "==========================="
echo "Creating Xcode project..."
echo "==========================="
cmake -G Xcode -H. -B Xcode -DCUSTOM_PATH=$CUSTOM_PATH -DUSER_DIR=$HOME

echo "==========================="
echo "Opening Xcode..."
echo "==========================="
open Xcode/${plugin_name}.xcodeproj