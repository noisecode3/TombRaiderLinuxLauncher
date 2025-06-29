"""PS4 controller input handler to map controller events to keyboard keys using evdev and UInput."""
import sys
import argparse
import evdev
from evdev import InputDevice, UInput, ecodes as e

CONTROLLER_NAMES = (
    "Wireless Controller",
    "Sony Interactive Entertainment Wireless Controller"
)


class DeviceManager:
    """Manages device discovery and virtual input setup."""

    def __init__(self):
        """Initialize UInput device and internal state."""
        self.device = None
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


class StickAxis:  # pylint: disable=too-few-public-methods
    """Handles one analog stick axis."""

    def __init__(self, ui, threshold, output_key):
        """
        Initialize analog Stick handler.

        Args:
            ui: The uinput device.
            threshold: Tuple of (low_threshold, high_threshold).
            output_key: Tuple of (negative_key, positive_key), e.g. (KEY_LEFT, KEY_RIGHT).
        """
        self.ui = ui
        self.last_state = 0
        self.threshold = threshold
        self.output_key = output_key

    def handle(self, value):
        """Handle axis values."""
        tr1, tr2 = self.threshold
        neg_key, pos_key = self.output_key

        if value < tr1:
            if self.last_state == -1:
                return
            if self.last_state == 1:
                self.ui.write(e.EV_KEY, pos_key, 0)
            self.ui.write(e.EV_KEY, neg_key, 1)
            self.last_state = -1
            self.ui.syn()

        elif value > tr2:
            if self.last_state == 1:
                return
            if self.last_state == -1:
                self.ui.write(e.EV_KEY, neg_key, 0)
            self.ui.write(e.EV_KEY, pos_key, 1)
            self.last_state = 1
            self.ui.syn()

        else:
            if self.last_state == -1:
                self.ui.write(e.EV_KEY, neg_key, 0)
                self.last_state = 0
                self.ui.syn()
            elif self.last_state == 1:
                self.ui.write(e.EV_KEY, pos_key, 0)
                self.last_state = 0
                self.ui.syn()


class Stick:  # pylint: disable=too-few-public-methods
    """Handles analog stick input."""

    def __init__(self, ui):
        """
        Initialize analog Stick handler.

        Args:
            ui: The uinput device.
        """
        self.ui = ui
        self.event_x = e.ABS_X
        self.event_y = e.ABS_Y

        self.x_axis = StickAxis(
            ui,
            threshold=(53, 202),
            output_key=(e.KEY_LEFT, e.KEY_RIGHT)
        )
        self.y_axis = StickAxis(
            ui, threshold=(63, 192),
            output_key=(e.KEY_UP, e.KEY_DOWN)
        )

    def handle_event(self, event):
        """Handle the events."""
        if event.code == self.event_x:
            self.x_axis.handle(event.value)
        elif event.code == self.event_y:
            self.y_axis.handle(event.value)


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

    def handle_event(self, event):
        """
        Handle digital button events and emit key presses/releases.

        Args:
            event: An evdev input event.
        """
        if event.code == self.button_code:
            self.ui.write(e.EV_KEY, self.keyout, event.value)
            self.ui.syn()


class Controller:
    """Manages input mappings for a game controller."""

    def __init__(self, ui):
        """
        Initialize the Controller with input handlers.

        Args:
            ui: The uinput device.
        """
        self.abs_handlers = []
        self.key_handlers = []
        self.ui = ui

    def add_dpad(self):
        """Add D-pad handler."""
        self.abs_handlers.append(Dpad(self.ui))

    def add_stick(self):
        """Add analog stick handler."""
        self.abs_handlers.append(Stick(self.ui))

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
        self.controller = Controller(self.ui)

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


def _ps4():
    preset = Preset()
    controller = preset.get_controller()
    controller.add_dpad()
    controller.add_stick()
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
    controller.add_stick()
    preset.read_loop()


def _main():
    parser = argparse.ArgumentParser(
        description="PS4 Controller Mapper using evdev + uinput"
    )

    parser.add_argument(
        "mode",
        type=str,
        choices=["ps4", "only-ps4-share", "only-left-stick"],
        help="Select input mapping mode",
    )
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')

    args = parser.parse_args()

    if args.mode == "ps4":
        _ps4()
    elif args.mode == "only-ps4-share":
        _only_ps4_share()
    elif args.mode == "only-left-stick":
        _only_left_stick()


if __name__ == "__main__":
    _main()
