"""Common classes and functions for TRController."""
from evdev import ecodes as e


class Reference:
    """
    An object that is meant to be shared between objects.

    This can be used like a QT signal between objects running in a loop.
    It should be used on the same thread.
    """

    def __init__(self, value):
        """
        Initialize the reference with an initial value.

        Args:
            value: The initial value to store.
        """
        self.value = value

    def set(self, value):
        """
        Update the stored value.

        Args:
            value: The new value to store. Any objects holding this
            Reference will see the updated value on their next get().
        """
        self.value = value

    def get(self):
        """
        Return the currently stored value.

        Returns:
            The current value.
        """
        return self.value


def get_key(obj, key: str, path: str, expected_type: type = dict):
    """Retrieve `key` from `obj` (a plain dict) and validate its type."""
    if key not in obj:
        raise KeyError(f"Missing required config key '{path}'")
    value = obj[key]
    if not isinstance(value, expected_type):
        parts = path.split(".")
        structure = "{...}"
        for part in reversed(parts):
            structure = f"{{{part}: {structure}}}"
        raise TypeError(
            f"Config key '{path}' must be a {expected_type.__name__}, "
            f"got {type(value).__name__}.\n"
            f"Expected structure:\n"
            f"  {structure}"
        )
    return value


def get_ecode(name: str, path: str):
    """Resolve a string like 'KEY_LEFT' to its evdev ecode constant."""
    try:
        return getattr(e, name)
    except AttributeError as exc:
        raise ValueError(
            f"Config key '{path}' has value '{name}', which is not a valid evdev key code."
        ) from exc
