"""
Controller input handler to map controller events to keyboard keys using evdev and UInput.

One way game context joystick input, Input Mapping Contexts (IMC)

The Joysticks can have states activated when pulled down. Especially the right stick
can active shortcuts like.


"""
import argparse
import math
import sys
import time
from dataclasses import dataclass

import evdev
from evdev import InputDevice, UInput
from evdev import ecodes as e


class Reference:
    def __init__(self, value):
        self.value = value
    def set(self, value):
        self.value = value
    def get(self):
        return self.value


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
        self.abs_x = abs_x
        self.abs_y = abs_y

        absinfo_x = device.absinfo(abs_x)
        absinfo_y = device.absinfo(abs_y)

        self.center_y = (absinfo_y.min + absinfo_y.max) / 2.0
        self.half_range_y = (absinfo_y.max - absinfo_y.min) / 2.0

        self.center_x = (absinfo_x.min + absinfo_x.max) / 2.0
        self.half_range_x = (absinfo_x.max - absinfo_x.min) / 2.0

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
        if event.code == self.abs_x:
            self.current_x = self.normalize_x(event.value)

        elif event.code == self.abs_y:
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

        +180°            0° (Right)

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
        if self.in_deadzone and self.deadzone_enter_time is not None \
                and time.monotonic() - self.deadzone_enter_time < self.deadzone_delay:
            return

        # Apply hysteresis to prevent threshold flicker
        if radius > self.threshold:
            self.in_deadzone = False
            self.in_deadzone_first_time = True
            self.angle = math.degrees(math.atan2(current_y, current_x))
        else:
            if radius < (self.threshold - self.hysteresis) \
                    and not self.in_deadzone:
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
        if left_up > angle > right_up:
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


class FourwayTriggerJoystickHandle:
    """Set controller to use 4 directions to trigger shortcuts."""

    def __init__(self, stick_vector, set_deadzone, ui, fourway_key_list):
        """
        Initialize trigger joystick handler.

        You'll find e.KEY_? here at /usr/include/linux/input-event-codes.h usually.

        Args:
            stick_vector: Gives angle and deadzone information.
            set_deadzone: The function to call when on_deadzone.
            ui: The uinput device.
            fourway_key_list: list of 4 items, up, down, left and right.

        """
        self.stick_vector = stick_vector
        self.on_deadzone = set_deadzone
        self.ui = ui
        self.clicked_state_reference = None
        self.fourway_active_index = None
        self.fourway_key_list = fourway_key_list

    def set_clicked_state_reference(self, clicked):
        """Set reference to clicked state."""
        self.clicked_state_reference = clicked

    def handle_state(self):
        """Handle the states for the joystick trigger."""
        if self.stick_vector.in_deadzone:
            self.on_deadzone()
            if self.fourway_active_index is not None:
                keycode = self.fourway_key_list[self.fourway_active_index]
                if keycode is not None:
                    self.ui.write(e.EV_KEY, self.fourway_key_list[self.fourway_active_index], 0)
                    self.ui.syn()
                self.fourway_active_index = None
                self.clicked_state_reference[0] = False
            return

        if self.fourway_active_index is not None:
            return

        angle = self.stick_vector.angle
        # Up
        if 135 > angle > 45:
            self._fire(0)
        # Right
        elif -45 < angle < 45:
            self._fire(3)
        # Left
        elif (-180 < angle < -135) or (180 > angle > 135):
            self._fire(2)
        # Down
        elif -135 < angle < -45:
            self._fire(1)

    def _fire(self, index):
        self.fourway_active_index = index
        keycode = self.fourway_key_list[self.fourway_active_index]
        if keycode is not None:
            self.ui.write(e.EV_KEY, keycode, 1)
            self.ui.syn()


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
            "right_stick_clicked": [False],
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

        self.fourway_trigger_joystick_handle = FourwayTriggerJoystickHandle(
            self.stick_vector,
            self.set_deadzone,
            self.ui,
            [e.KEY_1, e.KEY_2, e.KEY_3, e.KEY_4],
        )

    def set_deadzone(self):
        """Deactivate all arrow keys."""
        if self.stick_vector.in_deadzone_first_time:
            self.stick_vector.in_deadzone_first_time = False
        self.direction.release_all()

    def set_look_state_reference(self, look):
        """Set reference to look state."""
        self.state["look"] = look

    def set_clicked_state_reference(self, clicked):
        """Set reference to clicked state."""
        self.state["clicked"] = clicked

    def set_right_stick_clicked_state_reference(self, right_stick_clicked):
        """Set reference to right stick clicked state."""
        self.state["right_stick_clicked"] = right_stick_clicked
        self.fourway_trigger_joystick_handle.set_clicked_state_reference(right_stick_clicked)

    def handle_event(self, event):
        """Handle the events."""
        self.stick.handle_event(event)
        self.stick_vector.process(self.stick.current_x, self.stick.current_y)
        if self.state["right_stick_clicked"][0]:
            self.fourway_trigger_joystick_handle.handle_state()
        elif self.state["classic_overlap"] or self.state["look"][0]:
            self.sector_size_joystick_handle.handle_state()
        else:
            self.default_joystick_handle.handle_state()


class CircleTriggerJoystickHandle:
    """Set controller to use half circle to trigger 2 shortcuts."""

    def __init__(self, stick_vector, set_deadzone, ui, cw_key, ccw_key):
        """
        Initialize trigger joystick handler.

        You'll find e.KEY_? here at /usr/include/linux/input-event-codes.h usually.
        Args:
            stick_vector: Gives angle and deadzone information.
            set_deadzone: The function to call when on_deadzone.
            ui: The uinput device.
            cw_key: clockwise trigger keycode.
            ccw_key: counter clockwise trigger keycode
        """
        self.stick_vector = stick_vector
        self.on_deadzone = set_deadzone
        self.ui = ui
        self.prev_angle = None
        self.cumulative_rotation = 0.0
        self.active_key = None
        self.cw_key = cw_key
        self.ccw_key = ccw_key

    def handle_state(self):
        """Handle the states for the circle trigger."""
        if self.stick_vector.in_deadzone:
            self.on_deadzone()
            if self.active_key is not None:
                self.ui.write(e.EV_KEY, self.active_key, 0)
                self.ui.syn()
                self.prev_angle = None
                self.cumulative_rotation = 0.0
                self.active_key = None
            return

        angle = self.stick_vector.angle
        if self.prev_angle is None:
            self.prev_angle = angle
            return

        delta = angle - self.prev_angle
        if delta > 180:
            delta -= 360
        elif delta <= -180:
            delta += 360
        self.cumulative_rotation += delta
        self.prev_angle = angle

        if self.active_key is None:
            if self.cumulative_rotation >= 180:
                self.ui.write(e.EV_KEY, self.cw_key, 1)
                self.active_key = self.cw_key
            elif self.cumulative_rotation <= -180:
                self.ui.write(e.EV_KEY, self.ccw_key, 1)
                self.active_key = self.ccw_key
            self.ui.syn()


class RightStick:
    """Handle right analog stick input."""

    def __init__(self, ui, device, tr=3):
        """
        Initialize right joystick handler.

        Args:
            ui: The uinput device.
            device: The evdev controller input device.
            tr: Tomb Raider input map type.

        """
        if tr not in (1, 2, 3):
            print("Object error: Not a good input map type")
            sys.exit(1)

        self.ui = ui
        self.stick = Stick(device, e.ABS_RX, e.ABS_RY)
        self.stick_vector = StickVector()

        self.state = {
            "clicked": [False],
        }

        ccw_key = None
        cw_key = None
        fourway_key_list = None
        self.circle_trigger_joystick_handle = None
        self.fourway_trigger_joystick_handle = None

        if tr == 3:
            ccw_key = e.KEY_9
            cw_key = e.KEY_0
            fourway_key_list = [e.KEY_5, e.KEY_6, e.KEY_7, e.KEY_8]

        elif tr == 2:
            ccw_key = e.KEY_8
            cw_key = e.KEY_9
            fourway_key_list = [e.KEY_5, e.KEY_6, e.KEY_7, None]

        elif tr == 1:
            ccw_key = e.KEY_F5
            cw_key = e.KEY_F6

        self.circle_trigger_joystick_handle = CircleTriggerJoystickHandle(
            self.stick_vector,
            self.set_deadzone,
            self.ui,
            ccw_key,
            cw_key,
        )

        if tr != 1:
            self.fourway_trigger_joystick_handle = FourwayTriggerJoystickHandle(
                self.stick_vector,
                self.set_deadzone,
                self.ui,
                fourway_key_list,
            )

    def set_deadzone(self):
        """Deactivate all arrow keys."""
        if self.stick_vector.in_deadzone_first_time:
            self.stick_vector.in_deadzone_first_time = False

    def set_clicked_state_reference(self, clicked):
        """Set reference to clicked state."""
        self.state["clicked"] = clicked
        if self.fourway_trigger_joystick_handle is not None:
            self.fourway_trigger_joystick_handle.set_clicked_state_reference(clicked)

    def handle_event(self, event):
        """Handle the events."""
        self.stick.handle_event(event)
        self.stick_vector.process(self.stick.current_x, self.stick.current_y)
        if self.state["clicked"][0] and self.fourway_trigger_joystick_handle is not None:
            self.fourway_trigger_joystick_handle.handle_state()
        else:
            self.circle_trigger_joystick_handle.handle_state()


class Trigger:
    """Handles analog trigger input."""

    def __init__(self, ui, device, event_in, keyout, shortcut_keyout=None):
        """
        Initialize the Trigger handler.

        Args:
            ui: The uinput virtual device.
            device: The evdev controller input device.
            event_in: The event code for the analog trigger (e.g., ABS_Z).
            keyout: The key code to output when the trigger is pressed.
            shortcut_keyout: Second special shortcut key code to output.
        """
        self.range = 1
        self.threshold = 0.80

        if event_in == e.ABS_Z:
            abs_z = device.absinfo(e.ABS_Z)
            self.range = abs_z.max - abs_z.min
            self.threshold = 0.90

        self.ui = ui
        self.event_in = event_in
        self.keyout = keyout
        if shortcut_keyout is not None:
            self.shortcut_keyout = shortcut_keyout
        self.shortcut_state = None
        self.pressed = False

    def set_shortcut_state_reference(self, shortcut_state):
        """Set reference to look state."""
        self.shortcut_state = shortcut_state

    def handle_event(self, event):
        """
        Handle analog trigger events and emit key presses accordingly.

        Args:
            event: An evdev input event.
        """
        if event.code == self.event_in:
            value = event.value / self.range
            if value > self.threshold and not self.pressed:
                if self.shortcut_state is not None and self.shortcut_state[0] is True:
                    self.ui.write(e.EV_KEY, self.shortcut_keyout, 1)
                else:
                    self.ui.write(e.EV_KEY, self.keyout, 1)
                self.pressed = True
            elif value <= self.threshold and self.pressed:
                if self.shortcut_state is not None and self.shortcut_state[0] is True:
                    self.ui.write(e.EV_KEY, self.shortcut_keyout, 0)
                    self.shortcut_state[0] = False
                else:
                    self.ui.write(e.EV_KEY, self.keyout, 0)
                self.pressed = False
            self.ui.syn()


class Key:
    """Handles key trigger input."""

    def __init__(self, ui, event_in, keyout, shortcut_keyout=None):
        """
        Initialize the Key handler.

        Args:
            ui: The uinput device.
            event_in: The button event code (e.g., BTN_SOUTH).
            keyout: The key code to output (e.g., KEY_LEFTCTRL).
            shortcut_keyout: Second special shortcut key code to output.
        """
        self.ui = ui
        self.event_in = event_in
        self.keyout = keyout
        if shortcut_keyout is not None:
            self.shortcut_keyout = shortcut_keyout
        self.shortcut_state = None
        self.look = None
        self.thumb_clicked = None
        self.thumb_clicked_last_time = time.monotonic()

    def set_shortcut_state_reference(self, shortcut_state):
        """Set reference to look state."""
        self.shortcut_state = shortcut_state

    def set_look_reference(self, look):
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
        if event.code == self.event_in:
            if self.look is not None and self.event_in is e.BTN_TL:
                if event.value == 1:
                    self.look[0] = True
                else:
                    self.look[0] = False
                self.ui.write(e.EV_KEY, self.keyout, event.value)
                self.ui.syn()
            elif self.thumb_clicked is not None \
                    and self.event_in in (e.BTN_THUMBL, e.BTN_THUMBR):
                if event.value == 1:
                    now = time.monotonic()
                    if now - self.thumb_clicked_last_time < 0.6:
                        self.thumb_clicked[0] = not self.thumb_clicked[0]
                    else:
                        self.thumb_clicked_last_time = now
            elif self.shortcut_state is not None \
                    and self.shortcut_state[0] is True:
                self.ui.write(e.EV_KEY, self.shortcut_keyout, event.value)
                self.ui.syn()
                if event.value == 0:
                    self.shortcut_state[0] = False
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
        self.right_stick_clicked = [False]

    def add_dpad(self):
        """Add D-pad handler."""
        self.abs_handlers.append(Dpad(self.ui))

    def add_stick(self, classic=False, tr=3):
        """Add analog stick handler."""
        left_stick = LeftStick(self.ui, self.device, classic)
        left_stick.set_look_state_reference(self.look)
        left_stick.set_clicked_state_reference(self.left_stick_clicked)
        left_stick.set_right_stick_clicked_state_reference(self.right_stick_clicked)
        self.abs_handlers.append(left_stick)
        right_stick = RightStick(self.ui, self.device, tr)
        right_stick.set_clicked_state_reference(self.right_stick_clicked)
        self.abs_handlers.append(right_stick)

    def add_trigger(self, event, keyout, shortcut_keyout=None):
        """
        Add analog trigger handler.

        Usually called R2/L2 or ZR/ZL. They have a range
        from 0 to 255 and are not buttons.

        Args:
            event: The analog axis code (e.g., ABS_Z).
            keyout: The corresponding keyboard output code.
            shortcut_keyout: The secondary keyboard key code to emit.
        """
        if shortcut_keyout is not None:
            trigger = Trigger(self.ui, self.device, event, keyout, shortcut_keyout)
            trigger.set_shortcut_state_reference(self.right_stick_clicked)
            self.abs_handlers.append(trigger)
        else:
            trigger = Trigger(self.ui, self.device, event, keyout)
            self.abs_handlers.append(trigger)

    def add_key(self, event, keyout=None, shortcut_keyout=None):
        """
        Add digital button-to-key handler.

        Args:
            event: The gamepad button code.
            keyout: The keyboard key code to emit.
            shortcut_keyout: The secondary keyboard key code to emit.
        """
        if (event == e.BTN_THUMBL) and (keyout is None):
            key = Key(self.ui, event, keyout)
            key.set_thumb_click(self.left_stick_clicked)
            self.key_handlers.append(key)
        elif (event == e.BTN_THUMBR) and (keyout is None):
            key = Key(self.ui, event, keyout)
            key.set_thumb_click(self.right_stick_clicked)
            self.key_handlers.append(key)
        elif keyout == e.KEY_KP0:
            key = Key(self.ui, event, keyout)
            key.set_look_reference(self.look)
            self.key_handlers.append(key)
        elif keyout is not None and shortcut_keyout is not None:
            key = Key(self.ui, event, keyout, shortcut_keyout)
            key.set_shortcut_state_reference(self.right_stick_clicked)
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


def _ps4_3to5(overlap):
    preset = Preset()
    controller = preset.get_controller()
    controller.add_dpad()
    controller.add_stick(classic=overlap, tr=3)
    controller.add_key(e.BTN_THUMBL)
    controller.add_key(e.BTN_THUMBR)
    controller.add_trigger(e.ABS_Z, e.KEY_DOT)
    controller.add_trigger(e.ABS_RZ, e.KEY_SLASH)
    controller.add_key(e.BTN_EAST, e.KEY_END)
    controller.add_key(e.BTN_SOUTH, e.KEY_LEFTCTRL)
    controller.add_key(e.BTN_WEST, e.KEY_LEFTALT)
    controller.add_key(e.BTN_NORTH, e.KEY_SPACE)
    controller.add_key(e.BTN_TL, e.KEY_KP0, e.KEY_J)
    controller.add_key(e.BTN_TR, e.KEY_LEFTSHIFT, e.KEY_P)
    controller.add_key(e.BTN_SELECT, e.KEY_COMMA, e.KEY_F5)
    controller.add_key(e.BTN_START, e.KEY_ESC, e.KEY_F6)
    preset.read_loop()


def _ps4_2(overlap):
    preset = Preset()
    controller = preset.get_controller()
    controller.add_dpad()
    controller.add_stick(classic=overlap, tr=2)
    controller.add_key(e.BTN_THUMBL)
    controller.add_key(e.BTN_THUMBR)
    controller.add_trigger(e.ABS_Z, e.KEY_DELETE)
    controller.add_trigger(e.ABS_RZ, e.KEY_PAGEDOWN)
    controller.add_key(e.BTN_EAST, e.KEY_END)
    controller.add_key(e.BTN_SOUTH, e.KEY_LEFTCTRL)
    controller.add_key(e.BTN_WEST, e.KEY_LEFTALT)
    controller.add_key(e.BTN_NORTH, e.KEY_SPACE)
    controller.add_key(e.BTN_TL, e.KEY_KP0)
    controller.add_key(e.BTN_TR, e.KEY_LEFTSHIFT)
    controller.add_key(e.BTN_SELECT, e.KEY_COMMA, e.KEY_F5)
    controller.add_key(e.BTN_START, e.KEY_ESC, e.KEY_F6)
    preset.read_loop()


def _ps4_1(overlap):
    preset = Preset()
    controller = preset.get_controller()
    controller.add_dpad()
    controller.add_stick(classic=overlap, tr=1)
    controller.add_key(e.BTN_THUMBL)
    controller.add_key(e.BTN_THUMBR)
    controller.add_trigger(e.ABS_Z, e.KEY_DELETE)
    controller.add_trigger(e.ABS_RZ, e.KEY_PAGEDOWN)
    controller.add_key(e.BTN_EAST, e.KEY_END)
    controller.add_key(e.BTN_SOUTH, e.KEY_LEFTCTRL)
    controller.add_key(e.BTN_WEST, e.KEY_LEFTALT)
    controller.add_key(e.BTN_NORTH, e.KEY_SPACE)
    controller.add_key(e.BTN_TL, e.KEY_KP0, e.KEY_J)
    controller.add_key(e.BTN_TR, e.KEY_LEFTSHIFT, e.KEY_P)
    controller.add_key(e.BTN_START, e.KEY_ESC)
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
        choices=[
            "ps4-3to4",
            "ps4-3to4-overlap",
            "ps4-2",
            "ps4-2-overlap",
            "ps4-1",
            "ps4-1-overlap",
            "only-ps4-share",
            "only-left-stick",
        ],
        help="Select input mapping mode",
    )

    parser.add_argument('--version', action='version', version='%(prog)s 1.0.0')

    args = parser.parse_args()

    if args.mode == "ps4-3to4":
        _ps4_3to5(False)
    elif args.mode == "ps4-3to4-overlap":
        _ps4_3to5(True)
    if args.mode == "ps4-2":
        _ps4_2(False)
    elif args.mode == "ps4-2-overlap":
        _ps4_2(True)
    if args.mode == "ps4-1":
        _ps4_1(False)
    elif args.mode == "ps4-1-overlap":
        _ps4_1(True)
    elif args.mode == "only-ps4-share":
        _only_ps4_share()
    elif args.mode == "only-left-stick":
        _only_left_stick()


if __name__ == "__main__":
    _main()
