# Tomb Raider Linux Launcher
A tool to run Tomb Raider classics 1, 2, 3, 4, 5 on Linux with Wine/Proton. Problems that you may face for 3 and 4 are one particular input problem and sometimes graphical glitches, crashes or lags. The work around for the input problem is to avoid ctrl alt. The other work around is to use a kernel suitable for gaming/wine and try to use mature suitable graphics drivers, usually open source. That will get you far if you want to play those games, but you should also know modding is tricky and only some might work. I'm doing it for fun and I would love to learn more c++ and Qt

This application will be written in c++ for the GUI and for modding and manipulation configuration files. Bash for launching with Steam or Lutris or just Wine.

# Features
* with steam or Lutris to select a map from trle.net to play
* launch along the game a controller mapper like qjoypad or antimicrox
* script so that I dont have to manually move all game files when I want to play a mod with steam input to steams tomb raider game folder
* backup game savefiles and configurations for all maps and moods.
* check for corrupted config.txt file and possible other workarounds. (TR3)
* check if I forgot to connect controller and so on... On Linux. I like my Steam controller configurations also.
* implement patching of files and backup.
* checksums
* download maps/mods, and install them

# trle.net mods the launcher will download and play with wine-tkg that I tested
* [Calypsis Jungle - Part One](https://www.trle.net/sc/levelfeatures.php?lid=3500)
* [Feder - Templars Secret](https://www.trle.net/sc/levelfeatures.php?lid=3082)
* [Delca - Kitten Adventure](https://www.trle.net/sc/levelfeatures.php?lid=3379)

# Guide

How to play Tomb Raider 3

# Patches
The game uses old API and old optimizations..
With wine you need a minimum
4 x DDR3-1600mhz and 3.0 GHz CPU
It's a bit demanding on the memory, try use gallium nine if you have nouveau
No special graphics card, but if you gonna play mods you could need
One 2010+ card that has about 2-4 Gb for some Tomb Raider 4 mods with enhanced graphics
recommend patches, on steam proton 7
* https://github.com/Trxyebeep/tomb3
* https://github.com/dege-diosg/dgVoodoo2
* https://tombraiders.net/stella/downloads/widescreen.html
* https://core-design.com/community_tr3withoutcrystals.html

## TR3
Most, if not all trle.net tr3 mods will work with proton 7, if unpacked and configured with dgVoodoo2
some new tr3 mods already comes with the tomb3 patch. A combination of tomb3 and dgVoodoo2 is recommended for the original game.

# Tips
## Recommended compatibility layer
* Proton 7.0 for TR3 - with dgVoodoo2
* Proton 5.0-10 for TR3
* Sometimes avoiding Ctrl and Alt will make wine/proton
* [GloriousEggroll](https://github.com/GloriousEggroll/proton-ge-custom/releases/tag/6.21-GE-2)
* [Wine-tkg](https://github.com/Frogging-Family/wine-tkg-git/releases/tag/7.6.r12.g51472395) tick "emulate a virtual desktop"

## Recommended compatibility layer configuration and other tips
### TR3
* Use fsync or esync
* If you have problems witn FMV cut-scenes. Run winecfg go to graphics tab and tick "emulate a virtual desktop" use same size as you're desktop under. Or use dgVoodoo2
* Sometimes when you run tomb3.exe -setup you window manager will put the background over the configuration windows, for example in i3 mod+f "full screen key" or something similar like alt+tab could help you see the window
* The game will recognize your controller, left stick only, see [https://github.com/AntiMicroX/antimicrox](https://github.com/AntiMicroX/antimicrox)
