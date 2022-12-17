# Tomb Raider Linux Launcher
A tool to run Tomb Raider classics 1, 2, 3, 4, 5 on Linux with Wine/Proton. Problems that you may face for 3 and 4 are one particular input problem and sometimes graphical glitches, crashes or lags. The work around for the input problem is to avoid ctrl alt. The other work around is to use a kernel suitable for gaming/wine and try to use mature suitable graphics drivers, usually open source. That will get you far if you want to play those games, but you should also know modding is tricky and only some might work. I'm doing it for fun and I would love to learn more c++ and Qt

This application will be written in c++ for the GUI and for modding and manipulation configuration files. Bash for launching with Steam or Lutris or just Wine.

# Features

- with steam or Lutris to select a map from trle.net to play
- launch along the game a controller mapper like qjoypad or antimicrox
- script so that I dont have to manually move all game files when I want to play a mod with steam input to steams tomb raider game folder
- backup game savefiles and configurations for all maps and moods.
- check for corrupted config.txt file and possible other workarounds. (TR3)
- check if I forgot to connect controller and so on... On Linux. I like my Steam controller configurations also.
- implement patching of files and backup.
- checksums

# Guide

How to play Tomb Raider 3

# Patches
The game is old and broken even on windows... We can make it work perfectly, and lag
I recommend these patches, on the steam version

- https://tombraiders.net/stella/downloads/widescreen.html

- https://core-design.com/community_tr3withoutcrystals.html


## TR3
If you have good vulkan drivers GloriousEggroll 6.21-GE-2 will work perfectly on steam, no bugs at all
I tested with Intel, Nvidia and AMD a lot. I play this game a lot. Here is some wine-tkg example:
- With -setup
```
cd ~/games/TombRaider\ \(III\) # must be in the current working directory
WINEPREFIX=~/games/TombRaider\ \(III\)_WINEPREFIX \
STAGING_SHARED_MEMORY=1 \
STAGING_RT_PRIORITY_SERVER=95 \
STAGING_RT_PRIORITY_BASE=95 \
WINEFSYNC=1 \
DRI_PRIME=1 \
/opt/wine-tkg-staging-fsync-git-6.17.r0.g5f19a815/bin/wine tomb3.exe -setup
```
- With Gallium Hud, you should not see a single frame drop but its only 30 fps game lol, 1 cpu could be saturated for short time, on 20 years almost potato laptop
```
cd ~/games/TombRaider\ \(III\)
MONITORGPU=".w550.h280.c70fps:70,.w550.h280GPU-load:100,.w550.h280.c80temperature:80"
MONITORCPU=".w550.h280cpu0+cpu1+cpu2+cpu3+cpu4+cpu5+cpu6+cpu7+cpu8+cpu9+cpu10+cpu11:100"
GALLIUM_HUD=${MONITORGPU},${MONITORCPU} \
WINEPREFIX=~/games/TombRaider\ \(III\)_WINEPREFIX \
STAGING_SHARED_MEMORY=1 \
STAGING_RT_PRIORITY_SERVER=95 \
STAGING_RT_PRIORITY_BASE=95 \
WINEFSYNC=1 \
DRI_PRIME=1 \
/opt/wine-tkg-staging-fsync-git-6.17.r0.g5f19a815/bin/wine tomb3.exe
```
# Help
## Recommended compatibility layer
- Proton 5.0-10 (for TR3) - Small input bug and if you have nvidia the cutscene after the firt map would be a black screen
- GloriousEggroll (https://github.com/GloriousEggroll/proton-ge-custom/releases/tag/6.21-GE-2)
- Wine-tkg (https://github.com/Frogging-Family/wine-tkg-git/releases/tag/6.17.r0.g5f19a815) tick "emulate a virtual desktop"
- Wine-tkg works good in general

## Recommended compatibility layer configuration and other tips
### TR3
- Make sure mesa is compiled with gallium (this is better on some old computers, i915,i965,r100,r200,nouveau), or use dxvk with wine-tkg will run the best way
- Use fsync or esync
- Use winetricks latest gallium
- If you have problems witn FMV cut-scenes. Run winecfg go to graphics tab and tick "emulate a virtual desktop" use same size as you're desktop under.
- Use a gaming kernel, for example https://xanmod.org/
- Sometimes when you run tomb3.exe -setup you window manager will put the background over the configuration windows, for example in i3 mod+f "full screen key" or something similar could help you see the window
- The game will recognize you're controller, left stick and all the other, except D-pad was missing for me, see https://github.com/AntiMicroX/antimicrox
