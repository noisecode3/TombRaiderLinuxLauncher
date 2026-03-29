#!/bin/bash
set -e

cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"

git clone https://github.com/Kron4ek/Conty
cd Conty

settings_mininal () {
  awk '
  /^PACKAGES=\(/ {
    print "PACKAGES=("
    print "    qt6-wayland qt6-webengine qt6-imageformats qt6-svg"
    print "    python-pycurl python-tqdm python-cryptography python-beautifulsoup4 python-pillow"
    print "    umu-launcher protontricks gamemode"
    print ")"
    skip=1
    next
  }

  /^AUR_PACKAGES=\(/ {
    print "AUR_PACKAGES=("
    print ")"
    skip=2
    next
  }

  skip==1 && /^\)/ {skip=0; next}
  skip==2 && /^\)/ {skip=0; next}
  skip==3 && /^\)/ {skip=0; next}
  skip {next}
  1
  ' settings.sh > settings.sh.tmp && mv settings.sh.tmp settings.sh
}

settings_build () {
  awk '
  /^PACKAGES=\(/ {
    print "PACKAGES=("
    print "    base-devel cmake ccache"
    print "    qt6-wayland qt6-webengine qt6-imageformats qt6-svg"
    print "    python-pycurl python-tqdm python-cryptography python-beautifulsoup4 python-pillow"
    print "    umu-launcher protontricks wine-staging winetricks lutris steam gamemode"
    print "    lib32-vulkan-radeon vulkan-tools"
    print ")"
    skip=1
    next
  }

  /^AUR_PACKAGES=\(/ {
    print "AUR_PACKAGES=("
    print "    megatools"
    print ")"
    skip=2
    next
  }

  /^LOCALES=\(/ {
    print "LOCALES=("
    print "    en_US.UTF-8"
    print ")"
    skip=3
    next
  }

  skip==1 && /^\)/ {skip=0; next}
  skip==2 && /^\)/ {skip=0; next}
  skip==3 && /^\)/ {skip=0; next}
  skip {next}
  1
  ' settings.sh > settings.sh.tmp && mv settings.sh.tmp settings.sh
}

awk '
/^chroot .*\/bin\/bash$/ {
    print "chroot \"${bootstrap}\" /usr/bin/env LANG=en_US.UTF-8 TERM=xterm PATH=\"/bin:/sbin:/usr/bin:/usr/sbin\" /bin/bash -c \"\\"
    print "echo \047en_US.UTF-8 UTF-8\047 > /etc/locale.gen && locale-gen"
    print "echo \047LANG=en_US.UTF-8\047 > /etc/locale.conf"
    print "\""
    next
}
{ print }
' enter-chroot.sh > enter-chroot-noninteractive.sh
chmod +x enter-chroot-noninteractive.sh

case "${1:-}" in
  --minimal) settings_minimal ;;
  --devel)   settings_build ;;
  *)         settings_build ;;  # default
esac

# this id done so that if we need sudo after create-arch-bootstrap.sh
# we dont ask for password 2 times, we keep the session alive
MAIN_PID=$$
(
  start=$(date +%s)
  while kill -0 "$MAIN_PID" 2>/dev/null; do
    now=$(date +%s)
    [ $((now - start)) -gt 1800 ] && exit
    sudo -n -v 2>/dev/null
    sleep 60
  done
) &
SUDO_LOOP_PID=$!

cleanup() {
  kill "$SUDO_LOOP_PID" 2>/dev/null || true
}
trap cleanup EXIT

# We create the container 
sudo ./create-arch-bootstrap.sh
sudo ./enter-chroot-noninteractive.sh || true
./create-conty.sh

# clean up
if [ ! -d "$SCRIPT_DIR/Conty" ]; then
    echo "Error: Conty dir not found in current folder"
    exit 1
fi

mv "$SCRIPT_DIR/Conty/conty.sh" "$SCRIPT_DIR/conty.sh" 2>/dev/null || true
sudo rm -rf "$SCRIPT_DIR/Conty/root.x86_64"
sudo rm -f "$SCRIPT_DIR/Conty/pkglist.x86_64.txt"
sudo rm -f "$SCRIPT_DIR/Conty/version"
rm -rf "$SCRIPT_DIR/Conty"
