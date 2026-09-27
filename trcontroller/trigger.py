"""Trigger mapper objects."""
from evdev import ecodes as e

from common import Reference, get_ecode, get_key


class Trigger:
    """Handles analog trigger input."""

    def __init__(self, ui, device, config: dict):
        """
        Initialize the Trigger handler.

        Usually called R2/L2 or ZR/ZL. They have a range
        from 0 to 255 and are not buttons.

        {
            "type": "trigger",
            "mapping": {
                "input_key": "ABS_Z",
                "output_key": KEY_P,
                "shortcut_key": None,
                "threshold": 0.90
            }
        }

        Args:
            ui: The uinput virtual device.
            device: The evdev controller input device.
            config:
                input_key: The event code for the analog trigger (e.g., ABS_Z).
                output_key: The key code to output when the trigger is pressed.
                shortcut_key: Second special shortcut key code to output.
                threshold: A number from 0.01 to 0.99
        """
        self.range = 1
        self.threshold = 0.90

        mapping = config["mapping"]
        if mapping is not None:
            input_key = mapping["input_key"]
            if input_key is not None:
                self.event_in = get_ecode(input_key, "components.mapping")
                absinfo = device.absinfo(self.event_in)
                self.range = absinfo.max - absinfo.min
            output_key = mapping["output_key"]
            if output_key is not None:
                self.keyout = get_ecode(output_key, "components.mapping")
            shortcut_key = mapping["shortcut_key"]
            if shortcut_key is not None:
                self.shortcut_keyout = get_ecode(shortcut_key, "components.mapping")

        self.ui = ui
        self.shortcut_state = Reference(False)
        self.pressed = False

    def set_shortcut_state_reference(self, shortcut_state: Reference):
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
                if self.shortcut_state.get() is True and self.shortcut_keyout is not None:
                    self.ui.write(e.EV_KEY, self.shortcut_keyout, 1)
                else:
                    self.ui.write(e.EV_KEY, self.keyout, 1)
                self.pressed = True
            elif value <= self.threshold and self.pressed:
                if self.shortcut_state.get() is True and self.shortcut_keyout is not None:
                    self.ui.write(e.EV_KEY, self.shortcut_keyout, 0)
                    self.shortcut_state.set(False)
                else:
                    self.ui.write(e.EV_KEY, self.keyout, 0)
                self.pressed = False
            self.ui.syn()
