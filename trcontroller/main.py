"""
Controller input handler to map controller events to keyboard keys using evdev and UInput.

...
"""
import argparse
import json

import json_template
from evdev_uinput_devices import DeviceManager

TEMPLATES = {
    "ps4-3to5": "TombRaider3to5",
    "ps4-2": "TombRaider2",
    "ps4-1": "TombRaider1",
}

SPECIAL_TEMPLATES = {
    "only-ps4-share": json_template.make_json_template_only_ps4_share,
    "only-left-stick": json_template.make_json_template_only_left_stick,
}


def _emit_template(name: str) -> None:
    if name in TEMPLATES:
        data = json_template.make_json_template(TEMPLATES[name])
    elif name in SPECIAL_TEMPLATES:
        data = SPECIAL_TEMPLATES[name]()
    else:
        raise ValueError(f"unknown template: {name}")
    print(json.dumps(data, indent=4, ensure_ascii=False))


def _run_mapper(config_path: str) -> None:
    with open(config_path, encoding="utf-8") as f:
        config = json.load(f)
    DeviceManager().run(config)


def _read_device_events(device: str) -> None:
    DeviceManager().read_events(device)


def _list_devices() -> None:
    for name, path in DeviceManager().list_devices():
        print(f"{name}: {path}")


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="main.py",
        description="PS4 Controller Mapper using evdev + uinput",
    )
    parser.add_argument("--version", action="version", version="%(prog)s 1.0.0")
    sub = parser.add_subparsers(dest="command", required=True)

    p_template = sub.add_parser("template", help="print a controller JSON template to stdout")
    p_template.add_argument(
        "name",
        choices=[*TEMPLATES, *SPECIAL_TEMPLATES],
        help="Example: template ps4-3to5 > my_controller.json",
    )

    p_run = sub.add_parser("run", help="run the controller mapper with a config file")
    p_run.add_argument("config", help="path to controller JSON config, e.g. my_controller.json")

    sub.add_parser("list", help="List all devices on the system")

    p_read = sub.add_parser("read", help="run the controller mapper with a config file")
    p_read.add_argument("device", help="path to controller JSON config, e.g. my_controller.json")

    return parser


def _main() -> None:
    args = _build_parser().parse_args()
    if args.command == "template":
        _emit_template(args.name)
    elif args.command == "list":
        _list_devices()
    elif args.command == "read":
        _read_device_events(args.device)
    elif args.command == "run":
        _run_mapper(args.config)


if __name__ == "__main__":
    _main()
