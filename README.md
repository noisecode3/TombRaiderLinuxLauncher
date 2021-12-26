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

How to play Tomb Raider 3 steam input working example.

![screenshot](https://raw.githubusercontent.com/noisecode3/TombRaiderLinuxSteamManager/main/controller.png "controller")

# Patches
The game is old and broken even on windows...
I recommend these patches

- https://tombraiders.net/stella/downloads/widescreen.html

- https://core-design.com/community_tr3withoutcrystals.html

- https://github.com/legluondunet/mlls-tools/blob/master/dgVoodoo2/dgVoodoo2_61.zip

# dgVoodoo2 info
## TR3
Files used:<br />
MS/x86/DDraw.dll<br />
MS/x86/D3DImm.dll<br />
dgVoodoo.conf --> change
```
      DirectX:
        ForceVerticalSync: true
        Resolution: max_qhd_isf
        VRAM: 256
        dgVoodooWatermark: false
      General:
        CaptureMouse: false
        FullScreenMode: false
        ScalingMode: stretched_ar
```
Put these in game directory
# Help
## Recommended compatibility layer
- Proton 5.0-10 (for TR3)
- Wine-tkg (in general works good)

## Recommended compatibility layer configuration
### TR3
- Overide ddraw to native,builtin (especially with dgVoodoo2 using D3D11 not Glide)
- CSMT should probably be disabled in winecfg (It may not matter nowadays)
- Use fsync or esync
- If you have problems witn FMV cut-scenes. Run winecfg go to graphics tab and tick "emulate a virtual desktop" use same size as you're desktop under.
- installing d3dcompiler_47 with winetricks can help D3D compatibility (It may not matter nowadays)

