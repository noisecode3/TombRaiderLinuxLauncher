# Installation

## Dependencies

On Arch this should be enough, you get the rest probably when you install the base, curl + openssl
Ensure the following are installed on your Linux desktop:

- `curl` (7.71.0 or newer)
- `OpenSSL`
- `Qt6`
- `Python3 (pycurl, tqdm, cryptography, beautifulsoup4, pillow)`

```shell
sudo pacman -S qt6-wayland qt6-webengine qt6-imageformats qt6-svg python-pycurl python-tqdm python-cryptography python-beautifulsoup4 python-pillow
paru -S megatools
```

### Ubuntu/Debian

```shell
sudo apt update
sudo apt-get install build-essential pkg-config cmake \
    qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools \
    qt6-declarative-dev qt6-multimedia-dev \
    qt6-webengine-dev qml6-module-qtwebengine \
    qt6-image-formats-plugins qt6-svg \
    libcurl4-openssl-dev python3-pycurl python3-tqdm \
    python3-cryptography python3-bs4 python3-pil git megatools
```

### Fedora

```shell
sudo dnf install python3-pycurl python3-tqdm python3-cryptography python3-beautifulsoup4 python3-pillow
sudo dnf install -y \
    curl curl-devel openssl openssl-devel git \
    qt6-qtbase qt6-qtbase-devel \
    qt6-qtwayland qt6-qtimageformats qt6-qtsvg \
    qt6-qtdeclarative qt6-qttools \
    qt6-qtwebengine qt6-qtwebengine-devel \
    python3 python3-pycurl python3-tqdm \
    python3-cryptography python3-beautifulsoup4 python3-pillow \
    megatools

```
qt6-qtwebengine-devel is required even for runtime use with QML/WebEngine apps.
curl-devel is needed for pycurl to link correctly.

### openSUSE

```shell
sudo zypper install -y \
    curl libcurl-devel openssl openssl-devel git \
    qt6-base-devel qt6-wayland \
    qt6-imageformats qt6-svg \
    qt6-declarative-devel qt6-tools-devel \
    qt6-webengine-devel \
    python3 python3-pycurl python3-tqdm \
    python3-cryptography python3-beautifulsoup4 python3-Pillow \
    megatools
```
WebEngine requires the -devel package even for runtime apps.

### Alpine Linux

```shell
sudo apk add \
    curl curl-dev openssl openssl-dev git \
    qt6-qtbase qt6-qtbase-dev \
    qt6-qtwayland qt6-qtimageformats qt6-qtsvg \
    qt6-qtdeclarative qt6-qttools \
    qt6-qtwebengine qt6-qtwebengine-dev \
    python3 py3-pycurl py3-tqdm \
    py3-cryptography py3-beautifulsoup4 py3-pillow \
    megatools
```

### Build
```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/.local -DRELEASE=on ..
make install -j$(nproc)
```

## Use only database

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
### pip

Never use sudo with pip, you can use a virtual environment or keep updating them in home.
If a required module isn't available in the system's package manager, fall back to pip.

```shell
python3 -m venv myenv
source myenv/bin/activate
pip install pycurl tqdm cryptography beautifulsoup4 pillow
```

To install python dependencies in home

```shell
pip install pycurl tqdm cryptography beautifulsoup4 pillow
```

How to update with pip

```shell
#!/bin/bash
pimp_my_pip() {
    pip list --outdated | awk 'NR > 2 {print $1}' | xargs -n1 pip install -U
}
pimp_my_pip
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

