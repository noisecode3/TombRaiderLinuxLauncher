"""Key mapper objects."""
from __future__ import annotations

import time
from dataclasses import dataclass

import evdev
from evdev import ecodes as e

from common import Reference, get_ecode, get_key


class Key:
    """Handles key trigger input."""

    input_key: int
    output_key: int | None = None
    shortcut_key: int | None = None
    thumb_key: int | None = None

    @dataclass
    class _States:
        shortcut: Reference
        look: Reference
        thumb_clicked: Reference
        this_is_look: bool = False
        thumb_clicked_last_time: float = float("-inf")

    def __init__(self, ui: evdev.UInput, config: dict):
        """
        Initialize the Key handler.

        Args:
            ui: The uinput device.
            event_in: The button event code (e.g., BTN_SOUTH).
            keyout: The key code to output (e.g., KEY_LEFTCTRL).
            shortcut_key: Second special shortcut key code to output.

        Config example:
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_SOUTH",
                    "output_key": "KEY_LEFTCTRL",
                    "shortcut_key": None
                }
            },

        """
        self.ui = ui
        mapping_dict = get_key(config, "mapping", "mapping")

        input_key_str = get_key(mapping_dict, "input_key", "mapping", expected_type=str)
        self.input_key = get_ecode(input_key_str, "mapping")

        output_key_str = mapping_dict.get("output_key")
        if output_key_str is not None:
            self.output_key = get_ecode(output_key_str, "mapping")
            self.handle_event = self._handle_key_event
        else:
            self.handle_event = self._handle_thumb_event

        shortcut_key_str = mapping_dict.get("shortcut_key")
        if shortcut_key_str is not None:
            self.shortcut_key = get_ecode(shortcut_key_str, "mapping")

        self.state = self._States(Reference(False), Reference(False), Reference(False))

    def set_shortcut_state_reference(self, shortcut_state: Reference):
        """Set reference to look state."""
        self.state.shortcut = shortcut_state

    def set_look_reference(self, look: Reference):
        """Set reference to look state. Requires a key-mode handler."""
        if self.output_key is None:
            raise ValueError(
                f"set_look_reference requires output_key (input_key={self.input_key}); "
                "thumb handlers do not support look"
            )
        self.state.look = look
        self.state.this_is_look = True

    def set_thumb_click(self, thumb_clicked: Reference):
        """Set reference to thumbl clicked state."""
        self.state.thumb_clicked = thumb_clicked

    def _handle_key_event(self, event):
        if event.code == self.input_key:
            key = self.output_key

            if self.state.this_is_look:
                self.state.look.set(event.value == 1)
            elif self.state.shortcut.get() and self.shortcut_key is not None:
                key = self.shortcut_key
                if event.value == 0:
                    self.state.shortcut.set(False)

            self.ui.write(e.EV_KEY, key, event.value)
            self.ui.syn()

    def _handle_thumb_event(self, event):
        if event.code == self.input_key and event.value == 1:
            now = time.monotonic()
            if now - self.state.thumb_clicked_last_time < 0.6:
                self.state.thumb_clicked.set(not self.state.thumb_clicked.get())
            else:
                self.state.thumb_clicked_last_time = now
