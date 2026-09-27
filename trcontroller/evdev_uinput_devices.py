"""Manages devic."""
import sys

import controller_config
import evdev


class DeviceManager:
    """Manages device discovery and virtual input setup."""

    def __init__(self):
        """Initialize UInput device and internal state."""
        self.device = None
        self.controller = None
        self.ui = evdev.UInput()

    def run(self, config: dict):
        """Start a device and map it to uinput in real-time."""
        devices = config["controller_names"]
        if devices is not None:
            self._set_device(devices)
            components = config["components"]
            if components is not None:
                self.controller = controller_config.Controller(self.ui, self.device, components)
                print("Listening to controller... Press Ctrl+C to exit.")
                try:
                    for event in self.device.read_loop():
                        self.controller.process_event(event)
                except KeyboardInterrupt:
                    print("\nExiting.")
            else:
                print("config fire dose not contain a components section")
        else:
            print("config fire dose not contain a controller_names section")

    def list_devices(self):
        """Return list of input devices as (name, path) tuples."""
        return [(evdev.InputDevice(path).name, path) for path in evdev.list_devices()]

    def _set_device(self, controller_names: list):
        """Search and select a supported controller device."""
        for name, path in self.list_devices():
            if name in controller_names:
                print(f"Using device: {name} at {path}")
                self.device = evdev.InputDevice(path)
                return self.device

        print("No supported controller found. Available devices:")
        for name, path in self.list_devices():
            print(f" - {name} at {path}")
        sys.exit(1)

    def read_events(self, device: str) -> None:
        """Only output events keycode no mapping."""
        self._set_device([device])
        print(f"Reading events from {self.device.name}. Ctrl+C to exit.")
        try:
            for event in self.device.read_loop():
                print(evdev.categorize(event))
        except KeyboardInterrupt:
            print("\nExiting.")
