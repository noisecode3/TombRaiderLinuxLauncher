"""Tests for dpad.py."""
# pylint: disable=missing-function-docstring
from types import SimpleNamespace
from unittest.mock import MagicMock, call

from evdev import ecodes as e

from dpad import Dpad


def ev(code, value):
    return SimpleNamespace(code=code, value=value)


def make_dpad():
    ui = MagicMock()
    cfg = {
        "type": "dpad",
        "axes": {
            "input_x": "ABS_HAT0X",
            "x_keys": {
                "neg_key_value": "KEY_LEFT",
                "pos_key_value": "KEY_RIGHT",
            },
            "input_y": "ABS_HAT0Y",
            "y_keys": {
                "neg_key_value": "KEY_UP",
                "pos_key_value": "KEY_DOWN",
            },
        },
    }
    return Dpad(ui, cfg), ui


def make_axis():
    ui = MagicMock()
    return Dpad.DpadAxis(e.KEY_LEFT, e.KEY_RIGHT), ui


# --- __init__ ---------------------------------------------------------------

def test_init_parses_config():
    handler, _ = make_dpad()
    assert handler.input_x == e.ABS_HAT0X
    assert handler.input_y == e.ABS_HAT0Y
    assert handler.output_x.neg_key == e.KEY_LEFT
    assert handler.output_x.pos_key == e.KEY_RIGHT
    assert handler.output_y.neg_key == e.KEY_UP
    assert handler.output_y.pos_key == e.KEY_DOWN


def test_init_axes_start_inactive():
    handler, _ = make_dpad()
    for axis in (handler.output_x, handler.output_y):
        assert axis.neg_active is False
        assert axis.pos_active is False


# --- DpadAxis ---------------------------------------------------------------

def test_axis_negative_press_and_release():
    axis, ui = make_axis()
    axis.handle(ui, -1)
    assert axis.neg_active is True
    axis.handle(ui, 0)
    assert axis.neg_active is False
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_LEFT, 1),
        call(e.EV_KEY, e.KEY_LEFT, 0),
    ]
    assert ui.syn.call_count == 2


def test_axis_positive_press_and_release():
    axis, ui = make_axis()
    axis.handle(ui, 1)
    assert axis.pos_active is True
    axis.handle(ui, 0)
    assert axis.pos_active is False
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_RIGHT, 1),
        call(e.EV_KEY, e.KEY_RIGHT, 0),
    ]
    assert ui.syn.call_count == 2


def test_axis_repeated_press_writes_once_but_still_syncs():
    axis, ui = make_axis()
    axis.handle(ui, -1)
    axis.handle(ui, -1)
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_LEFT, 1)
    assert ui.syn.call_count == 2


def test_axis_release_when_idle_writes_nothing():
    axis, ui = make_axis()
    axis.handle(ui, 0)
    ui.write.assert_not_called()
    ui.syn.assert_called_once_with()


def test_axis_direct_switch_without_release_keeps_both_active():
    # Documents current behavior: a -1 -> 1 transition without an
    # intervening 0 leaves both keys pressed. Each following 0 only
    # releases one of them (neg first).
    axis, ui = make_axis()
    axis.handle(ui, -1)
    axis.handle(ui, 1)
    assert axis.neg_active is True
    assert axis.pos_active is True

    axis.handle(ui, 0)
    assert axis.neg_active is False
    assert axis.pos_active is True
    axis.handle(ui, 0)
    assert axis.pos_active is False

    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_LEFT, 1),
        call(e.EV_KEY, e.KEY_RIGHT, 1),
        call(e.EV_KEY, e.KEY_LEFT, 0),
        call(e.EV_KEY, e.KEY_RIGHT, 0),
    ]


# --- handle_event -----------------------------------------------------------

def test_x_axis_events_use_x_keys():
    handler, ui = make_dpad()
    handler.handle_event(ev(e.ABS_HAT0X, -1))
    handler.handle_event(ev(e.ABS_HAT0X, 0))
    handler.handle_event(ev(e.ABS_HAT0X, 1))
    handler.handle_event(ev(e.ABS_HAT0X, 0))
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_LEFT, 1),
        call(e.EV_KEY, e.KEY_LEFT, 0),
        call(e.EV_KEY, e.KEY_RIGHT, 1),
        call(e.EV_KEY, e.KEY_RIGHT, 0),
    ]
    assert ui.syn.call_count == 4


def test_y_axis_events_use_y_keys():
    handler, ui = make_dpad()
    handler.handle_event(ev(e.ABS_HAT0Y, -1))
    handler.handle_event(ev(e.ABS_HAT0Y, 0))
    handler.handle_event(ev(e.ABS_HAT0Y, 1))
    handler.handle_event(ev(e.ABS_HAT0Y, 0))
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_UP, 1),
        call(e.EV_KEY, e.KEY_UP, 0),
        call(e.EV_KEY, e.KEY_DOWN, 1),
        call(e.EV_KEY, e.KEY_DOWN, 0),
    ]
    assert ui.syn.call_count == 4


def test_ignores_other_codes():
    handler, ui = make_dpad()
    handler.handle_event(ev(e.ABS_X, 1))
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    ui.write.assert_not_called()
    ui.syn.assert_not_called()


def test_axes_are_independent():
    handler, ui = make_dpad()
    handler.handle_event(ev(e.ABS_HAT0X, -1))
    handler.handle_event(ev(e.ABS_HAT0Y, 1))
    assert handler.output_x.neg_active is True
    assert handler.output_y.pos_active is True

    handler.handle_event(ev(e.ABS_HAT0X, 0))
    assert handler.output_x.neg_active is False
    assert handler.output_y.pos_active is True

    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_LEFT, 1),
        call(e.EV_KEY, e.KEY_DOWN, 1),
        call(e.EV_KEY, e.KEY_LEFT, 0),
    ]
