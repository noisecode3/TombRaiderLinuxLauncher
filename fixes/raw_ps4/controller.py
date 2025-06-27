"""PS4 controller input handler to map controller events to keyboard keys using evdev and UInput."""
import sys
import evdev
from evdev import InputDevice, UInput, ecodes as e

controller_names = (
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

    def set_device(self):
        """Search and select a supported controller device."""
        for name, path in self.list_devices():
            if name in controller_names:
                print(f"Using device: {name} at {path}")
                self.device = InputDevice(path)
                return self.device

        print("No supported controller found.")
        return None


class Dpad:
    """Handles D-pad input."""

    def __init__(self, ui):
        """
        Initialize D-Pad handler.

        Args:
            ui: The uinput device.
        """
        self.ui = ui
        self.last = {'left_x': False, 'right_x': False, 'up_y': False, 'down_y': False}
        self.event_x = e.ABS_HAT0X
        self.event_y = e.ABS_HAT0Y

    def handle_x(self, value):
        """Handle x valeus."""
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
        """Handle y valeus."""
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


class Stick:
    """Handles analog stick input."""

    def __init__(self, ui):
        """
        Initialize analog Stick handler.

        Args:
            ui: The uinput device.
        """
        self.ui = ui
        self.last_x = 0
        self.last_y = 0
        self.event_x = e.ABS_X
        self.event_y = e.ABS_Y
        self.threshold = {'left': 53, 'right': 202, 'up': 63, 'down': 192}

    def handle_x(self, value):
        """Handle x valeus."""
        if value < self.threshold['left']:
            if self.last_x == 1:
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
            self.ui.write(e.EV_KEY, e.KEY_LEFT, 1)
            self.last_x = -1
        elif value > self.threshold['right']:
            if self.last_x == -1:
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
            self.ui.write(e.EV_KEY, e.KEY_RIGHT, 1)
            self.last_x = 1
        else:
            if self.last_x == -1:
                self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
            elif self.last_x == 1:
                self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
            self.last_x = 0
        self.ui.syn()

    def handle_y(self, value):
        """Handle y valeus."""
        if value < self.threshold['up']:
            if self.last_y == 1:
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 0)
            self.ui.write(e.EV_KEY, e.KEY_UP, 1)
            self.last_y = -1
        elif value > self.threshold['down']:
            if self.last_y == -1:
                self.ui.write(e.EV_KEY, e.KEY_UP, 0)
            self.ui.write(e.EV_KEY, e.KEY_DOWN, 1)
            self.last_y = 1
        else:
            if self.last_y == -1:
                self.ui.write(e.EV_KEY, e.KEY_UP, 0)
            elif self.last_y == 1:
                self.ui.write(e.EV_KEY, e.KEY_DOWN, 0)
            self.last_y = 0
        self.ui.syn()

    def handle_event(self, event):
        """Handle the events."""
        if event.code == self.event_x:
            self.handle_x(event.value)
        elif event.code == self.event_y:
            self.handle_y(event.value)


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

        Usally call R2/L2 or ZR/ZL. They have an range
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


def print_info():
    """Print help info."""
    help_text = """
Usage: python3 controller.py [options]
  Options:
      -ps4             Start ps4 preset
      -only-ps4-share  Start ps4 share only preset (Tomb Raider I-III remaster)
      -only-left-stick Start ps4 stick only preset (TRX1)

"""
    print(help_text.strip())


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


if __name__ == "__main__":
    number_of_argument = len(sys.argv)
    if (number_of_argument == 1 or number_of_argument >= 3):
        print_info()
        sys.exit(1)

    elif (sys.argv[1] == "-h" and number_of_argument == 2):
        print_info()

    elif (sys.argv[1] == "-ps4" and number_of_argument == 2):
        _ps4()
    elif (sys.argv[1] == "-only-ps4-share" and number_of_argument == 2):
        _only_ps4_share()
    elif (sys.argv[1] == "-only-left-stick" and number_of_argument == 2):
        _only_left_stick()
    else:
        print_info()
