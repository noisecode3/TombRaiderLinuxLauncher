# Tomb Raider Level Launcher

A tool for running and modding classic Tomb Raider games (1–5) on Windows (After Linux) and Linux with Wine/Proton. It also supports searching and downloading levels from trle.net, enabling direct play from the application. Obs repository will move to TombRaiderLevelLauncher.

This project is my passion for Tomb Raider classics combined with learning C++, Python, and SQL. My dream is to create an arcade machine featuring all classic Tomb Raider games and TRLE levels, potentially with a co-op mode.

## Features

- Integrates with Linux launchers (Steam, Lutris, etc.)
- Controller mapper support (e.g., qjoypad, antimicrox)
- Save file and configuration backups
- Workarounds for Linux/Wine compatibility issues
- Modding support for executable-based mods
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

Some levels wont be added because they use external or different download URL's
You can add levels to the database if you cd into where you installed you're database.
From you're browser copy the number from level page https://www.trle.net/sc/levelfeatures.php?lid=3684

If you did just follow the command above you can use:


```shell
python tombll_manage_data.py -h
python3 tombll_manage_data.py -a 3684

```
Now that you have an data.json file you get a chance to edit it.
Sometimes you need or want to edit those but right now I allow only trle.net
You can add you're own local file with its md5sum and it should not try to download it.
But it has to be a zip file at this point.

```text
  "zipFileName": "",
  "zipFileMd5": "",
  "download_url": ""
```

```shell
python3 tombll_manage_data.py -af data.json

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
![screenshot1](https://raw.githubusercontent.com/noisecode3/TombRaiderLinuxLauncher/main/doc/screenshot3.jpg)

## Guide

The game uses old API and old optimizations, but you can play basically on a potato laptop.
It depends on drivers quality, but with proton you can try `PROTON_USE_WINED3D=1`
or using [dgvoodoo2] with `PROTON_USE_WINED3D=0` usually you get the best performance with (DXVK/dgvoodoo2).
The easiest way to get this setup it using Lutris. Some levels can be lagging even on
a computer with a strong video card.
[dgvoodoo2]: https://github.com/lutris/dgvoodoo2/releases

### TR1
* <https://github.com/LostArtefacts/TRX>

### TR2
* <https://github.com/Arsunt/TR2Main>

### TR3
* <https://github.com/Trxyebeep/tomb3>

### TR4
* <https://github.com/Trxyebeep/TOMB4>

### TR5
* <https://github.com/Trxyebeep/TOMB5>

## Targets
I program mostly on Slackware-current and Arch, I gonna try target and test SteamOS for second release.
I don't use Windows but it should build on both Windows and Linux with MinGw. I will test with Wine/VM.
It's recommended as a standard to use Arch Linux or MSYS2 on Windows, with qt-creator.
Its possible to set it up with neovim or vscode or any other but its more work.

## License
This project is licensed under GPLv3. Art assets are under Creative Commons Attribution-NonCommercial-ShareAlike 4.0. Tomb Raider is a trademark of Embracer Group AB.
