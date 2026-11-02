#!/bin/bash
# Double-click this file to install RAGE LEAD for your user account (macOS).
cd "$(dirname "$0")"
DEST="$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$DEST"
rm -rf "$DEST/RAGE LEAD.vst3"
cp -R "RAGE LEAD.vst3" "$DEST/"
xattr -cr "$DEST/RAGE LEAD.vst3" 2>/dev/null
echo ""
echo "Done! RAGE LEAD was installed to: $DEST"
echo "Open your DAW and rescan plugins."
read -n 1 -s -r -p "Press any key to close..."
