"""Tests for key.py."""
# pylint: disable=missing-function-docstring
from types import SimpleNamespace
from unittest.mock import MagicMock, call

from evdev import ecodes as e

import key as key_module
from common import Reference
from key import Key


def ev(code, value):
    return SimpleNamespace(code=code, value=value)


def make_key(**mapping):
    ui = MagicMock()
    cfg = {"type": "key", "mapping": {"input_key": "BTN_SOUTH", **mapping}}
    return Key(ui, cfg), ui


def fake_time(*values):
    it = iter(values)
    return SimpleNamespace(monotonic=lambda: next(it))


# --- __init__ ---------------------------------------------------------------

def test_init_parses_mapping():
    handler, _ = make_key(output_key="KEY_A", shortcut_key="KEY_B")
    assert handler.input_key == e.BTN_SOUTH
    assert handler.output_key == e.KEY_A
    assert handler.shortcut_key == e.KEY_B


def test_init_optional_keys_default_to_none():
    handler, _ = make_key()
    assert handler.output_key is None
    assert handler.shortcut_key is None


# --- plain output -----------------------------------------------------------

def test_plain_output_press_and_release():
    handler, ui = make_key(output_key="KEY_LEFTCTRL")
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    handler.handle_event(ev(e.BTN_SOUTH, 0))
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_LEFTCTRL, 1),
        call(e.EV_KEY, e.KEY_LEFTCTRL, 0),
    ]
    assert ui.syn.call_count == 2


def test_ignores_other_codes():
    handler, ui = make_key(output_key="KEY_LEFTCTRL")
    handler.handle_event(ev(e.BTN_EAST, 1))
    ui.write.assert_not_called()
    ui.syn.assert_not_called()


def test_no_output_key_writes_nothing():
    # Fails today: handle_event calls ui.write(EV_KEY, None, ...).
    handler, ui = make_key()
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    ui.write.assert_not_called()


# --- shortcut ---------------------------------------------------------------

def test_shortcut_inactive_uses_output_key():
    handler, ui = make_key(output_key="KEY_A", shortcut_key="KEY_B")
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)


def test_shortcut_active_emits_shortcut_key_and_resets_on_release():
    handler, ui = make_key(output_key="KEY_A", shortcut_key="KEY_B")
    state = Reference(True)
    handler.set_shortcut_state_reference(state)

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert state.get() is True
    handler.handle_event(ev(e.BTN_SOUTH, 0))
    assert state.get() is False
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_B, 1),
        call(e.EV_KEY, e.KEY_B, 0),
    ]


def test_shortcut_state_without_shortcut_key_falls_back_to_output():
    handler, ui = make_key(output_key="KEY_A")
    state = Reference(True)
    handler.set_shortcut_state_reference(state)

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    handler.handle_event(ev(e.BTN_SOUTH, 0))
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_A, 1),
        call(e.EV_KEY, e.KEY_A, 0),
    ]
    assert state.get() is True


# --- look -------------------------------------------------------------------

def test_look_sets_state_and_writes_output_key():
    handler, ui = make_key(output_key="KEY_A")
    look = Reference(False)
    handler.set_look_reference(look)
    handler.this_is_look = True

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert look.get() is True
    handler.handle_event(ev(e.BTN_SOUTH, 0))
    assert look.get() is False
    assert ui.write.call_args_list == [
        call(e.EV_KEY, e.KEY_A, 1),
        call(e.EV_KEY, e.KEY_A, 0),
    ]
    assert ui.syn.call_count == 2


def test_look_writes_output_key():
    handler, ui = make_key(output_key="KEY_A")
    handler.set_look_reference(Reference(False))
    handler.this_is_look = True

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    ui.write.assert_called_once_with(e.EV_KEY, e.KEY_A, 1)


# --- thumb ------------------------------------------------------------------

def _thumb_key():
    handler, ui = make_key()
    clicked = Reference(False)
    handler.set_thumb_click(clicked)
    return handler, ui, clicked


def test_thumb_double_click_toggles(monkeypatch):
    handler, ui, clicked = _thumb_key()
    monkeypatch.setattr(key_module, "time", fake_time(100.0, 100.3))

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert clicked.get() is False
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert clicked.get() is True
    ui.write.assert_not_called()


def test_thumb_slow_clicks_do_not_toggle(monkeypatch):
    handler, ui, clicked = _thumb_key()
    monkeypatch.setattr(key_module, "time", fake_time(100.0, 101.0))

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert clicked.get() is False
    ui.write.assert_not_called()


def test_thumb_release_does_nothing(monkeypatch):
    handler, ui, clicked = _thumb_key()
    # No values: any call to monotonic() raises StopIteration.
    monkeypatch.setattr(key_module, "time", fake_time())

    handler.handle_event(ev(e.BTN_SOUTH, 0))
    assert clicked.get() is False
    ui.write.assert_not_called()


def test_thumb_third_click_compares_against_first(monkeypatch):
    # Documents current behavior: the timestamp is only updated on a
    # non-toggling click, so click 3 is measured against click 1.
    handler, _, clicked = _thumb_key()
    monkeypatch.setattr(key_module, "time", fake_time(100.0, 100.3, 100.5))

    handler.handle_event(ev(e.BTN_SOUTH, 1))
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert clicked.get() is True
    handler.handle_event(ev(e.BTN_SOUTH, 1))
    assert clicked.get() is False
