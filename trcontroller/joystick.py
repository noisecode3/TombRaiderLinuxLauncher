"""Joystick mapper objects."""
import math
import sys
import time
from dataclasses import dataclass

import evdev
from evdev import ecodes as e

from common import Reference, get_ecode


class ArrowButton:
    """Arrow button state and key press/release handling."""

    def __init__(self, ui, direction, keycode):
        """Initialize an arrow button for the given direction."""
        self.ui = ui
        self.key = get_ecode(keycode, "sectors.direction")
        if direction not in ("up", "right", "left", "down"):
            print("Object Error: Unknown Direction")
            sys.exit(1)
        self.direction = direction
        self.pressed = False

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

    def __init__(self, ui, directions: dict):
        """Initialize an arrow button for the given direction."""
        self.up = ArrowButton(ui, "up", directions["up"]["output_key"])
        self.down = ArrowButton(ui, "down", directions["down"]["output_key"])
        self.left = ArrowButton(ui, "left", directions["left"]["output_key"])
        self.right = ArrowButton(ui, "right", directions["right"]["output_key"])

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

    def __init__(self, device: evdev.InputDevice, abs_x, abs_y):
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
        if event.type != evdev.ecodes.EV_ABS:
            return
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
        elif radius < (self.threshold - self.hysteresis) and not self.in_deadzone:
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

    def __init__(self, direction, stick_vector, on_deadzone, sector_size):
        """Set sector angle boundaries and callbacks for direction detection."""
        self.direction = direction
        self.stick_vector = stick_vector
        self.sector_size_up = sector_size["up"]
        self.sector_size_right = sector_size["right"]
        self.sector_size_down = sector_size["down"]
        self.sector_size_left = sector_size["left"]
        self.on_deadzone = on_deadzone

    def handle_state(self):
        """Handle the states for the arrow keys."""
        if self.stick_vector.in_deadzone:
            self.on_deadzone()
            return

        # Up
        if 90-self.sector_size_up/2 < self.stick_vector.angle < 90+self.sector_size_up/2:
            self.direction.up.set_pressed()
        else:
            self.direction.up.set_release()

        # Right
        if -self.sector_size_right/2 < self.stick_vector.angle < self.sector_size_right/2:
            self.direction.right.set_pressed()
        else:
            self.direction.right.set_release()

        # Left
        if self.stick_vector.angle < -180+self.sector_size_left/2 or \
                self.stick_vector.angle > 180-self.sector_size_left/2:
            self.direction.left.set_pressed()
        else:
            self.direction.left.set_release()

        # Down
        if -90-self.sector_size_down/2 < self.stick_vector.angle < -90+self.sector_size_down/2:
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

    def set_clicked_state_reference(self, clicked: Reference):
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
                self.clicked_state_reference.set(False)
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


class Joystick:
    """Handle analog stick input."""

    def __init__(self, ui: evdev.UInput, device: evdev.InputDevice, config: dict):
        """
        Initialize left joystick handler.

        {
            "type": "joystick",
            "axes": {
                "input_x": "ABS_X",
                "input_y": "ABS_Y"
            },
            "sectors": {
                "use_sector_size_for_movement": False,
                "use_sector_size_for_look_key": "KEY_KP0",
                "up": {
                    "sub_right_up": 60.0,
                    "sub_left_up": 120.0,
                    "size": 138.0,
                    "output_key": "KEY_UP"
                },
                "right": {
                    "right_up": 20.0,
                    "right_down": -20.0,
                    "size": 138.0,
                    "output_key": "KEY_RIGHT"
                },
                "down": {
                    "sub_right_down": -60.0,
                    "sub_left_down": -120.0,
                    "size": 138.0,
                    "output_key": "KEY_DOWN"
                },
                "left": {
                    "left_up": 160.0,
                    "left_down": -160.0,
                    "size": 138.0,
                    "output_key": "KEY_LEFT"
                }
            },
            "circle_trigger_shortcuts": None,
            "fourway_trigger_shortcuts":
                left_joystick_fourway_trigger_shortcuts,
            "double_click_key": {
                "input_key": "BTN_THUMBL",
                "toggle_shortcut_mode": False,
                "toggle_hold_output_key": False,
                "output_key": None
            }
        },
        {
            "type": "joystick",
            "axes": {
                "input_x": "ABS_RX",
                "input_y": "ABS_RY"
            },
            "sectors": None,
            "circle_trigger_shortcuts": right_circle_trigger_shortcuts,
            "fourway_trigger_shortcuts":
                right_joystick_fourway_trigger_shortcuts,
            "double_click_key": {
                "input_key": "BTN_THUMBR",
                "toggle_shortcut_mode": toggle_shortcut_mode,
                "toggle_hold_output_key": False,
                "output_key": None
            }
        }

        Args:
            ui: The uinput device.
            device: The evdev controller input device.
            classic_overlap: if set there is only a simple 8-way stateless input

        """
        self.ui = ui
        self.stick = Stick(
            device,
            get_ecode(config["axes"]["input_x"], "axes.input_x"),
            get_ecode(config["axes"]["input_y"], "axes.input_y"),
        )
        self.direction = None
        self.stick_vector = StickVector()

        self.default_joystick_handle = None
        self.sector_size_joystick_handle = None
        self.circle_trigger_joystick_handle = config["circle_trigger_shortcuts"]
        self.use_sector_size_for_look_key = None

        sectors = config["sectors"]
        self.classic_overlap = False

        if sectors is not None:
            self.direction = DirectionalButtons(ui, sectors)
            use_sector_size_for_look_key = sectors["use_sector_size_for_look_key"]
            self.use_sector_size_for_look_key = get_ecode(use_sector_size_for_look_key, "sectors")
            classic_overlap = sectors["use_sector_size_for_movement"]
            if classic_overlap is None:
                classic_overlap = False
            self.classic_overlap = classic_overlap

            attr_sources = {
                "sub_right_up": ("up", "sub_right_up"),
                "sub_left_up": ("up", "sub_left_up"),
                "right_up": ("right", "right_up"),
                "right_down": ("right", "right_down"),
                "sub_right_down": ("down", "sub_right_down"),
                "sub_left_down": ("down", "sub_left_down"),
                "left_up": ("left", "left_up"),
                "left_down": ("left", "left_down"),
            }

            sector_points_default = SectorPoints(**{
                attr: sectors[sector][key]
                for attr, (sector, key) in attr_sources.items()
                if sectors[sector][key] is not None
            })

            self.default_joystick_handle = DefaultJoystickHandle(
                self.direction,
                self.stick_vector,
                self.set_deadzone,
                sector_points_default,
            )

            sector_size_input = {"up": 138, "right": 138, "down": 138, "left": 138}
            point_size = sectors["up"]["size"]
            if point_size is not None:
                sector_size_input["up"] = point_size

            point_size = sectors["right"]["size"]
            if point_size is not None:
                sector_size_input["right"] = point_size

            point_size = sectors["down"]["size"]
            if point_size is not None:
                sector_size_input["down"] = point_size

            point_size = sectors["left"]["size"]
            if point_size is not None:
                sector_size_input["left"] = point_size

            self.sector_size_joystick_handle = SectorSizeJoystickHandle(
                self.direction,
                self.stick_vector,
                self.set_deadzone,
                sector_size_input,
            )

        elif self.circle_trigger_joystick_handle is not None:
            cw_key_str = self.circle_trigger_joystick_handle["cw_key"]
            ccw_key_str = self.circle_trigger_joystick_handle["ccw_key"]
            cw_key = get_ecode(cw_key_str, "circle_trigger_shortcuts")
            ccw_key = get_ecode(ccw_key_str, "circle_trigger_shortcuts")
            if (cw_key is not None) or (ccw_key is not None):
                self.circle_trigger_joystick_handle = CircleTriggerJoystickHandle(
                    self.stick_vector,
                    self.set_deadzone,
                    self.ui,
                    cw_key,
                    ccw_key,
                )

        self.fourway_trigger_joystick_handle = None
        fourway_trigger_shortcuts = config["fourway_trigger_shortcuts"]
        if fourway_trigger_shortcuts is not None:

            self.fourway_trigger_joystick_handle = FourwayTriggerJoystickHandle(
                self.stick_vector,
                self.set_deadzone,
                self.ui,
                [
                    get_ecode(fourway_trigger_shortcuts["up"], "fourway_trigger_shortcuts"),
                    get_ecode(fourway_trigger_shortcuts["down"], "fourway_trigger_shortcuts"),
                    get_ecode(fourway_trigger_shortcuts["left"], "fourway_trigger_shortcuts"),
                    get_ecode(fourway_trigger_shortcuts["right"], "fourway_trigger_shortcuts")
                ],
            )

        self.double_click_key = config["double_click_key"]
        if self.double_click_key is not None:
            self.double_click_key["input_key"] = \
                    get_ecode(self.double_click_key["input_key"], "double_click_key")
            """
                "double_click_key": {
                    "input_key": "BTN_THUMBR",
                    "toggle_shortcut_mode": True,
                    "toggle_hold_output_key": False,
                    "output_key": None
                }
            """

        self.state: dict[str, Reference] = {
            "clicked": Reference(False),
            "clicked_shortcut": Reference(False),
            "clicked_look": Reference(False),
        }

    def set_deadzone(self):
        """Deactivate all arrow keys."""
        if self.stick_vector.in_deadzone_first_time:
            self.stick_vector.in_deadzone_first_time = False
        if isinstance(self.direction, DirectionalButtons):
            self.direction.release_all()

    def set_look_state_reference(self, look: Reference):
        """Set reference to look state."""
        if isinstance(look, Reference):
            ref = look.get()
            if isinstance(ref, bool):
                self.state["clicked_look"] = look
            else:
                raise TypeError("look is not Reference(bool)")
        else:
            raise TypeError("look is not Reference")

    def set_clicked_state_reference(self, clicked: Reference):
        """Set reference to clicked state."""
        self.state["clicked"] = clicked

    def set_clicked_shortcut_state_reference(self, clicked_shortcut: Reference):
        """Set reference to shortcut clicked state."""
        self.state["clicked_shortcut"] = clicked_shortcut
        if isinstance(self.fourway_trigger_joystick_handle, FourwayTriggerJoystickHandle):
            self.fourway_trigger_joystick_handle.set_clicked_state_reference(clicked_shortcut)

    def handle_event(self, event):
        """Handle the events."""
        self.stick.handle_event(event)
        self.stick_vector.process(self.stick.current_x, self.stick.current_y)
        if self.state["clicked_shortcut"].get():
            self.fourway_trigger_joystick_handle.handle_state()
        elif self.classic_overlap or self.state["clicked_look"].get():
            self.sector_size_joystick_handle.handle_state()
        elif self.default_joystick_handle is not None:
            self.default_joystick_handle.handle_state()
        elif self.circle_trigger_joystick_handle is not None:
            self.circle_trigger_joystick_handle.handle_state()
