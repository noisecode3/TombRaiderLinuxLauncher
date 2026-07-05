"""
Controller input handler to map controller events to keyboard keys using evdev and UInput.

Game context-aware joystick input, Input Mapping Contexts (IMC)

The Joysticks have states activated when pulled down.
Use left-stick running or stepping back, use normal 8-way overlapping.
If left-stick pulled down it toggle Auto-Run. Lara will then run even when in dead zone
and if pulled up and then sliglty to the sides the arrow key will pulseate untill pulled
fully to the side. When pulled fully to the side both arrow up and side will be pulled
down at the same time. If pulled fully back och if left-stick pulled down again, then exit
Auto-Run mode.


If the user want we can also fall back to classic 8-way analog stick.

The rest is just simple analog trigger event on/off mappings.

"""
import sys
import argparse
import math
import time
import threading
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


class Dpad:  # pylint: disable=too-few-public-methods
    """Handles D-pad input."""

    class DpadAxis:  # pylint: disable=too-few-public-methods
        """Handles D-pad Axis."""

        def __init__(self, neg_key, pos_key):
            """
            Initialize D-Pad Axis.

            Args:
                neg_key: evdev ecodes number (like KEY_LEFT).
                pos_key: evdev ecodes number (like KEY_RIGHT).
            """
            self.neg_key = neg_key
            self.pos_key = pos_key
            self.neg_active = False
            self.pos_active = False

        def handle(self, ui, value):
            """
            Initialize D-Pad Axis.

            Args:
                ui: The uinput device.
                value: -1 or 1.
            """
            if value == 0:
                if self.neg_active:
                    ui.write(e.EV_KEY, self.neg_key, 0)
                    self.neg_active = False
                elif self.pos_active:
                    ui.write(e.EV_KEY, self.pos_key, 0)
                    self.pos_active = False
            elif value == -1 and not self.neg_active:
                ui.write(e.EV_KEY, self.neg_key, 1)
                self.neg_active = True
            elif value == 1 and not self.pos_active:
                ui.write(e.EV_KEY, self.pos_key, 1)
                self.pos_active = True
            ui.syn()

    def __init__(self, ui):
        """
        Initialize D-Pad handler.

        Args:
            ui: The uinput device.
        """
        self.ui = ui
        self.x = self.DpadAxis(e.KEY_LEFT, e.KEY_RIGHT)
        self.y = self.DpadAxis(e.KEY_UP, e.KEY_DOWN)

    def handle_event(self, event):
        """Handle x and y separately."""
        if event.code == e.ABS_HAT0X:
            self.x.handle(self.ui, event.value)
        elif event.code == e.ABS_HAT0Y:
            self.y.handle(self.ui, event.value)


class APRGenerator:
    """Arrow-keys press and release generator."""

    def __init__(self, ui):
        """Initialize interval, key direction and event handler."""
        self.press_interval = 1.0
        self.release_interval = 1.0
        self.direction = None
        self.running = False
        self.wakeup = threading.Event()
        self.ui = ui

    def set_speed(self, interval):
        """Set different interval, and wakeup."""
        self.press_interval = interval
        self.release_interval = 1.0 - interval
        self.wakeup.set()

    def start(self, direction):
        """Set key direction and start the thread."""
        if not self.running:
            self.running = True
            self.direction = direction
            threading.Thread(target=self._run, daemon=True).start()

    def stop(self):
        """Reset and stop."""
        self.running = False
        self.wakeup.set()
        self.generate_release()

    def generate_press(self):
        """Press the key."""
        if self.direction == "left":
            self.ui.write(e.EV_KEY, e.KEY_LEFT, 1)
        elif self.direction == "right":
            self.ui.write(e.EV_KEY, e.KEY_RIGHT, 1)
        self.ui.syn()

    def generate_release(self):
        """Release the key."""
        if self.direction == "left":
            self.ui.write(e.EV_KEY, e.KEY_LEFT, 0)
        elif self.direction == "right":
            self.ui.write(e.EV_KEY, e.KEY_RIGHT, 0)
        self.ui.syn()

    def _run(self):
        cycle_start = time.monotonic()

        while self.running:

            now = time.monotonic()
            phase = (now - cycle_start) % 1.0

            if phase < self.press_interval:
                self.generate_press()
                wait = self.press_interval - phase
            else:
                self.generate_release()
                wait = 1.0 - phase

            self.wakeup.wait(wait)
            self.wakeup.clear()


class ArrowButton:
    """Arrow button state and key press/release handling."""

    def __init__(self, ui, direction):
        """Initialize an arrow button for the given direction."""
        self.ui = ui
        self.key = None
        self.pressed = False

        if direction == "up":
            self.key = e.KEY_UP
        elif direction == "right":
            self.key = e.KEY_RIGHT
        elif direction == "left":
            self.key = e.KEY_LEFT
        elif direction == "down":
            self.key = e.KEY_DOWN
        else:
            print("Object Error: Unknown Direction")
            sys.exit(1)

    def set_pressed(self):
        """Send a key press event if not already pressed."""
        if not self.pressed:
            self.pressed = True
            self.ui.write(e.EV_KEY, self.key, 1)
            self.ui.syn()

    def set_release(self):
        """Send a key release event if currently pressed."""
        if self.pressed:
            self.pressed = False
            self.ui.write(e.EV_KEY, self.key, 0)
            self.ui.syn()


class StickIMC:  # pylint: disable=too-many-instance-attributes
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
            "deadzone": [False],
            "look": [False],
            "clicked": [False],
        }

        self.up = ArrowButton(ui, "up")
        self.down = ArrowButton(ui, "down")
        self.left = ArrowButton(ui, "left")
        self.right = ArrowButton(ui, "right")
        self.apr_generator = APRGenerator(ui)

    def set_deadzone_state(self, deadzone):
        """Set reference to deadzone state."""
        self.state["deadzone"] = deadzone

    def set_look(self, look):
        """Set reference to look state."""
        self.state["look"] = look

    def set_clicked(self, clicked):
        """Set reference to clicked state."""
        self.state["clicked"] = clicked

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
            elif self.state["clicked"][0]:

                self.handle_run_state(True, angle)
            else:
                self.handle_state(True, angle)

        else:
            if radius < (self.threshold - self.hysteresis):
                if self.classic_overlap or self.state["look"][0]:
                    self.handle_state_classic(False, 0)
                elif self.state["clicked"][0]:
                    self.handle_run_state(False, 0)
                else:
                    self.handle_state(False, 0)

    def set_deadzone(self):
        """Deactivate all arrow keys."""
        if not self.state["deadzone"][0]:
            self.state["deadzone"][0] = True
            self.deadzone_enter_time = time.monotonic()
            self.up.set_release()
            self.down.set_release()
            self.left.set_release()
            self.right.set_release()
            if self.apr_generator.running:
                self.apr_generator.stop()

    # pylint: disable=too-many-statements too-many-branches
    def handle_run_state(self, active, angle):
        """Handle the states for the arrow keys."""
        if not active:
            if not self.state["deadzone"][0]:
                self.state["deadzone"][0] = True
                self.deadzone_enter_time = time.monotonic()
                self.up.set_pressed()
                self.down.set_release()
                self.left.set_release()
                self.right.set_release()
                if self.apr_generator.running:
                    self.apr_generator.stop()
            return

        if self.state["deadzone"][0]:
            if time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
                return
            self.state["deadzone"][0] = False

        sector_point_right_up = 65
        sector_point_left_up = 115
        sector_point_right_down = -65
        sector_point_left_down = -115

        # Up
        if sector_point_right_up < angle < sector_point_left_up:
            if angle > 100:
                self.apr_generator.set_speed((angle-100)/15)
                if not self.apr_generator.running:
                    self.up.set_pressed()
                    self.right.set_release()
                    self.apr_generator.start("left")

            elif angle < 80:
                self.apr_generator.set_speed(-1.0*((80-angle)/15))
                if not self.apr_generator.running:
                    self.up.set_pressed()
                    self.left.set_release()
                    self.apr_generator.start("right")
            else:
                self.up.set_pressed()
                self.down.set_release()
                self.left.set_release()
                self.right.set_release()

        # Right
        elif sector_point_right_down < angle < sector_point_right_up:
            if self.apr_generator.running:
                self.apr_generator.stop()
            self.left.set_release()
            self.right.set_pressed()

        # Left
        elif (-180 < angle < sector_point_left_down) or \
                (180 > angle > sector_point_left_up):
            if self.apr_generator.running:
                self.apr_generator.stop()
            self.left.set_pressed()
            self.right.set_release()

        # Down
        elif sector_point_left_down < angle < sector_point_right_down:
            if self.apr_generator.running:
                self.apr_generator.stop()
            self.up.set_release()
            self.down.set_release()
            self.left.set_release()
            self.right.set_release()
            self.state["clicked"][0] = False
            self.state["deadzone"][0] = False

    # pylint: disable=too-many-statements too-many-branches
    def handle_state(self, active, angle):
        """Handle the states for the arrow keys."""
        if not active:
            self.set_deadzone()
            return

        if self.state["deadzone"][0]:
            if time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
                return
            self.state["deadzone"][0] = False

        sector_point_right_up = 25
        sector_point_left_up = 155
        sector_point_right_down = -25
        sector_point_left_down = -155

        # Up
        if sector_point_right_up < angle < sector_point_left_up:
            if angle > 120:
                self.up.set_pressed()
                self.down.set_release()
                self.left.set_pressed()
                self.right.set_release()
            elif angle < 60:
                self.up.set_pressed()
                self.down.set_release()
                self.right.set_pressed()
                self.left.set_release()
            else:
                self.up.set_pressed()
                self.down.set_release()
                self.left.set_release()
                self.right.set_release()

        # Right
        elif sector_point_right_down < angle < sector_point_right_up:
            self.left.set_release()
            self.right.set_pressed()
            self.up.set_release()
            self.down.set_release()

        # Left
        elif (-180 < angle < sector_point_left_down) or \
                (180 > angle > sector_point_left_up):
            self.left.set_pressed()
            self.right.set_release()
            self.up.set_release()
            self.down.set_release()

        # Down
        elif sector_point_left_down < angle < sector_point_right_down:
            if angle < -120:
                self.up.set_release()
                self.down.set_pressed()
                self.left.set_pressed()
                self.right.set_release()
            elif angle > -60:
                self.up.set_release()
                self.down.set_pressed()
                self.right.set_pressed()
                self.left.set_release()
            else:
                self.up.set_release()
                self.down.set_pressed()
                self.left.set_release()
                self.right.set_release()

    # pylint: disable=too-many-statements too-many-branches
    def handle_state_classic(self, active, angle):
        """Handle the states for the arrow keys."""
        if not active:
            self.set_deadzone()
            return

        if self.state["deadzone"][0]:
            if time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
                return
            self.state["deadzone"][0] = False

        size_up = 138
        size_right = 138
        size_left = 138
        size_down = 138

        # Up
        if 90-size_up/2 < angle < 90+size_up/2:
            self.up.set_pressed()
        else:
            self.up.set_release()

        # Right
        if -size_right/2 < angle < size_right/2:
            self.right.set_pressed()
        else:
            self.right.set_release()

        # Left
        if angle < -180+size_left/2 or angle > 180-size_left/2:
            self.left.set_pressed()
        else:
            self.left.set_release()

        # Down
        if -90-size_down/2 < angle < -90+size_down/2:
            self.down.set_pressed()
        else:
            self.down.set_release()


class Trigger:  # pylint: disable=too-few-public-methods
    """Handles analog trigger input."""

    def __init__(self, ui, event_code, keyout, device):
        """
        Initialize the Trigger handler.

        Args:
            ui: The uinput device.
            event_code: The event code for the analog trigger (e.g., ABS_Z).
            keyout: The key code to output when the trigger is pressed.
        """
        self.range = 1
        self.threshold = 0.80

        if event_code == e.ABS_Z:
            abs_z = device.absinfo(e.ABS_Z)
            self.range = abs_z.max - abs_z.min
            self.threshold = 0.90

        self.ui = ui
        self.event_code = event_code
        self.keyout = keyout
        self.pressed = False

    def handle_event(self, event):
        """
        Handle analog trigger events and emit key presses accordingly.

        Args:
            event: An evdev input event.
        """
        if event.code == self.event_code:
            value = event.value / self.range
            if value > self.threshold and not self.pressed:
                self.ui.write(e.EV_KEY, self.keyout, 1)
                self.pressed = True
            elif value <= self.threshold and self.pressed:
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
        self.thumb_clicked = None
        self.deadzone_state = None

    def set_look(self, look):
        """Set reference to look state."""
        self.look = look

    def set_thumb_click(self, thumb_clicked):
        """Set reference to thumbl clicked state."""
        self.thumb_clicked = thumb_clicked

    def set_deadzone_state(self, deadzone_state):
        """Set reference to deadzone_state state."""
        self.deadzone_state = deadzone_state

    def handle_event(self, event):
        """
        Handle digital button events and emit key presses/releases.

        Args:
            event: An evdev input event.
        """
        if event.code == self.button_code:
            if self.look is not None and self.button_code is e.BTN_TL:
                if event.value == 1:
                    self.look[0] = True
                else:
                    self.look[0] = False
                    self.ui.write(e.EV_KEY, self.keyout, event.value)
                    self.ui.syn()
            elif self.thumb_clicked is not None and self.button_code is e.BTN_THUMBL:
                if event.value == 1:
                    if self.thumb_clicked[0]:
                        self.thumb_clicked[0] = False
                        self.deadzone_state[0] = False
                    else:
                        self.thumb_clicked[0] = True
                        self.deadzone_state[0] = False
            else:
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
        self.deadzone_state = [False]
        self.look = [False]
        self.left_stick_clicked = [False]

    def add_dpad(self):
        """Add D-pad handler."""
        self.abs_handlers.append(Dpad(self.ui))

    def add_stick(self, classic=False):
        """Add analog stick handler."""
        stick = StickIMC(self.ui, self.device, classic)
        stick.set_deadzone_state(self.deadzone_state)
        stick.set_look(self.look)
        stick.set_clicked(self.left_stick_clicked)
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
        self.abs_handlers.append(Trigger(self.ui, event, keyout, self.device))

    def add_key(self, event, keyout):
        """
        Add digital button-to-key handler.

        Args:
            event: The gamepad button code.
            keyout: The keyboard key code to emit.
        """
        if (event == e.BTN_THUMBL) and (keyout is None):
            key = Key(self.ui, event, keyout)
            key.set_thumb_click(self.left_stick_clicked)
            key.set_deadzone_state(self.deadzone_state)
            self.key_handlers.append(key)
        elif keyout == e.KEY_KP0:
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
    controller.add_key(e.BTN_THUMBL, None)
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
