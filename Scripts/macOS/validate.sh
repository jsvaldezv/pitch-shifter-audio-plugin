#!/bin/bash

PLUGINS_PATH=$1
PLUGIN_NAME=$2

# ====================================================================================
# Pluginval

echo "==========================="
echo "Validating with pluginval"

curl -L "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_macOS.zip" -o pluginval.zip
unzip pluginval

echo "==========================="
echo "Validating VST3"

pluginval.app/Contents/MacOS/pluginval \
    --strictness-level 5 \
    --validate-in-process \
    --validate $PLUGINS_PATH/$PLUGIN_NAME.vst3 \
    --output-dir pluginval-logs \
    --timeout-ms 500000 \ || exit 1

echo "==========================="
echo "Validating VST"

pluginval.app/Contents/MacOS/pluginval \
    --strictness-level 5 \
    --validate-in-process \
    --validate $PLUGINS_PATH/$PLUGIN_NAME.vst \
    --output-dir pluginval-logs \
    --timeout-ms 500000 \ || exit 1

echo "==========================="
echo "Validating AU"

pluginval.app/Contents/MacOS/pluginval \
    --strictness-level 5 \
    --validate-in-process \
    --validate $PLUGINS_PATH/$PLUGIN_NAME.component \
    --output-dir pluginval-logs \
    --timeout-ms 500000 \ || exit 1

rm pluginval.zip
rm -rf pluginval.app
rm -rf pluginval-logs

# ====================================================================================
# Auval

echo "==========================="
echo "Validating with auval"

echo "Validating AU"
auvaltool -v aufx Gc0p Manu || exit 1