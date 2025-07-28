# Tomb Raider Level Launcher

A tool for running and modding classic Tomb Raider games (1–5) on Windows (After Linux) and Linux with Wine/Proton. It also supports searching and downloading levels from trle.net, enabling direct play from the application. Obs repository will move to TombRaiderLevelLauncher.

This project is my passion for Tomb Raider classics combined with learning C++, Python, and SQL. My dream is to create an arcade machine featuring all classic Tomb Raider games and TRLE levels, potentially with a co-op mode.

## Features

- Run level wine/proton from app
- Search trle.net with a fast offline indexing
- Lazy loading trle.net level info (runtime scraping)
- Integrates with Linux other launchers (Steam, Lutris, etc.)
- Controller mapper support using uinput
- Game save and configuration file backups
- Workarounds for Linux/Wine compatibility issues
- Modding support for executable-based mods(Download and applying open source modes)
- Level download and installation from trle.net
- Pool level files to save disk space, need NTFS on Windows for hard links
- Respectful scraping and filtering of data from trle.net (with site owner permission)

## Installation

### Dependencies

On Arch this should be enough, you get the rest probably when you install the base, curl + openssl
Ensure the following are installed on your Linux desktop:

- `curl` (7.71.0 or newer)
- `Boost`
- `OpenSSL`
- `Qt5`

```shell
sudo pacman -S qt5-wayland qt5-webengine qt5-imageformats boost
```

### Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/.local -DRELEASE=on ..
# or cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/.local -DNO_DATABASE=ON -DRELEASE=on ..
make install -j$(nproc)
```

## Use database

### pip

Never use sudo with pip, you can use a virtual environment or keep updating them in home.
If a required module isn't available in the system's package manager, fall back to pip.

```shell
python3 -m venv myenv
source myenv/bin/activate
pip install pycurl tqdm cryptography beautifulsoup4 pillow
```

or just

```shell
pip install pycurl tqdm cryptography beautifulsoup4 pillow
```

how to update

```shell
#!/bin/bash
pimp_my_pip() {
    pip list --outdated | awk 'NR > 2 {print $1}' | xargs -n1 pip install -U
}
pimp_my_pip
```

### Arch Linux

```shell
sudo pacman -S python-pycurl python-tqdm python-cryptography python-beautifulsoup4 python-pillow
```

### Ubuntu/Debian

```shell
sudo apt update
sudo apt install python3-pycurl python3-tqdm python3-cryptography python3-bs4 python3-pil
```

### Fedora

```shell
sudo dnf install python3-pycurl python3-tqdm python3-cryptography python3-beautifulsoup4 python3-pillow
```

### openSUSE

```shell
sudo zypper install python3-pycurl python3-tqdm python3-cryptography python3-beautifulsoup4 python3-Pillow
```

### Alpine Linux

```shell
sudo apk add py3-pycurl py3-tqdm py3-cryptography py3-beautifulsoup4 py3-pillow
```
Use the -sc flag to sync to trle

```shell
python tombll_manage_data.py -h
python3 tombll_manage_data.py -sc

```

For testing widescreen patch
it could work with only tr4 and tr5
compile with cmake -DTEST=on

```shell
./TombRaiderLinuxLauncherTest -w tomb4.exe
```

I was going to mix trle.net with trcustoms.org data, I have not made contacted with the site owner
to ask if I can use the site for scraping for non commercial use. As this task turned out to be
harder than I thought, to match data without creating doubles, I'm gonna wait until the basics
and trle.net part is implemented.

## Screenshots

![screenshot1](https://raw.githubusercontent.com/noisecode3/TombRaiderLinuxLauncher/main/doc/screenshot1.jpg)
![screenshot1](https://raw.githubusercontent.com/noisecode3/TombRaiderLinuxLauncher/main/doc/screenshot2.jpg)

## Guide

The game uses an old API with legacy instructions, timers, and single-threaded code, which means it can run on very low-end hardware — even a "potato" laptop.

On Linux, technologies like NTSYNC, GE-Proton, and sometimes dgVoodoo2 can significantly improve performance. However, dgVoodoo2 only works with specific Proton versions and may introduce new problems such as input lag or crashes. Therefore, it's recommended to try running the game without dgVoodoo2 first and use it only as a last resort.

I tested the original, unpatched Tomb Raider III on an i7-4800MQ using its integrated GPU, and launched it like this:

```shell
WINEPREFIX="/home/noisecode3/.newtombprefix" PROTONPATH="/home/noisecode3/.steam/steam/compatibilitytools.d/GE-Proton10-10" GAMEID="225320" MESA_SHADER_CACHE="true"
```
It ran nearly perfectly — only a tiny framedrop once per hour, or sometimes not at all. Timing jitter caused by Wine’s threading system is largely eliminated under NTSync.
NTSync can help DDraw, DirectSound other I/O thread workers in wine. On Arch Linux (or other distros with a modular kernel), you’ll need to manually load the ntsync driver to enable proper synchronization support:

```shell
sudo modprobe ntsync

```

I listed some open source modes that work well with proton/wine.

### TR1

- <https://github.com/LostArtefacts/TRX>

### TR2

- <https://github.com/Arsunt/TR2Main>

### TR3

- <https://github.com/Trxyebeep/tomb3>

### TR4

- <https://github.com/Trxyebeep/TOMB4>

### TR5

- <https://github.com/Trxyebeep/TOMB5>

## Targets

I program mostly on Slackware-current and Arch, I gonna try target and test SteamOS for second release.
I don't use Windows but it should build on both Windows and Linux with MinGw.
It's recommended as a standard to use Arch Linux or MSYS2 on Windows, with qt-creator.

## License

This project is licensed under GPLv3. Art assets are under Creative Commons Attribution-NonCommercial-ShareAlike 4.0. Tomb Raider is a trademark of Embracer Group AB.
