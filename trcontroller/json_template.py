"""Template for creating PS4 controller input mapper config files in json."""


def make_json_template_only_ps4_share():
    """Template for creating only PS4 share button."""
    return {
        "controller_names": [
            "Wireless Controller",
            "Sony Interactive Entertainment Wireless Controller"
        ],
        "components": [
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_SELECT",
                    "output_key": "KEY_COMMA",
                    "shortcut_key": None
                }
            }
        ]
    }


def make_json_template_only_left_stick():
    """Template for creating only PS4 left stick movement input."""
    return {
        "controller_names": [
            "Wireless Controller",
            "Sony Interactive Entertainment Wireless Controller"
        ],
        "components": [
            {
                "type": "joystick",
                "axes": {
                    "input_x": "ABS_X",
                    "input_y": "ABS_Y"
                },
                "sectors": {
                    "use_sector_size_for_movement": False,
                    "use_sector_size_for_look_key": None,
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
                "fourway_trigger_shortcuts": None,
                "double_click_key": {
                    "input_key": "BTN_THUMBL",
                    "toggle_shortcut_mode": False,
                    "toggle_hold_output_key": False,
                    "output_key": None
                }
            }
        ]
    }


def make_json_template(mode):
    """Template for creating complete PS4 input config files."""
    left_joystick_fourway_trigger_shortcuts = {
        k: {"up": "KEY_1", "down": "KEY_2", "left": "KEY_3", "right": "KEY_4"}
        for k in ("TombRaider3to5", "TombRaider2")
    }
    left_joystick_fourway_trigger_shortcuts = \
        left_joystick_fourway_trigger_shortcuts.get(mode)

    right_joystick_fourway_trigger_shortcuts = {
        k: {"up": "KEY_5", "down": "KEY_6", "left": "KEY_7", "right": None}
        for k in ("TombRaider3to5", "TombRaider2")
    }
    right_joystick_fourway_trigger_shortcuts["TombRaider3to5"]["right"] = \
        "KEY_8"
    right_joystick_fourway_trigger_shortcuts = \
        right_joystick_fourway_trigger_shortcuts.get(mode)

    right_circle_trigger_presets = {
        "TombRaider3to5": {"cw_key": "KEY_9",  "ccw_key": "KEY_0"},
        "TombRaider2": {"cw_key": "KEY_8",  "ccw_key": "KEY_9"},
        "TombRaider1": {"cw_key": "KEY_F5", "ccw_key": "KEY_F6"}
    }
    right_circle_trigger_shortcuts = right_circle_trigger_presets.get(mode)

    toggle_shortcut_mode = mode in ("TombRaider3to5", "TombRaider2")

    left_trigger = "KEY_DOT" if mode == "TombRaider3to5" else "KEY_DELETE"
    right_trigger = "KEY_SLASH" if mode == "TombRaider3to5" else "KEY_PAGEDOWN"

    left_bumper_shortcut_key = "KEY_J" if mode == "TombRaider3to5" else None
    right_bumper_shortcut_key = "KEY_P" if mode == "TombRaider3to5" else None

    select_key = "KEY_COMMA" if mode != "TombRaider1" else None

    select_shortcut_key = \
        "KEY_F5" if mode in ("TombRaider3to5", "TombRaider2") else None
    start_shortcut_key = \
        "KEY_F6" if mode in ("TombRaider3to5", "TombRaider2") else None

    tr_sony = {
        "controller_names": [
            "Wireless Controller",
            "Sony Interactive Entertainment Wireless Controller"
        ],
        "components": [
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
            },
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
            },
            {
                "type": "trigger",
                "mapping": {
                    "input_key": "ABS_Z",
                    "output_key": left_trigger,
                    "shortcut_key": None,
                    "threshold": 0.90
                }
            },
            {
                "type": "trigger",
                "mapping": {
                    "input_key": "ABS_RZ",
                    "output_key": right_trigger,
                    "shortcut_key": None,
                    "threshold": 0.90
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_TL",
                    "output_key": "KEY_KP0",
                    "shortcut_key": left_bumper_shortcut_key
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_TR",
                    "output_key": "KEY_LEFTSHIFT",
                    "shortcut_key": right_bumper_shortcut_key
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_SELECT",
                    "output_key": select_key,
                    "shortcut_key": select_shortcut_key
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_START",
                    "output_key": "KEY_ESC",
                    "shortcut_key": start_shortcut_key
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_NORTH",
                    "output_key": "KEY_SPACE",
                    "shortcut_key": None
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_EAST",
                    "output_key": "KEY_END",
                    "shortcut_key": None
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_SOUTH",
                    "output_key": "KEY_LEFTCTRL",
                    "shortcut_key": None
                }
            },
            {
                "type": "key",
                "mapping": {
                    "input_key": "BTN_WEST",
                    "output_key": "KEY_LEFTALT",
                    "shortcut_key": None
                }
            }
        ]
    }

    return tr_sony
