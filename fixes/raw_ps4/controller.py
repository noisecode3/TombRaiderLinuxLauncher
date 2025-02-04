import sys
import evdev
from evdev import UInput, ecodes as e

# Open your PS4 controller device
controller_names = (
    "Wireless Controller",
    "Sony Interactive Entertainment Wireless Controller"
)

device = None
for device_path in evdev.list_devices():
    temp_device = evdev.InputDevice(device_path)
    if temp_device.name in controller_names:
        device = temp_device
        break

if device == None:
    print("No device found")
    sys.exit(1)

# Create a virtual keyboard
ui = UInput()

# They must have last value and dead zone state to not interfere and remove the noise
LASTX = 0
LASTY = 0
INDEADZONE_X = True
INDEADZONE_Y = True
INDEADZONE_Z = True
INDEADZONE_RZ = True

print(f"Listening for input from {device.name}...")

for event in device.read_loop():
    if event.type == e.EV_ABS:  # Detects D-pad, Sticks and Triggers
        if event.code == e.ABS_HAT0X:  # Left (-1) / Right (1)
            if event.value == -1:  # Press Left Arrow
                LASTX = -1
                ui.write(e.EV_KEY, e.KEY_LEFT, 1)
                ui.syn()
            elif event.value == 1:  # Press Right Arrow
                LASTX = 1
                ui.write(e.EV_KEY, e.KEY_RIGHT, 1)
                ui.syn()
            elif event.value == 0 and LASTX == -1:  # Release Left Arrow
                LASTX = 0
                ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                ui.syn()
            elif event.value == 0 and LASTX == 1:  # Release Right Arrow
                LASTX = 0
                ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                ui.syn()

        if event.code == e.ABS_HAT0Y:  # Up (-1) / Down (1)
            if event.value == -1:  # Press Up Arrow
                LASTY = -1
                ui.write(e.EV_KEY, e.KEY_UP, 1)
                ui.syn()
            elif event.value == 1:  # Press Down Arrow
                LASTY = 1
                ui.write(e.EV_KEY, e.KEY_DOWN, 1)
                ui.syn()
            elif event.value == 0 and LASTY == -1:  # Release Up Arrow
                LASTY = 0
                ui.write(e.EV_KEY, e.KEY_UP, 0)
                ui.syn()
            elif event.value == 0 and LASTY == 1:  # Release Down Arrow
                LASTY = 0
                ui.write(e.EV_KEY, e.KEY_DOWN, 0)
                ui.syn()
        """
        Sticks and Triggers
        ABS_X and ABS_Y 0 means left stick is most to the left or most down
            227-228 is the middle 255 is most to the right or most up.
            dead zone should be something like 64 to 191.

        We then have the trigger ABS_Z L2 0 means not pushed down
            and 255 means most pushed down.

        We then have the same on the right, stick and R2.
        ABS_RX, ABS_RY and ABS_RZ. We might not use ABS_RX and ABS_RY.

        Stick must have a threshold and pair of ways or single way
        so it can trigger one or two ways in the game.
        This is called 8-way digital stick. We can start simple
        with only a threshold. We should at lest calculate a circle
        threshold we can start with a 8 way square.
        """
        if event.code == e.ABS_X:  # Left stick X
            if event.value < 63:  # Left threshold
                INDEADZONE_X = False
                ui.write(e.EV_KEY, e.KEY_LEFT, 1)  # Press Left Arrow
                ui.syn()
            elif event.value > 192: # Right threshold
                INDEADZONE_X = False
                ui.write(e.EV_KEY, e.KEY_RIGHT, 1)  # Press Right Arrow
                ui.syn()
            elif not INDEADZONE_X:
                INDEADZONE_X = True
                ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                ui.syn()

        elif event.code == e.ABS_Y:  # Left stick Y
            if event.value < 63:  # Up threshold
                INDEADZONE_Y = False
                ui.write(e.EV_KEY, e.KEY_UP, 1)  # Press Up Arrow
                ui.syn()
            elif event.value > 192: # Down threshold
                INDEADZONE_Y = False
                ui.write(e.EV_KEY, e.KEY_DOWN, 1)  # Press Down Arrow
                ui.syn()
            elif not INDEADZONE_Y:
                INDEADZONE_Y = True
                ui.write(e.EV_KEY, e.KEY_UP, 0)
                ui.write(e.EV_KEY, e.KEY_DOWN, 0)
                ui.syn()

        elif event.code == e.ABS_Z:  # Left trigger Z
            if event.value > 200:
                INDEADZONE_Z = False
                ui.write(e.EV_KEY, e.KEY_S, 1)  # Activate S
                ui.syn()
            elif not INDEADZONE_Z:
                INDEADZONE_Z = True
                ui.write(e.EV_KEY, e.KEY_S, 0)  # Deactivate S
                ui.syn()

        elif event.code == e.ABS_RZ:  # Right trigger Z
            if event.value > 200:
                INDEADZONE_RZ = False
                ui.write(e.EV_KEY, e.KEY_C, 1)  # Activate C
                ui.syn()
            elif not INDEADZONE_RZ:
                INDEADZONE_RZ = True
                ui.write(e.EV_KEY, e.KEY_C, 0)  # Deactivate C
                ui.syn()

    if event.type == e.EV_KEY:  # Detects simple click buttons
        """
        start at 1 ends at 0 when lifted, super simple.
        BTN_EAST Ring
        BTN_SOUTH Cross
        BTN_WEST Square
        BTN_NORTH Triangle

        BTN_TL L1
        BTN_TR R1

        BTN_SELECT Share
        BTN_START Options

        """
        if event.code == e.BTN_EAST:  # Ring
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_END, 1)  # Activate END
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_END, 0)  # Deactivate END
                ui.syn()

        if event.code == e.BTN_SOUTH:  # Cross
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_LEFTCTRL, 1)  # Activate Left CTRL
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_LEFTCTRL, 0)  # Deactivate Left CTRL
                ui.syn()

        if event.code == e.BTN_WEST:  # Square
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_LEFTALT, 1)  # Activate Left ALT
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_LEFTALT, 0)  # Deactivate Left ALT
                ui.syn()

        if event.code == e.BTN_NORTH:  # Triangle
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_SPACE, 1)  # Activate SPACE
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_SPACE, 0)  # Deactivate SPACE
                ui.syn()

        if event.code == e.BTN_TL:  # L1
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_KP0, 1)  # Activate Numpad 0
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_KP0, 0)  # Deactivate Numpad 0
                ui.syn()

        if event.code == e.BTN_TR:  # R1
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_LEFTSHIFT, 1)  # Activate LEFTSHIFT
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_LEFTSHIFT, 0)  # Deactivate LEFTSHIFT
                ui.syn()

        if event.code == e.BTN_SELECT:  # Share
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_COMMA, 1)  # Activate COMMA
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_COMMA, 0)  # Deactivate COMMA
                ui.syn()

        if event.code == e.BTN_START:  # Options
            if event.value == 1:
                ui.write(e.EV_KEY, e.KEY_ESC, 1)  # Activate ESC
                ui.syn()
            else:
                ui.write(e.EV_KEY, e.KEY_ESC, 0)  # Deactivate ESC
                ui.syn()


# This function implements an analog-to-digital conversion with angular thresholding,
# where the joystick angle determines whether to send a full digital press or a pulsed key press.
# This makes the joystick act more smooth to Lara's turning movement, 
# def map_analog_to_digital(joystick_input):
    # ... implementation details ...

