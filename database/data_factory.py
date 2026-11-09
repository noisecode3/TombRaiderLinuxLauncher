"""Index data definitions."""
# pylint: disable=too-many-instance-attributes
from dataclasses import dataclass, field


@dataclass
class ZipFile:
    """trcustoms.org or TRLE zipfile."""

    name: str = ""
    size: float = 0.0
    md5: str = ""
    url: str = ""
    release: str = ""
    version: str = ""


@dataclass
class Picture:
    """trcustoms.org or TRLE picture data."""

    position: int = 0
    md5sum: str = ""
    md5: bytes = b""


@dataclass
class TrleLevel:
    """trle.net record data."""

    trle_id: int = 0
    author: str = ""
    title: str = ""
    difficulty: str = ""
    duration: str = ""
    class_: str = ""  # "class" is a reserved word
    type: str = ""
    release: str = ""


@dataclass
class TrlePage:
    """trle.net page."""

    offset: int = 0
    records_total: int = 0
    levels: list[TrleLevel] = field(default_factory=list)


@dataclass
class TrleTombll:
    """Level data the app uses at the moment."""

    title: str = ""
    authors: list[str] = field(default_factory=list)
    tags: list[str] = field(default_factory=list)
    genres: list[str] = field(default_factory=list)
    type: str = ""
    class_: str = ""
    release: str = ""
    difficulty: str = ""
    duration: str = ""
    screen: str = ""
    large_screens: list[Picture] = field(default_factory=list)
    zip_files: list[ZipFile] = field(default_factory=list)
    trle_id: int = 0
    trcustoms_id: int = 0
    body: str = ""
    walkthrough: str = ""


@dataclass
class TrcustomsLevel:
    """trcustoms.org record data."""

    trcustoms_id: int = 0
    authors: list[str] = field(default_factory=list)
    title: str = ""
    tags: list[str] = field(default_factory=list)
    genres: list[str] = field(default_factory=list)
    type: str = ""
    release: str = ""
    difficulty: str = ""
    duration: str = ""
    cover: str = ""
    cover_md5sum: str = ""


@dataclass
class TrcustomsPage:
    """trcustoms.org page."""

    current_page: int = 0
    total_pages: int = 0
    records_total: int = 0
    levels: list[TrcustomsLevel] = field(default_factory=list)
