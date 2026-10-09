"""Trigger mapper objects."""
from __future__ import annotations

from dataclasses import dataclass

from evdev import ecodes as e

from common import Reference, get_ecode, get_key


class Trigger:
    """Handles analog trigger input."""

    @dataclass
    class _Keys:
        input: int = 0
        output: int | None = None
        shortcut: int | None = None

    @dataclass
    class _States:
        shortcut: Reference
        look: Reference
        this_is_look: bool = False
        active_key: int | None = None
        active_is_sc: bool = False

    def __init__(self, ui, device, config: dict):
        """
        Initialize the Trigger handler.

        Usually called R2/L2 or ZR/ZL. They have a range
        from 0 to 255 and are not buttons.

        {
            "type": "trigger",
            "mapping": {
                "input_key": "ABS_Z",
                "output_key": "KEY_P",
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
        self.ui = ui
        self.keys = self._Keys()
        self.state = self._States(Reference(False), Reference(False))

        self.pressed = False
        self.threshold = 0.90

        mapping_dict = get_key(config, "mapping", "mapping")

        input_key_str = get_key(mapping_dict, "input_key", "mapping", expected_type=str)
        self.keys.input = get_ecode(input_key_str, "mapping")
        absinfo = device.absinfo(self.keys.input)
        self.min = absinfo.min
        self.range = absinfo.max - absinfo.min

        if self.range < 1:
            raise ValueError(
                f"Invalid axis range for '{input_key_str}' (ecode {self.keys.input}): "
                f"max - min = {self.range} (min={absinfo.min}, max={absinfo.max}), "
                "expected >= 1. Range is used as a divisor."
            )

        output_key_str = mapping_dict.get("output_key")
        if output_key_str is not None:
            self.keys.output = get_ecode(output_key_str, "mapping")

        shortcut_key_str = mapping_dict.get("shortcut_key")
        if shortcut_key_str is not None:
            self.keys.shortcut = get_ecode(shortcut_key_str, "mapping")

        threshold = mapping_dict.get("threshold")
        if threshold is not None:
            threshold = get_key(mapping_dict, "threshold", "mapping", expected_type=float)
            if not 0.01 <= threshold <= 0.99:
                raise ValueError(f"threshold must be between 0.01 and 0.99, got {threshold}")
            self.threshold = threshold

    def set_shortcut_state_reference(self, shortcut_state: Reference):
        """Set reference to shortcut state."""
        self.state.shortcut = shortcut_state

    def set_look_reference(self, look: Reference):
        """Set reference to look state. Requires a key-mode handler."""
        if self.keys.output is None:
            raise ValueError(
                f"set_look_reference requires output_key (input_key={self.keys.input}); "
                "thumb handlers do not support look"
            )
        self.state.look = look
        self.state.this_is_look = True

    def handle_event(self, event):
        """
        Handle analog trigger events and emit key presses accordingly.

        Args:
            event: An evdev input event.
        """
        if event.type != e.EV_ABS or event.code != self.keys.input:
            return
        is_down = ((event.value - self.min) / self.range) > self.threshold
        if is_down == self.pressed:
            return

        if is_down:
            self.state.active_is_sc = (
                self.state.shortcut.get() is True and self.keys.shortcut is not None
            )
            self.state.active_key = (
                self.keys.shortcut if self.state.active_is_sc else self.keys.output
            )

        if self.state.active_key is not None:
            self.ui.write(e.EV_KEY, self.state.active_key, int(is_down))
            self.ui.syn()
        if not is_down and self.state.active_is_sc:
            self.state.shortcut.set(False)
        if self.state.this_is_look:
            self.state.look.set(is_down)
        self.pressed = is_down
