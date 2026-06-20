# Build and run with Conty

Compile and run the project inside the Conty container using `conty.sh`.

## Build

First run `create-conty.sh` here, then from the source top directory run this.

```sh
mkdir build
cd build
../conty/conty.sh cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/.local -DRELEASE=on ..
../conty/conty.sh make install -j$(nproc)
```

## Run from the build directory

```sh
../conty/conty.sh ./TombRaiderLinuxLauncher
```

## Setting up the desktop shortcut to use Conty

Open the desktop file:

```sh
open ~/.local/share/applications/TombRaiderLinuxLauncher.desktop
```

Edit the `Exec` line so it launches the installed binary through `conty.sh`. Note that it only accepts full path arguments and you're user name instead of noisecode3.

```ini
[Desktop Entry]
Name=Tomb Raider Linux Launcher
Comment=Play Tomb Raider Custom Levels and Tomb Raider Core Design games
Exec=/home/noisecode3/TombRaiderLinuxLauncher/conty/conty.sh /home/noisecode3/.local/bin/TombRaiderLinuxLauncher
Icon=TombRaiderLinuxLauncher
Terminal=false
Type=Application
Categories=Game;
```
