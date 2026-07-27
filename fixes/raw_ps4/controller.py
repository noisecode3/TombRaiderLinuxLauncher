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
from dataclasses import dataclass
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

    class DpadAxis:
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


class DirectionalButtons:
    """A cluster of ArrowButton objects, by 4 directions."""

    def __init__(self, ui):
        """Initialize an arrow button for the given direction."""
        self.up = ArrowButton(ui, "up")
        self.down = ArrowButton(ui, "down")
        self.left = ArrowButton(ui, "left")
        self.right = ArrowButton(ui, "right")

    def release_all(self):
        """Release all four buttons, e.g. on focus loss or disconnect."""
        for btn in (self.up, self.down, self.left, self.right):
            btn.set_release()

    def set_pressed_only(self, *directions):
        """Press the given directions, release everything else."""
        for name, btn in (("up", self.up), ("down", self.down),
                          ("left", self.left), ("right", self.right)):
            if name in directions:
                btn.set_pressed()
            else:
                btn.set_release()


class Stick:
    """Handle analog stick input and hold x and y."""

    def __init__(self, device, abs_x, abs_y):
        """
        Initialize generic joystick handler.

        Args:
            device: The uinput device.

                 (Up)
                 y=1
                  |
                  |
        x=-1 <----0----> x=1 (Right)
                  |
                  |
                 y=-1
                (Down)
        """
        # Read real hardware ranges
        abs_x = device.absinfo(abs_x)
        abs_y = device.absinfo(abs_y)

        self.center_y = (abs_y.min + abs_y.max) / 2.0
        self.half_range_y = (abs_y.max - abs_y.min) / 2.0

        self.center_x = (abs_x.min + abs_x.max) / 2.0
        self.half_range_x = (abs_x.max - abs_x.min) / 2.0

        self.current_x = 0.0
        self.current_y = 0.0

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


@dataclass
class SectorPoints:
    """8-way sector angle boundaries."""

    right_up: float = 20
    sub_right_up: float = 60
    left_up: float = 160
    sub_left_up: float = 120
    right_down: float = -20
    sub_right_down: float = -60
    left_down: float = -160
    sub_left_down: float = -120


class StickVector:
    """Handle analog stick input as a vector with threshold state."""

    def __init__(self, threshold=0.95, hysteresis=0.05, deadzone_delay=0.2):
        """
        Initialize generic joystick handler.

        Args:
            device: The uinput device.

                 (Up)
                  90°
            135°       45°

        +-180°            0° (Right)

           -135°      -45°
                 -90°
                (Down)
        """
        self.threshold = threshold
        self.in_deadzone = False
        self.in_deadzone_first_time = True
        # prevents flicker
        self.hysteresis = hysteresis
        self.deadzone_delay = deadzone_delay
        self.deadzone_enter_time = None
        self.angle = 0

    def process(self, current_x, current_y):
        """Generate lengt from middle and if over threashold also angle."""
        radius = math.hypot(current_x, current_y)

        # Bounce protection: right after entering the deadzone, a spring-loaded
        # stick can overshoot back past the threshold on release. Ignore all
        # readings for deadzone_delay seconds so that bounce can't be mistaken
        # for a new, intentional push.
        if self.in_deadzone and self.deadzone_enter_time is not None:
            if time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
                return

        # Apply hysteresis to prevent threshold flicker
        if radius > self.threshold:
            self.in_deadzone = False
            self.in_deadzone_first_time = True
            self.angle = math.degrees(math.atan2(current_y, current_x))
        else:
            if radius < (self.threshold - self.hysteresis):
                if not self.in_deadzone:
                    self.in_deadzone = True
                    self.deadzone_enter_time = time.monotonic()


class DefaultJoystickHandle:
    """Set controller to use Default direction detection."""

    def __init__(self, direction, stick_vector, on_deadzone, sector_points):
        """Set sector angle boundaries and callbacks for direction detection."""
        self.direction = direction
        self.stick_vector = stick_vector
        self.sector_points = sector_points
        self.on_deadzone = on_deadzone
        self.hysteresis = 2
        self.prev_major = None
        self.prev_diag = None

    @staticmethod
    def _adj(threshold, prev_state, lower_state, upper_state, h):
        """
        Shift a boundary threshold toward whichever side we were already on.

        So a value sitting right on the line doesn't flip back and forth
        every frame due to sensor jitter.
        threshold separates lower_state (angle < threshold) from
        upper_state (angle > threshold).
        """
        if prev_state == lower_state:
            return threshold + h
        if prev_state == upper_state:
            return threshold - h
        return threshold

    def handle_state(self):
        """Handle the states for the arrow keys."""
        if self.stick_vector.in_deadzone:
            self.on_deadzone()
            self.prev_major = None
            self.prev_diag = None
            return

        angle = self.stick_vector.angle
        sp = self.sector_points
        h = self.hysteresis

        # Recompute the four major boundaries with hysteresis applied
        right_up = self._adj(sp.right_up, self.prev_major, "right", "up", h)
        left_up = self._adj(sp.left_up, self.prev_major, "up", "left", h)
        left_down = self._adj(sp.left_down, self.prev_major, "left", "down", h)
        right_down = self._adj(sp.right_down, self.prev_major, "down", "right", h)

        # Up
        if right_up < angle < left_up:
            self.prev_major = "up"
            # Recompute diagonal sub-boundaries with hysteresis too
            sub_left_up = self._adj(sp.sub_left_up, self.prev_diag, "up_center", "up_left", h)
            sub_right_up = self._adj(sp.sub_right_up, self.prev_diag, "up_right", "up_center", h)
            if angle > sub_left_up:
                self.prev_diag = "up_left"
                self.direction.set_pressed_only("up", "left")
            elif angle < sub_right_up:
                self.prev_diag = "up_right"
                self.direction.set_pressed_only("up", "right")
            else:
                self.prev_diag = "up_center"
                self.direction.set_pressed_only("up")
        # Right
        elif right_down < angle < right_up:
            self.prev_major = "right"
            self.prev_diag = None
            self.direction.set_pressed_only("right")
        # Left
        elif (-180 < angle < left_down) or (180 > angle > left_up):
            self.prev_major = "left"
            self.prev_diag = None
            self.direction.set_pressed_only("left")
        # Down
        elif left_down < angle < right_down:
            self.prev_major = "down"
            sub_left_down = self._adj(
                sp.sub_left_down,
                self.prev_diag,
                "down_left",
                "down_center",
                h
            )
            sub_right_down = self._adj(
                sp.sub_right_down,
                self.prev_diag,
                "down_center",
                "down_right",
                h
            )
            if angle < sub_left_down:
                self.prev_diag = "down_left"
                self.direction.set_pressed_only("down", "left")
            elif angle > sub_right_down:
                self.prev_diag = "down_right"
                self.direction.set_pressed_only("down", "right")
            else:
                self.prev_diag = "down_center"
                self.direction.set_pressed_only("down")


class SectorSizeJoystickHandle:
    """Set controller to use direction detection based on sector size."""

    def __init__(self, direction, stick_vector, on_deadzone, sector_size=138):
        """Set sector angle boundaries and callbacks for direction detection."""
        self.direction = direction
        self.stick_vector = stick_vector
        self.sector_size = sector_size
        self.on_deadzone = on_deadzone

    def handle_state(self):
        """Handle the states for the arrow keys."""
        if self.stick_vector.in_deadzone:
            self.on_deadzone()
            return

        # Up
        if 90-self.sector_size/2 < self.stick_vector.angle < 90+self.sector_size/2:
            self.direction.up.set_pressed()
        else:
            self.direction.up.set_release()

        # Right
        if -self.sector_size/2 < self.stick_vector.angle < self.sector_size/2:
            self.direction.right.set_pressed()
        else:
            self.direction.right.set_release()

        # Left
        if self.stick_vector.angle < -180+self.sector_size/2 or \
                self.stick_vector.angle > 180-self.sector_size/2:
            self.direction.left.set_pressed()
        else:
            self.direction.left.set_release()

        # Down
        if -90-self.sector_size/2 < self.stick_vector.angle < -90+self.sector_size/2:
            self.direction.down.set_pressed()
        else:
            self.direction.down.set_release()


class LeftStick:
    """Handle analog stick input."""

    def __init__(self, ui, device, classic_overlap=False):
        """
        Initialize left joystick handler.

        Args:
            ui: The uinput device.
            device: The evdev controller input device.
            classic_overlap: if set there is only a simple 8-way stateless input

        """
        self.ui = ui
        self.stick = Stick(device, e.ABS_X, e.ABS_Y)
        self.stick_vector = StickVector()
        self.direction = DirectionalButtons(ui)

        self.state = {
            "look": [False],
            "clicked": [False],
            "classic_overlap": classic_overlap,
        }

        sector_points_default = SectorPoints()
        self.default_joystick_handle = DefaultJoystickHandle(
            self.direction,
            self.stick_vector,
            self.set_deadzone,
            sector_points_default,
        )
        self.sector_size_joystick_handle = SectorSizeJoystickHandle(
            self.direction,
            self.stick_vector,
            self.set_deadzone,
        )

    def set_deadzone(self):
        """Deactivate all arrow keys."""
        if self.stick_vector.in_deadzone_first_time:
            self.stick_vector.in_deadzone_first_time = False
        self.direction.release_all()

    def set_look(self, look):
        """Set reference to look state."""
        self.state["look"] = look

    def set_clicked(self, clicked):
        """Set reference to clicked state."""
        self.state["clicked"] = clicked

    def handle_event(self, event):
        """Handle the events."""
        self.stick.handle_event(event)
        self.stick_vector.process(self.stick.current_x, self.stick.current_y)
        if self.state["classic_overlap"] or self.state["look"][0]:
            self.sector_size_joystick_handle.handle_state()
        elif self.state["clicked"][0]:
            self.auto_run_joystick_handle.handle_state()
        else:
            self.default_joystick_handle.handle_state()


class Trigger:
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


class Key:
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
        self.thumb_clicked_last_time = time.monotonic()

    def set_look(self, look):
        """Set reference to look state."""
        self.look = look

    def set_thumb_click(self, thumb_clicked):
        """Set reference to thumbl clicked state."""
        self.thumb_clicked = thumb_clicked

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
                    now = time.monotonic()
                    if now - self.thumb_clicked_last_time < 0.6:
                        self.thumb_clicked[0] = not self.thumb_clicked[0]
                    else:
                        self.thumb_clicked_last_time = now
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
        self.look = [False]
        self.left_stick_clicked = [False]

    def add_dpad(self):
        """Add D-pad handler."""
        self.abs_handlers.append(Dpad(self.ui))

    def add_stick(self, classic=False):
        """Add analog stick handler."""
        stick = LeftStick(self.ui, self.device, classic)
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
