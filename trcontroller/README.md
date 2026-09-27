# TRController

Default configuration supports PS4 out of the box.

> **Pre-alpha release, full of bugs.**
> OBS: This is the first time I've made a modular program based on JSON.
> The project is currently in a "broken" state and I'm not happy with it yet.
> Every component clearly needs tests (a component = a controller/input part: key, trigger, dpad, or joystick).

## How it works

Customize input and output, and work with any controller.

```bash
python3 main.py template ps4-3to5 > /home/lara/mycontroller.json
```

Open `/home/lara/mycontroller.json` and make sure the file matches your controller's evdev event codes.

```bash
python3 main.py read "Wireless Controller"  # a filter for this is coming
```

Then start the controller:

```bash
python3 main.py run /home/lara/mycontroller.json
```

## Fixing `/dev/uinput` Permissions

### Quick way

```bash
sudo modprobe uinput
sudo chown root:input /dev/uinput
sudo chmod 0660 /dev/uinput
```

To make this persist across reboots, add it to `/etc/modules-load.d/uinput.conf`:

```bash
echo "uinput" | sudo tee /etc/modules-load.d/uinput.conf
```

### 1. Give your user access to `/dev/uinput`

By default, only `root` and the `input` group can access it.

Add yourself to the `input` group:

```bash
sudo usermod -aG input $USER
```

Log out and back in (or reboot) for the group change to take effect.

If that still doesn't work, you might need udev rules.

### 2. Create a udev rule to fix permissions

If `/dev/uinput` still has `crw-------` (root-only access), create a new rule:

```bash
sudo nano /etc/udev/rules.d/99-uinput.rules
```

Add this line:

```
KERNEL=="uinput", GROUP="input", MODE="0660", OPTIONS+="static_node=uinput"
```

Then apply the new rule:

```bash
sudo udevadm control --reload-rules && sudo udevadm trigger
```

Now check if the permissions are fixed:

```bash
ls -l /dev/uinput
```

It should look like:

```
crw-rw---- 1 root input 10, 223 Feb  1 4:20 /dev/uinput
```

If it still says `crw-------`, reboot.
