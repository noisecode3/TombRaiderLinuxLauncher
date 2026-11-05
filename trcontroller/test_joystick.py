"""Tests for joystick.py."""
# pylint: disable=missing-function-docstring
import math
from types import SimpleNamespace
from unittest.mock import MagicMock, call

import pytest
from evdev import ecodes as e

from common import Reference
from joystick import Joystick

# Axis range in the tests is -100..100: center is 0 and 1.0 == 100 raw.
HALF = 100

SECTOR_DEFAULTS = {
    "up": {"sub_right_up": 60.0, "sub_left_up": 120.0,
           "size": 138.0, "output_key": "KEY_UP"},
    "right": {"right_up": 20.0, "right_down": -20.0,
              "size": 138.0, "output_key": "KEY_RIGHT"},
    "down": {"sub_right_down": -60.0, "sub_left_down": -120.0,
             "size": 138.0, "output_key": "KEY_DOWN"},
    "left": {"left_up": 160.0, "left_down": -160.0,
             "size": 138.0, "output_key": "KEY_LEFT"},
}
FOURWAY = {"up": "KEY_1", "down": "KEY_2", "left": "KEY_3", "right": "KEY_4"}
CIRCLE = {"cw_key": "KEY_5", "ccw_key": "KEY_6"}
DOUBLE_CLICK = {
    "input_key": "BTN_THUMBL",
    "toggle_shortcut_mode": True,
    "toggle_hold_output_key": False,
    "output_key": None,
}


def ev(code, value, type_=e.EV_ABS):
    return SimpleNamespace(type=type_, code=code, value=value)


def sectors(use_sector_size_for_movement=False, **per_direction):
    """Build a sectors config, overriding single fields per direction."""
    cfg = {
        "use_sector_size_for_movement": use_sector_size_for_movement,
        "use_sector_size_for_look_key": "KEY_KP0",
    }
    for name, base in SECTOR_DEFAULTS.items():
        cfg[name] = {**base, **per_direction.get(name, {})}
    return cfg


def make_joystick(sectors_cfg=None, circle=None, fourway=None,
                  abs_min=-100, abs_max=100, delay=0.0,
                  axes=("ABS_X", "ABS_Y")):
    ui = MagicMock()
    device = MagicMock()
    device.absinfo.return_value = SimpleNamespace(min=abs_min, max=abs_max)
    cfg = {
        "type": "joystick",
        "axes": {"input_x": axes[0], "input_y": axes[1]},
        "sectors": sectors_cfg,
        "circle_trigger_shortcuts": circle,
        "fourway_trigger_shortcuts": fourway,
        "double_click_key": dict(DOUBLE_CLICK),
    }
    handler = Joystick(ui, device, cfg)
    # Disable bounce protection unless a test wants it.
    handler.stick.vector.deadzone_delay = delay
    return handler, ui


def make_left(sectors_cfg=None, **kwargs):
    return make_joystick(sectors_cfg=sectors_cfg or sectors(), **kwargs)


def make_right(**kwargs):
    return make_joystick(axes=("ABS_RX", "ABS_RY"), **kwargs)


def tilt(handler, deg, radius=1.0):
    """Push the stick to an angle (0 = right, 90 = up) and radius."""
    rad = math.radians(deg)
    handler.handle_event(ev(handler.stick.internal.abs_x,
                            math.cos(rad) * radius * HALF))
    # Raw Y is inverted: up is negative.
    handler.handle_event(ev(handler.stick.internal.abs_y,
                            -math.sin(rad) * radius * HALF))


def center(handler):
    tilt(handler, 0, 0)


def sweep(handler, start, stop, step):
    """Rotate the stick in small steps. Radius > 1 keeps partial axis
    updates (x first, then y) outside the deadzone."""
    for deg in range(start, stop, step):
        tilt(handler, deg, radius=1.2)


def held(ui):
    """Keys currently held down according to the writes so far."""
    keys = set()
    for c in ui.write.call_args_list:
        _, code, value = c.args
        if value:
            keys.add(code)
        else:
            keys.discard(code)
    return keys


# --- __init__ ---------------------------------------------------------------

def test_init_with_sectors_builds_default_and_sector_size_handlers():
    handler, _ = make_left()
    assert handler.handlers.default is not None
    assert handler.handlers.sector_size is not None
    assert handler.handlers.circle_trigger is None
    assert handler.handlers.fourway_trigger is None
    assert handler.stick.directional_buttons is not None
    assert handler.use_sector_size_for_look_key == e.KEY_KP0


def test_init_without_sectors_has_no_directional_handlers():
    handler, _ = make_right()
    assert handler.handlers.default is None
    assert handler.handlers.sector_size is None
    assert handler.stick.directional_buttons is None


def test_init_uses_axes_from_config():
    handler, _ = make_right()
    assert handler.stick.internal.abs_x == e.ABS_RX
    assert handler.stick.internal.abs_y == e.ABS_RY


def test_init_missing_sector_points_fall_back_to_defaults():
    handler, _ = make_left(sectors(right={"right_up": None}))
    points = handler.handlers.default.sector_points
    assert points.right_up == 20
    assert points.right_down == -20.0


def test_init_sector_sizes_from_config_and_default():
    handler, _ = make_left(sectors(up={"size": None}, right={"size": 90.0}))
    assert handler.handlers.sector_size.sector_size_up == 138
    assert handler.handlers.sector_size.sector_size_right == 90.0


def test_init_classic_overlap_from_config():
    handler, _ = make_left(sectors(use_sector_size_for_movement=True))
    assert handler.state.classic_overlap is True


def test_init_circle_trigger_handler():
    handler, _ = make_right(circle=CIRCLE)
    assert handler.handlers.circle_trigger.cw_key == e.KEY_5
    assert handler.handlers.circle_trigger.ccw_key == e.KEY_6


def test_init_fourway_key_order_is_up_down_left_right():
    handler, _ = make_right(fourway=FOURWAY)
    assert handler.handlers.fourway_trigger.fourway_key_list == [
        e.KEY_1, e.KEY_2, e.KEY_3, e.KEY_4,
    ]


def test_init_stores_double_click_config():
    handler, _ = make_left()
    assert handler.double_click_key.input_key_str == "BTN_THUMBL"
    assert handler.double_click_key.output_key_str is None
    assert handler.double_click_key.toggle_shortcut_mode is True
    assert handler.double_click_key.toggle_hold_output_key is False


# --- state references -------------------------------------------------------

def test_set_clicked_state_reference():
    handler, _ = make_left()
    ref = Reference(False)
    handler.set_clicked_state_reference(ref)
    assert handler.state.clicked is ref


def test_set_look_state_reference():
    handler, _ = make_left()
    look = Reference(False)
    handler.set_look_state_reference(look)
    assert handler.state.clicked_look is look


def test_set_look_state_reference_rejects_non_reference():
    handler, _ = make_left()
    with pytest.raises(TypeError, match="look is not Reference$"):
        handler.set_look_state_reference(True)


def test_set_look_state_reference_rejects_non_bool_reference():
    handler, _ = make_left()
    with pytest.raises(TypeError, match=r"Reference\(bool\)"):
        handler.set_look_state_reference(Reference(1))


def test_set_clicked_shortcut_reference_propagates_to_fourway():
    handler, _ = make_right(fourway=FOURWAY)
    ref = Reference(False)
    handler.set_clicked_shortcut_state_reference(ref)
    assert handler.state.clicked_shortcut is ref
    assert handler.handlers.fourway_trigger.clicked_state_reference is ref


def test_set_clicked_shortcut_reference_without_fourway():
    handler, _ = make_left()
    ref = Reference(False)
    handler.set_clicked_shortcut_state_reference(ref)
    assert handler.state.clicked_shortcut is ref


# --- event filtering and normalization --------------------------------------

def test_ignores_non_abs_events():
    handler, ui = make_left()
    handler.handle_event(ev(e.ABS_X, HALF, type_=e.EV_KEY))
    assert handler.stick.internal.current_x == 0.0
    ui.write.assert_not_called()


def test_ignores_other_axes():
    handler, ui = make_left()
    handler.handle_event(ev(e.ABS_RX, HALF))
    handler.handle_event(ev(e.ABS_RY, HALF))
    assert handler.stick.internal.current_x == 0.0
    assert handler.stick.internal.current_y == 0.0
    ui.write.assert_not_called()


def test_normalizes_axes_and_inverts_y():
    handler, _ = make_left()
    handler.handle_event(ev(e.ABS_X, 50))
    handler.handle_event(ev(e.ABS_Y, 50))
    assert handler.stick.internal.current_x == pytest.approx(0.5)
    assert handler.stick.internal.current_y == pytest.approx(-0.5)


def test_normalizes_unsigned_axis_range():
    handler, _ = make_left(abs_min=0, abs_max=255)
    handler.handle_event(ev(e.ABS_X, 255))
    assert handler.stick.internal.current_x == pytest.approx(1.0)
    handler.handle_event(ev(e.ABS_X, 0))
    assert handler.stick.internal.current_x == pytest.approx(-1.0)


# --- default 8-way direction detection --------------------------------------

@pytest.mark.parametrize("deg, expected", [
    (90, {e.KEY_UP}),
    (0, {e.KEY_RIGHT}),
    (170, {e.KEY_LEFT}),
    (-90, {e.KEY_DOWN}),
    (45, {e.KEY_UP, e.KEY_RIGHT}),
    (135, {e.KEY_UP, e.KEY_LEFT}),
    (-45, {e.KEY_DOWN, e.KEY_RIGHT}),
    (-135, {e.KEY_DOWN, e.KEY_LEFT}),
])
def test_default_eight_way_directions(deg, expected):
    handler, ui = make_left()
    tilt(handler, deg)
    assert held(ui) == expected


def test_below_threshold_presses_nothing():
    handler, ui = make_left()
    tilt(handler, 90, radius=0.5)
    ui.write.assert_not_called()


def test_returning_to_center_releases_everything():
    handler, ui = make_left()
    tilt(handler, 45)
    assert held(ui) == {e.KEY_UP, e.KEY_RIGHT}
    center(handler)
    assert held(ui) == set()


def test_changing_direction_releases_previous_key():
    handler, ui = make_left()
    tilt(handler, 90)
    tilt(handler, 0)
    assert held(ui) == {e.KEY_RIGHT}


def test_held_direction_emits_once():
    handler, ui = make_left()
    tilt(handler, 90)
    ui.write.reset_mock()
    tilt(handler, 90)
    ui.write.assert_not_called()


def test_every_write_is_followed_by_syn():
    handler, ui = make_left()
    tilt(handler, 45)
    center(handler)
    assert ui.syn.call_count == ui.write.call_count


def test_radius_between_hysteresis_and_threshold_keeps_state():
    handler, ui = make_left()
    tilt(handler, 90)
    tilt(handler, 90, radius=0.92)  # below 0.95 but above 0.90
    assert held(ui) == {e.KEY_UP}
    tilt(handler, 90, radius=0.5)
    assert held(ui) == set()


def test_angle_hysteresis_keeps_previous_sector_near_boundary():
    handler, ui = make_left()
    tilt(handler, 0)
    tilt(handler, 21)  # right/up boundary is 20, widened by 2 while in "right"
    assert held(ui) == {e.KEY_RIGHT}


def test_angle_hysteresis_keeps_up_sector_near_boundary():
    handler, ui = make_left()
    tilt(handler, 90)
    tilt(handler, 19)  # boundary is 20, narrowed by 2 while in "up"
    assert e.KEY_UP in held(ui)


# --- bounce protection ------------------------------------------------------

def test_bounce_protection_ignores_push_right_after_deadzone(monkeypatch):
    clock = [0.0]
    monkeypatch.setattr("joystick.time.monotonic", lambda: clock[0])
    handler, ui = make_left(delay=0.2)

    center(handler)  # enters deadzone at t=0
    clock[0] = 0.1
    tilt(handler, 90)
    assert held(ui) == set()

    clock[0] = 0.3
    tilt(handler, 90)
    assert held(ui) == {e.KEY_UP}


# --- sector size / look mode ------------------------------------------------

WIDE_UP = {
    "up": {"size": 180.0},
    "right": {"size": 90.0},
    "down": {"size": 90.0},
    "left": {"size": 90.0},
}


def test_without_overlap_uses_default_boundaries():
    handler, ui = make_left(sectors(**WIDE_UP))
    tilt(handler, 10)
    assert held(ui) == {e.KEY_RIGHT}


def test_classic_overlap_uses_sector_sizes():
    handler, ui = make_left(sectors(use_sector_size_for_movement=True, **WIDE_UP))
    tilt(handler, 10)
    assert held(ui) == {e.KEY_UP, e.KEY_RIGHT}


def test_look_active_uses_sector_sizes():
    handler, ui = make_left(sectors(**WIDE_UP))
    handler.set_look_state_reference(Reference(True))
    tilt(handler, 10)
    assert held(ui) == {e.KEY_UP, e.KEY_RIGHT}


def test_look_inactive_uses_default_boundaries():
    handler, ui = make_left(sectors(**WIDE_UP))
    handler.set_look_state_reference(Reference(False))
    tilt(handler, 10)
    assert held(ui) == {e.KEY_RIGHT}


def test_sector_size_mode_releases_in_deadzone():
    handler, ui = make_left(sectors(use_sector_size_for_movement=True, **WIDE_UP))
    tilt(handler, 10)
    center(handler)
    assert held(ui) == set()


# --- four-way trigger shortcuts ---------------------------------------------

def make_fourway(clicked=True):
    handler, ui = make_right(fourway=FOURWAY)
    ref = Reference(clicked)
    handler.set_clicked_shortcut_state_reference(ref)
    return handler, ui, ref


@pytest.mark.parametrize("deg, key", [
    (90, e.KEY_1),
    (-90, e.KEY_2),
    (170, e.KEY_3),
    (0, e.KEY_4),
])
def test_fourway_fires_key_for_direction(deg, key):
    handler, ui, _ = make_fourway()
    tilt(handler, deg)
    assert ui.write.call_args_list == [call(e.EV_KEY, key, 1)]
    assert ui.syn.call_count == 1


def test_fourway_releases_key_and_clears_clicked_in_deadzone():
    handler, ui, clicked = make_fourway()
    tilt(handler, 90)
    center(handler)
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_1, 1),
        call(e.EV_KEY, e.KEY_1, 0),
    ]
    assert clicked.get() is False


def test_fourway_ignores_direction_change_while_active():
    handler, ui, _ = make_fourway()
    tilt(handler, 90)
    ui.write.reset_mock()
    tilt(handler, 0)
    ui.write.assert_not_called()


def test_fourway_inactive_when_shortcut_not_clicked():
    handler, ui, _ = make_fourway(clicked=False)
    tilt(handler, 90)
    ui.write.assert_not_called()


# --- circle trigger shortcuts -----------------------------------------------

def test_circle_clockwise_from_up_triggers_cw_key():
    # Docstring: cw_key starts from UP to right to down.
    handler, ui = make_right(circle=CIRCLE)
    sweep(handler, 90, -121, -15)
    assert ui.write.call_args_list == [call(e.EV_KEY, e.KEY_5, 1)]


def test_circle_counter_clockwise_from_up_triggers_ccw_key():
    # Docstring: ccw_key starts from UP to left to down.
    handler, ui = make_right(circle=CIRCLE)
    sweep(handler, 90, 301, 15)
    assert ui.write.call_args_list == [call(e.EV_KEY, e.KEY_6, 1)]


def test_circle_half_turn_not_reached_does_nothing():
    handler, ui = make_right(circle=CIRCLE)
    sweep(handler, 90, -1, -15)  # 90 -> 0 degrees
    ui.write.assert_not_called()


def test_circle_fires_once_per_rotation():
    handler, ui = make_right(circle=CIRCLE)
    sweep(handler, 90, -451, -15)  # a full turn
    assert len(ui.write.call_args_list) == 1


def test_circle_releases_key_in_deadzone():
    handler, ui = make_right(circle=CIRCLE)
    sweep(handler, 90, -121, -15)
    center(handler)
    press, release = ui.write.call_args_list
    assert press.args[2] == 1
    assert release.args[2] == 0
    assert release.args[1] == press.args[1]


def test_circle_ignores_rotation_not_started_near_up():
    handler, ui = make_right(circle=CIRCLE)
    sweep(handler, -90, -451, -15)  # starts at the bottom
    ui.write.assert_not_called()
