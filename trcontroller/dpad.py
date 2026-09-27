"""Dpad mapper objects."""
from evdev import ecodes as e

from common import get_ecode, get_key


class Dpad:
    """Handles D-pad input."""

    class DpadAxis:
        """Handles D-pad output events keys on one axis."""

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

    def __init__(self, ui, config: dict):
        """
        Initialize D-Pad handler.

        config example:
        {
            "type": "dpad",
            "axes": {
                "input_x": "ABS_HAT0X",
                "x_keys": {
                    "neg_key_value": "KEY_LEFT",
                    "pos_key_value": "KEY_RIGHT"
                },
                "input_y": "ABS_HAT0Y",
                "y_keys": {
                    "neg_key_value": "KEY_UP",
                    "pos_key_value": "KEY_DOWN"
                }
            }
        }

        Args:
            ui: The uinput device.
            config: configuration for the Dpad.
        """
        self.ui = ui
        axes = config["axes"]
        x_keys = axes["x_keys"]

        neg_x = get_key(x_keys, "neg_key_value", path="axes.x_keys", expected_type=str)
        pos_x = get_key(x_keys, "pos_key_value", path="axes.x_keys", expected_type=str)

        self.output_x = self.DpadAxis(
            get_ecode(neg_x, "axes.x_keys.neg_key_value"),
            get_ecode(pos_x, "axes.x_keys.pos_key_value"),
        )

        y_keys = axes["y_keys"]

        neg_y = get_key(y_keys, "neg_key_value", path="axes.y_keys", expected_type=str)
        pos_y = get_key(y_keys, "pos_key_value", path="axes.y_keys", expected_type=str)

        self.output_y = self.DpadAxis(
            get_ecode(neg_y, "axes.y_keys.neg_key_value"),
            get_ecode(pos_y, "axes.y_keys.pos_key_value"),
        )

        self.input_x = get_key(axes, "input_x", path="axes", expected_type=str)
        self.input_x = get_ecode(self.input_x, "axes.input_x")
        self.input_y = get_key(axes, "input_y", path="axes", expected_type=str)
        self.input_y = get_ecode(self.input_y, "axes.input_y")

    def handle_event(self, event):
        """Handle x and y separately."""
        if event.code == self.input_x:
            self.output_x.handle(self.ui, event.value)
        elif event.code == self.input_y:
            self.output_y.handle(self.ui, event.value)
