"""Tests for trigger.py."""
# pylint: disable=missing-function-docstring
from types import SimpleNamespace
from unittest.mock import MagicMock, call

import pytest
from evdev import ecodes as e

from common import Reference
from trigger import Trigger

PRESSED = 255
RELEASED = 0


def ev(code, value, type_=e.EV_ABS):
    return SimpleNamespace(type=type_, code=code, value=value)


def make_trigger(max_value=255, wire_shortcut=True, **mapping):
    ui = MagicMock()
    device = MagicMock()
    device.absinfo.return_value = SimpleNamespace(min=0, max=max_value)
    cfg = {"type": "trigger", "mapping": {"input_key": "ABS_Z", **mapping}}
    handler = Trigger(ui, device, cfg)
    if wire_shortcut:
        handler.set_shortcut_state_reference(Reference(False))
    return handler, ui


# --- __init__ ---------------------------------------------------------------

def test_init_parses_mapping():
    handler, _ = make_trigger(output_key="KEY_A", shortcut_key="KEY_B")
    assert handler.keys.input == e.ABS_Z
    assert handler.keys.output == e.KEY_A
    assert handler.keys.shortcut == e.KEY_B
    assert handler.range == 255


def test_init_optional_keys_default_to_none():
    handler, _ = make_trigger()
    assert handler.keys.output is None
    assert handler.keys.shortcut is None


def test_init_default_threshold():
    handler, _ = make_trigger(output_key="KEY_A")
    assert handler.threshold == 0.90


def test_init_custom_threshold():
    handler, _ = make_trigger(output_key="KEY_A", threshold=0.5)
    assert handler.threshold == 0.5


def test_init_rejects_out_of_range_threshold():
    with pytest.raises(ValueError, match="threshold"):
        make_trigger(output_key="KEY_A", threshold=1.5)


def test_init_rejects_zero_range():
    with pytest.raises(ValueError, match="Invalid axis range"):
        make_trigger(max_value=0, output_key="KEY_A")


# --- plain output -----------------------------------------------------------

def test_plain_output_press_and_release():
    handler, ui = make_trigger(output_key="KEY_LEFTCTRL")
    handler.handle_event(ev(e.ABS_Z, PRESSED))
    handler.handle_event(ev(e.ABS_Z, RELEASED))
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_LEFTCTRL, 1),
        call(e.EV_KEY, e.KEY_LEFTCTRL, 0),
    ]
    assert ui.syn.call_count == 2


def test_below_threshold_does_not_press():
    handler, ui = make_trigger(output_key="KEY_A")
    handler.handle_event(ev(e.ABS_Z, 200))  # 0.78 < 0.90
    ui.write.assert_not_called()


def test_threshold_is_exclusive():
    handler, ui = make_trigger(max_value=100, output_key="KEY_A", threshold=0.5)
    handler.handle_event(ev(e.ABS_Z, 50))  # 0.5 is not > 0.5
    ui.write.assert_not_called()
    handler.handle_event(ev(e.ABS_Z, 51))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)


def test_held_trigger_emits_once():
    handler, ui = make_trigger(output_key="KEY_A")
    handler.handle_event(ev(e.ABS_Z, 240))
    handler.handle_event(ev(e.ABS_Z, PRESSED))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)
    ui.syn.assert_called_once()


def test_ignores_other_codes():
    handler, ui = make_trigger(output_key="KEY_LEFTCTRL")
    handler.handle_event(ev(e.ABS_RZ, PRESSED))
    ui.write.assert_not_called()
    ui.syn.assert_not_called()


def test_no_output_key_writes_nothing():
    handler, ui = make_trigger()
    handler.handle_event(ev(e.ABS_Z, PRESSED))
    ui.write.assert_not_called()


def test_ignores_key_event_with_same_code():
    handler, ui = make_trigger(output_key="KEY_A")
    handler.handle_event(ev(e.ABS_Z, PRESSED, type_=e.EV_KEY))  # ABS_Z == 2 == KEY_1
    ui.write.assert_not_called()


# --- shortcut ---------------------------------------------------------------

def test_works_without_shortcut_reference():
    handler, ui = make_trigger(wire_shortcut=False, output_key="KEY_A")
    handler.handle_event(ev(e.ABS_Z, PRESSED))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)


def test_shortcut_inactive_uses_output_key():
    handler, ui = make_trigger(output_key="KEY_A", shortcut_key="KEY_B")
    handler.handle_event(ev(e.ABS_Z, PRESSED))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)


def test_shortcut_active_emits_shortcut_key_and_resets_on_release():
    handler, ui = make_trigger(output_key="KEY_A", shortcut_key="KEY_B")
    state = Reference(True)
    handler.set_shortcut_state_reference(state)

    handler.handle_event(ev(e.ABS_Z, PRESSED))
    assert state.get() is True
    handler.handle_event(ev(e.ABS_Z, RELEASED))
    assert state.get() is False
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_B, 1),
        call(e.EV_KEY, e.KEY_B, 0),
    ]


def test_shortcut_state_without_shortcut_key_falls_back_to_output():
    handler, ui = make_trigger(output_key="KEY_A")
    state = Reference(True)
    handler.set_shortcut_state_reference(state)

    handler.handle_event(ev(e.ABS_Z, PRESSED))
    handler.handle_event(ev(e.ABS_Z, RELEASED))
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_A, 1),
        call(e.EV_KEY, e.KEY_A, 0),
    ]
    assert state.get() is True


# --- look -------------------------------------------------------------------

def test_set_look_reference_requires_output_key():
    handler, _ = make_trigger()
    with pytest.raises(ValueError, match="requires output_key"):
        handler.set_look_reference(Reference(False))


def test_look_sets_state_and_writes_output_key():
    handler, ui = make_trigger(output_key="KEY_A")
    look = Reference(False)
    handler.set_look_reference(look)

    handler.handle_event(ev(e.ABS_Z, PRESSED))
    assert look.get() is True
    handler.handle_event(ev(e.ABS_Z, RELEASED))
    assert look.get() is False
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_A, 1),
        call(e.EV_KEY, e.KEY_A, 0),
    ]
    assert ui.syn.call_count == 2


def test_look_writes_output_key():
    handler, ui = make_trigger(output_key="KEY_A")
    handler.set_look_reference(Reference(False))

    handler.handle_event(ev(e.ABS_Z, PRESSED))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)
