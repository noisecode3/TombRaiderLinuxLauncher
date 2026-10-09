"""Put togheter the controller from components in the configuration file."""
from __future__ import annotations

import evdev
from evdev import ecodes as e

from common import Reference
from dpad import Dpad
from joystick import Joystick
from key import Key
from trigger import Trigger


class Controller:
    """Manages input mappings for a game controller."""

    def __init__(self, ui: evdev.UInput, device: evdev.InputDevice, components: list):
        """
        Initialize the Controller with input handlers.

        Args:
            ui: The uinput device.
        """
        self.abs_handlers: list[Joystick | Trigger | Dpad] = []
        self.key_handlers: list[Key] = []
        self.ui = ui
        self.device = device
        self.shortcut_state_ref = Reference(False)
        self.look_state_ref = Reference(False)
        self.look_key = None

        component_builders = {
            "dpad": self.build_dpad,
            "joystick": self.build_joystick,
            "key": self.build_key,
            "trigger": self.build_trigger,
        }

        for i, component in enumerate(components):
            if not isinstance(component, dict):
                raise TypeError(f"components[{i}] is not dict: {component!r}")

            comp_type = component.get("type")
            if not isinstance(comp_type, str):
                raise TypeError(f"components[{i}] does not have a 'type'-key: {comp_type!r}")
            builder = component_builders.get(comp_type)
            if builder is None:
                raise ValueError(f"components[{i}] have an invalid type: {comp_type!r}")

            obj = builder(component)

            if isinstance(obj, (Joystick, Trigger, Dpad)):
                self.abs_handlers.append(obj)
            elif isinstance(obj, (Key)):
                self.key_handlers.append(obj)
            else:
                raise TypeError(f"components[{i}]:'{comp_type}' returned unkown type {type(obj)!r}")

        joysticks = (h for h in self.abs_handlers if isinstance(h, Joystick))

        for j in joysticks:

            if j.double_click_key is not None:

                output_key = j.double_click_key["output_key"]
                if j.double_click_key["toggle_shortcut_mode"] is not None and \
                        j.double_click_key["toggle_shortcut_mode"] is True:
                    output_key = None

                key_config = {
                    "type": "key",
                    "mapping": {
                        "input_key": j.double_click_key["input_key"],
                        "output_key": output_key,
                        "shortcut_key": None
                    }
                }

                key = self.build_key(key_config)

                if j.double_click_key["toggle_shortcut_mode"] is not None and \
                        j.double_click_key["toggle_shortcut_mode"] is True:
                    key.set_thumb_click(self.shortcut_state_ref)

                j.set_clicked_shortcut_state_reference(self.shortcut_state_ref)

                if j.double_click_key["toggle_hold_output_key"] is not None and \
                        j.double_click_key["toggle_hold_output_key"] is True:
                    pass

                self.key_handlers.append(key)

            if isinstance(j.use_sector_size_for_look_key, int) and \
                    j.use_sector_size_for_look_key in e.KEY:
                self.look_key = j.use_sector_size_for_look_key
                j.set_look_state_reference(self.look_state_ref)

        triggers = (h for h in self.abs_handlers if isinstance(h, Trigger))
        for t in triggers:
            t.set_shortcut_state_reference(self.shortcut_state_ref)
            if t.keys.output == self.look_key:
                t.set_look_reference(self.look_state_ref)

        keys = (h for h in self.key_handlers if isinstance(h, Key))
        for k in keys:
            k.set_shortcut_state_reference(self.shortcut_state_ref)
            if k.output_key == self.look_key:
                k.set_look_reference(self.look_state_ref)

    def build_dpad(self, component: dict) -> Dpad:
        """Add D-pad handler."""
        return Dpad(self.ui, component)

    def build_key(self, component: dict) -> Key:
        """Add digital button-to-key handler."""
        return Key(self.ui, component)

    def build_joystick(self, component: dict) -> Joystick:
        """Add analog stick handler."""
        return Joystick(ui=self.ui, device=self.device, config=component)

    def build_trigger(self, component: dict) -> Trigger:
        """Add analog trigger handler."""
        return Trigger(self.ui, self.device, component)

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
