#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"
CONTY_DIR="$SCRIPT_DIR/Conty"

# this is done so that if we need sudo after create-arch-bootstrap.sh
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

# unmount any leftover chroot binds before we touch anything
# to avoid the EPERM-storm and total system crash from rm -fr
safe_remove_conty() {
  local dir="$1"
  [ -d "$dir" ] || return 0

  local mounts
  mounts=$(awk -v d="$dir" '
    { p=$2; gsub(/\\040/," ",p);
      if (p==d || index(p, d "/")==1) print p }
  ' /proc/mounts 2>/dev/null | sort -r) || true

  local mp
  for mp in $mounts; do
    sudo umount -R "$mp" 2>/dev/null || sudo umount -l "$mp" 2>/dev/null || true
  done

  sudo rm -rf --one-file-system "$dir"
}

cleanup() {
  safe_remove_conty "$CONTY_DIR"
  kill "$SUDO_LOOP_PID" 2>/dev/null || true
}
trap cleanup EXIT

# wipe any leftovers from a previous failed/partial run -
# root.x86_64 etc. are root-owned so a plain rm won't always clear them
safe_remove_conty "$CONTY_DIR"

git clone https://github.com/Kron4ek/Conty
cd "$CONTY_DIR"

settings_minimal () {
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
  skip {next}
  1
  ' settings.sh > settings.sh.tmp && mv settings.sh.tmp settings.sh
}

settings_build () {
  awk '
  /^PACKAGES=\(/ {
    print "PACKAGES=("
    print "    base-devel cmake ccache meson"
    print "    qt6-wayland qt6-webengine qt6-imageformats qt6-svg"
    print "    python-pycurl python-tqdm python-cryptography python-beautifulsoup4 python-pillow"
    print "    umu-launcher protontricks wine-staging winetricks lutris steam gamemode"
    print "    lib32-vulkan-radeon vulkan-tools vulkan-headers"
    print "    kvantum qt6ct"
    print "    curl glib2"
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
    print "cd /root"
    print "curl -L -O https://xff.cz/megatools/builds/builds/megatools-1.11.5.20250706.tar.gz"
    print "echo \04736f18f41f6e6323e34d939cb06086048 megatools-1.11.5.20250706.tar.gz\047 | md5sum -c || exit 1"
    print "tar xvzf megatools-1.11.5.20250706.tar.gz"
    print "rm megatools-1.11.5.20250706.tar.gz"
    print "cd megatools-1.11.5.20250706"
    print "meson setup --prefix /usr --libexecdir lib --sbindir bin --buildtype plain --auto-features enabled --wrap-mode nodownload -D b_pie=true -D symlinks=true build"
    print "ninja -C build"
    print "DESTDIR=\047/\047 ninja -C build install"
    print "rm -fr /root/megatools-1.11.5.20250706"
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

sudo ./create-arch-bootstrap.sh
sudo ./enter-chroot-noninteractive.sh || true
./create-conty.sh

# pull the output out before we nuke the whole build tree
if [ -f "$CONTY_DIR/conty.sh" ]; then
  mv "$CONTY_DIR/conty.sh" "$SCRIPT_DIR/conty.sh"
else
  echo "Error: create-conty.sh didn't produce conty.sh" >&2
  exit 1
fi
