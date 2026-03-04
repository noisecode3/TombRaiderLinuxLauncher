"""
Controller input handler to map controller events to keyboard keys using evdev and UInput.

Game context-aware joystick input, Input Mapping Contexts (IMC)

Make movement flowing and safer from mistakes, while staying modern and adaptable.
From the dead zone, use no overlapping between movement states.
When running or stepping back, use normal 8-way overlapping.
When running, Lara shouldn’t step back unless the stick is pulled fully downward within a
narrow angle (fully rotated). To jump forward-right, the player must press jump + pull forward +
pull right(we ship overlapping) — so it cannot be mixed up with a right-side jump.
To stop, the player must return to dead zone, always. From standing still Lara can now do a
full rotation move — it cannot be mixed up with a running forward-right input. With Look key
activated, controls revert to classic 8-way equal-angle overlapping (traditional tank-style grid).

If the user want we can also fall back to classic 8-way analog stick.

The rest is just simple analog trigger event on/off mappings.

"""
import sys
import argparse
import math
import time
import evdev
from evdev import InputDevice, UInput, ecodes as e

# We test with PS4 for now but we will support all controllers.
CONTROLLER_NAMES = (
    "Wireless Controller",
    "Sony Interactive Entertainment Wireless Controller"
)


class DeviceManager:
    """Manages device discovery and virtual input setup."""

    def __init__(self):
        """Initialize UInput device and internal state."""
        self.device = None
        self.capabilities = None
        self.ui = UInput()

    def get_ui(self):
        """Return the virtual input device."""
        return self.ui

    def list_devices(self):
        """Return list of input devices as (name, path) tuples."""
        return [(evdev.InputDevice(path).name, path) for path in evdev.list_devices()]

    def set_device(self, controller_names=CONTROLLER_NAMES):
        """Search and select a supported controller device."""
        for name, path in self.list_devices():
            if name in controller_names:
                print(f"Using device: {name} at {path}")
                self.device = InputDevice(path)
                return self.device

        print("No supported controller found. Available devices:")
        for name, path in self.list_devices():
            print(f" - {name} at {path}")
        sys.exit(1)


class Dpad:
    """Handles D-pad input."""

    def __init__(self, ui):
        """
        Initialize D-Pad handler.

        Args:
            ui: The uinput device.
        """
        self.ui = ui
        self.last = {
            'left_x': False,
            'right_x': False,
            'up_y': False,
            'down_y': False
        }
        self.event_x = e.ABS_HAT0X
        self.event_y = e.ABS_HAT0Y

    def handle_x(self, value):
        """Handle x values."""
        if value == 0:
            if self.last['left_x']:
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                self.last['left_x'] = False
            elif self.last['right_x']:
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                self.last['right_x'] = False

        elif value == -1:
            if self.last['left_x']:
                return
            self.ui.write(e.EV_KEY, e.KEY_LEFT, 1)
            self.last['left_x'] = True

        elif value == 1:
            if self.last['right_x']:
                return
            self.ui.write(e.EV_KEY, e.KEY_RIGHT, 1)
            self.last['right_x'] = True
        self.ui.syn()

    def handle_y(self, value):
        """Handle y values."""
        if value == 0:
            if self.last['up_y']:
                self.ui.write(e.EV_KEY, e.KEY_UP, 0)
                self.last['up_y'] = False
            elif self.last['down_y']:
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 0)
                self.last['down_y'] = False

        elif value == -1:
            if self.last['up_y']:
                return
            self.ui.write(e.EV_KEY, e.KEY_UP, 1)
            self.last['up_y'] = True

        elif value == 1:
            if self.last['down_y']:
                return
            self.ui.write(e.EV_KEY, e.KEY_DOWN, 1)
            self.last['down_y'] = True
        self.ui.syn()

    def handle_event(self, event):
        """Handle the events."""
        if event.code == self.event_x:
            self.handle_x(event.value)
        if event.code == self.event_y:
            self.handle_y(event.value)


class StickIMC:  # pylint: disable=too-few-public-methods disable=too-many-instance-attributes
    """Handle analog stick input."""

    def __init__(self, ui, device, classic_overlap=False):
        """
        Initialize left joystick handler.

        Args:
            ui: The uinput device.

                (Up)
                 90°
           135°       45°

         180°            0° (Right)

          -135°     -45°
               -90°
              (Down)
        """
        self.ui = ui

        # Read real hardware ranges
        abs_x = device.absinfo(e.ABS_X)
        abs_y = device.absinfo(e.ABS_Y)

        self.center_y = (abs_y.min + abs_y.max) / 2.0
        self.half_range_y = (abs_y.max - abs_y.min) / 2.0

        self.center_x = (abs_x.min + abs_x.max) / 2.0
        self.half_range_x = (abs_x.max - abs_x.min) / 2.0

        self.threshold = 0.95   # activation threshold
        self.hysteresis = 0.05  # prevents flicker
        self.deadzone_delay = 0.2
        self.deadzone_enter_time = None

        self.current_x = 0.0
        self.current_y = 0.0

        self.classic_overlap = classic_overlap

        self.state = {
            "deadzone": False,
            "up": False,
            "right": False,
            "left": False,
            "down": False,
            "look": [False],
        }

    def set_look(self, look):
        """Set reference to lool state."""
        self.state["look"] = look

    def normalize_x(self, value):
        """Map any axis range to -1 .. 1."""
        return (value - self.center_x) / self.half_range_x

    def normalize_y(self, value):
        """Map any axis range to -1 .. 1."""
        return -(value - self.center_y) / self.half_range_y

    def handle_event(self, event):
        """Handle the events."""
        if event.code == e.ABS_X:
            self.current_x = self.normalize_x(event.value)

        elif event.code == e.ABS_Y:
            self.current_y = self.normalize_y(event.value)

        self.process()

    def process(self):
        """Generate lengt from middle and if over threashold also angle."""
        radius = math.hypot(self.current_x, self.current_y)

        # Apply hysteresis to prevent threshold flicker
        if radius > self.threshold:
            angle = math.degrees(math.atan2(self.current_y, self.current_x))
            if self.classic_overlap or self.state["look"][0]:
                self.handle_state_classic(True, angle)
            else:
                self.handle_state(True, angle)

        else:
            if radius < (self.threshold - self.hysteresis):
                if self.classic_overlap or self.state["look"][0]:
                    self.handle_state_classic(False, 0)
                else:
                    self.handle_state(False, 0)

    def set_deadzone(self):
        """Deactivate all arrow keys."""
        if not self.state["deadzone"]:
            self.state["deadzone"] = True
            self.deadzone_enter_time = time.monotonic()

            if self.state["up"]:
                self.state["up"] = False
                self.ui.write(e.EV_KEY, e.KEY_UP, 0)
            if self.state["right"]:
                self.state["right"] = False
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
            if self.state["left"]:
                self.state["left"] = False
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
            if self.state["down"]:
                self.state["down"] = False
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 0)
            self.ui.syn()

    # pylint: disable=too-many-statements too-many-branches
    def handle_state(self, active, angle):
        """Handle the states for the arrow keys."""
        if not active:
            self.set_deadzone()
            return

        if self.state["deadzone"]:
            if time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
                return
            self.state["deadzone"] = False

        sector_point_right_up = 45
        sector_point_left_up = 135
        sector_point_right_down = -45
        sector_point_left_down = -135

        # Up
        if sector_point_right_up < angle < sector_point_left_up:
            if not self.state["up"]:
                self.state["up"] = True
                if self.state["right"]:
                    self.state["right"] = False
                    self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                if self.state["left"]:
                    self.state["left"] = False
                    self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                if self.state["down"]:
                    self.state["down"] = False
                    self.ui.write(e.EV_KEY, e.KEY_DOWN, 0)
                self.ui.write(e.EV_KEY, e.KEY_UP, 1)
                self.ui.syn()

        # Right
        elif sector_point_right_down < angle < sector_point_right_up:
            if not self.state["right"]:
                self.state["right"] = True
                if self.state["left"]:
                    self.state["left"] = False
                    self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 1)
                self.ui.syn()

        # Left
        elif (-180 < angle < sector_point_left_down) or \
                (180 > angle > sector_point_left_up):
            if not self.state["left"]:
                self.state["left"] = True
                if self.state["right"]:
                    self.state["right"] = False
                    self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 1)
                self.ui.syn()

        # Down
        elif sector_point_left_down < angle < sector_point_right_down:
            if not self.state["down"]:
                self.state["down"] = True
                if self.state["right"]:
                    self.state["right"] = False
                    self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                if self.state["left"]:
                    self.state["left"] = False
                    self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                if self.state["up"]:
                    self.state["up"] = False
                    self.ui.write(e.EV_KEY, e.KEY_UP, 0)
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 1)
                self.ui.syn()

    # pylint: disable=too-many-statements too-many-branches
    def handle_state_classic(self, active, angle):
        """Handle the states for the arrow keys."""
        if not active:
            self.set_deadzone()
            return

        if self.state["deadzone"]:
            if time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
                return
            self.state["deadzone"] = False

        size_up = 138
        size_right = 138
        size_left = 138
        size_down = 138

        # Up
        if 90-size_up/2 < angle < 90+size_up/2:
            if not self.state["up"]:
                self.state["up"] = True
                self.ui.write(e.EV_KEY, e.KEY_UP, 1)
                self.ui.syn()
        else:
            if self.state["up"]:
                self.state["up"] = False
                self.ui.write(e.EV_KEY, e.KEY_UP, 0)
                self.ui.syn()

        # Right
        if -size_right/2 < angle < size_right/2:
            if not self.state["right"]:
                self.state["right"] = True
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 1)
                self.ui.syn()
        else:
            if self.state["right"]:
                self.state["right"] = False
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
                self.ui.syn()

        # Left
        if angle < -180+size_left/2 or angle > 180-size_left/2:
            if not self.state["left"]:
                self.state["left"] = True
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 1)
                self.ui.syn()
        else:
            if self.state["left"]:
                self.state["left"] = False
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
                self.ui.syn()

        # Down
        if -90-size_down/2 < angle < -90+size_down/2:
            if not self.state["down"]:
                self.state["down"] = True
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 1)
                self.ui.syn()
        else:
            if self.state["down"]:
                self.state["down"] = False
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 0)
                self.ui.syn()


class Trigger:  # pylint: disable=too-few-public-methods
    """Handles analog trigger input."""

    def __init__(self, ui, event_code, keyout):
        """
        Initialize the Trigger handler.

        Args:
            ui: The uinput device.
            event_code: The event code for the analog trigger (e.g., ABS_Z).
            keyout: The key code to output when the trigger is pressed.
        """
        self.ui = ui
        self.event_code = event_code
        self.keyout = keyout
        self.threshold = 200
        self.pressed = False

    def handle_event(self, event):
        """
        Handle analog trigger events and emit key presses accordingly.

        Args:
            event: An evdev input event.
        """
        if event.code == self.event_code:
            if event.value > self.threshold and not self.pressed:
                self.ui.write(e.EV_KEY, self.keyout, 1)
                self.pressed = True
            elif event.value <= self.threshold and self.pressed:
                self.ui.write(e.EV_KEY, self.keyout, 0)
                self.pressed = False
            self.ui.syn()


class Key:  # pylint: disable=too-few-public-methods
    """Handles key trigger input."""

    def __init__(self, ui, button_code, keyout):
        """
        Initialize the Key handler.

        Args:
            ui: The uinput device.
            button_code: The button event code (e.g., BTN_SOUTH).
            keyout: The key code to output (e.g., KEY_LEFTCTRL).
        """
        self.ui = ui
        self.button_code = button_code
        self.keyout = keyout
        self.look = None

    def set_look(self, look):
        """Set reference to lool state."""
        self.look = look

    def handle_event(self, event):
        """
        Handle digital button events and emit key presses/releases.

        Args:
            event: An evdev input event.
        """
        if event.code == self.button_code:
            if self.look is not None:
                if event.value == 1:
                    self.look[0] = True
                else:
                    self.look[0] = False

            self.ui.write(e.EV_KEY, self.keyout, event.value)
            self.ui.syn()


class Controller:
    """Manages input mappings for a game controller."""

    def __init__(self, ui, device):
        """
        Initialize the Controller with input handlers.

        Args:
            ui: The uinput device.
        """
        self.abs_handlers = []
        self.key_handlers = []
        self.ui = ui
        self.device = device
        self.look = [False]

    def add_dpad(self):
        """Add D-pad handler."""
        self.abs_handlers.append(Dpad(self.ui))

    def add_stick(self, classic):
        """Add analog stick handler."""
        stick = StickIMC(self.ui, self.device, classic_overlap=classic)
        stick.set_look(self.look)
        self.abs_handlers.append(stick)

    def add_trigger(self, event, keyout):
        """
        Add analog trigger handler.

        Usually called R2/L2 or ZR/ZL. They have a range
        from 0 to 255 and are not buttons.

        Args:
            event: The analog axis code (e.g., ABS_Z).
            keyout: The corresponding keyboard output code.
        """
        self.abs_handlers.append(Trigger(self.ui, event, keyout))

    def add_key(self, event, keyout):
        """
        Add digital button-to-key handler.

        Args:
            event: The gamepad button code.
            keyout: The keyboard key code to emit.
        """
        if keyout == e.KEY_KP0:
            key = Key(self.ui, event, keyout)
            key.set_look(self.look)
            self.key_handlers.append(key)
        else:
            self.key_handlers.append(Key(self.ui, event, keyout))

    def process_event(self, event):
        """
        Route input event to the appropriate handler.

        Args:
            event: An evdev input event.
        """
        if event.type == e.EV_ABS:
            for h in self.abs_handlers:
                h.handle_event(event)
        elif event.type == e.EV_KEY:
            for h in self.key_handlers:
                h.handle_event(event)


class Preset:
    """Sets up the input device and controller mappings."""

    def __init__(self):
        """Initialize device manager, input device, and controller."""
        self.manager = DeviceManager()
        self.device = self.manager.set_device()
        self.ui = self.manager.get_ui()
        self.controller = Controller(self.ui, self.device)

    def get_controller(self):
        """
        Return the controller instance.

        Returns:
            Controller: The controller object.
        """
        return self.controller

    def read_loop(self):
        """
        Start reading events from the device and handle them.

        This method runs until interrupted by the user.
        """
        print("Listening to controller... Press Ctrl+C to exit.")
        try:
            for event in self.device.read_loop():
                self.controller.process_event(event)
        except KeyboardInterrupt:
            print("\nExiting.")


def _ps4(overlap):
    preset = Preset()
    controller = preset.get_controller()
    controller.add_dpad()
    controller.add_stick(classic=overlap)
    controller.add_trigger(e.ABS_Z, e.KEY_DOT)
    controller.add_trigger(e.ABS_RZ, e.KEY_SLASH)
    controller.add_key(e.BTN_EAST, e.KEY_END)
    controller.add_key(e.BTN_SOUTH, e.KEY_LEFTCTRL)
    controller.add_key(e.BTN_WEST, e.KEY_LEFTALT)
    controller.add_key(e.BTN_NORTH, e.KEY_SPACE)
    controller.add_key(e.BTN_TL, e.KEY_KP0)
    controller.add_key(e.BTN_TR, e.KEY_LEFTSHIFT)
    controller.add_key(e.BTN_SELECT, e.KEY_COMMA)
    controller.add_key(e.BTN_START, e.KEY_ESC)
    controller.add_key(e.BTN_MODE, e.KEY_P)
    preset.read_loop()


def _only_ps4_share():
    preset = Preset()
    controller = preset.get_controller()
    controller.add_key(e.BTN_SELECT, e.KEY_COMMA)
    preset.read_loop()


def _only_left_stick():
    preset = Preset()
    controller = preset.get_controller()
    controller.add_stick(classic=False)
    preset.read_loop()


def _main():
    parser = argparse.ArgumentParser(
        description="PS4 Controller Mapper using evdev + uinput"
    )

    parser.add_argument(
        "mode",
        type=str,
        choices=["ps4", "ps4-overlap", "only-ps4-share", "only-left-stick"],
        help="Select input mapping mode",
    )
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')

    args = parser.parse_args()

    if args.mode == "ps4":
        _ps4(False)
    if args.mode == "ps4-overlap":
        _ps4(True)
    elif args.mode == "only-ps4-share":
        _only_ps4_share()
    elif args.mode == "only-left-stick":
        _only_left_stick()


if __name__ == "__main__":
    _main()
