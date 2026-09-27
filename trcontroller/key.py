"""Key mapper objects."""
import time

from evdev import ecodes as e

from common import Reference, get_ecode, get_key


class Key:
    """Handles key trigger input."""

    def __init__(self, ui, config: dict):
        """
        Initialize the Key handler.

        Args:
            ui: The uinput device.
            event_in: The button event code (e.g., BTN_SOUTH).
            keyout: The key code to output (e.g., KEY_LEFTCTRL).
            shortcut_keyout: Second special shortcut key code to output.


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
        mapping = get_key(config, "mapping", "mapping")
        input_key = get_key(mapping, "input_key", "mapping", expected_type=str)
        self.input_key = get_ecode(input_key, "mapping")
        output_key = mapping["output_key"]
        if output_key is not None:
            self.output_key = get_ecode(output_key, "mapping")
        shortcut_key = mapping["shortcut_key"]
        if shortcut_key is not None:
            self.shortcut_key = get_ecode(shortcut_key, "mapping")

        self.state = {
            "shortcut_state": Reference(False),
            "look": Reference(False),
            "thumb_clicked": Reference(False)
        }
        self.thumb_clicked_last_time = time.monotonic()
        self.thumb_key = e.BTN_THUMBL
        self.this_is_look = False

    def set_shortcut_state_reference(self, shortcut_state: Reference):
        """Set reference to look state."""
        self.state["shortcut_state"] = shortcut_state

    def set_look_reference(self, look: Reference):
        """Set reference to look state."""
        self.state["look"] = look

    def set_thumb_click(self, thumb_clicked: Reference):
        """Set reference to thumbl clicked state."""
        self.state["thumb_clicked"] = thumb_clicked

    def handle_event(self, event):
        """
        Handle digital button events and emit key presses/releases.

        Args:
            event: An evdev input event.
        """
        if event.code == self.input_key:
            if self.state["look"].get() is True or self.this_is_look:
                if event.value == 1:
                    self.state["look"].set(True)
                else:
                    self.state["look"].set(False)
                self.ui.write(e.EV_KEY, self.input_key, event.value)
                self.ui.syn()
            elif self.input_key is self.thumb_key:
                if event.value == 1:
                    now = time.monotonic()
                    if now - self.thumb_clicked_last_time < 0.6:
                        self.state["thumb_clicked"].set(not self.state["thumb_clicked"].get())
                    else:
                        self.thumb_clicked_last_time = now
            elif self.state["shortcut_state"].get() is True and self.shortcut_key is not None:
                self.ui.write(e.EV_KEY, self.shortcut_key, event.value)
                self.ui.syn()
                if event.value == 0:
                    self.state["shortcut_state"].set(False)
            else:
                self.ui.write(e.EV_KEY, self.output_key, event.value)
                self.ui.syn()
