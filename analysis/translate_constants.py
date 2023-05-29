import re
from dataclasses import dataclass, field
from pathlib import Path

import numpy as np

from utils import root_dir


@dataclass
class Constants:
    _ints: dict[str, int] = field(default_factory=dict)
    _board_arrays: dict[str, np.ndarray] = field(default_factory=dict)

    def __getattr__(self, name: str):
        if name in self._ints:
            return self._ints[name]
        if name in self._board_arrays:
            return self._board_arrays[name]
        raise AttributeError(name)


def translate_constants(file_path: Path) -> Constants:
    with open(file_path) as f:
        text = f.read()
    
    ints = {}
    int_pattern = r'const int (\S+) = (-?[0-9]+);'
    chars_searched = 0
    while (match := re.search(int_pattern, text[chars_searched:])):
        const_name, const_value = match.groups()
        ints[const_name] = int(const_value)
        chars_searched += match.span()[1]

    board_arrays = {}
    array_pattern = r'const std::array<int, 64> (.+?) = \{\s*' + r'(-?[0-9]+),\s*' * 64 + r'\};'
    while (match := re.search(array_pattern, text[chars_searched:])):
        const_name = match.group(1)
        board_arrays[const_name] = np.array([int(match.group(2+i)) for i in range(64)]).reshape((8, 8))
        chars_searched += match.span()[1]

    return Constants(ints, board_arrays)

pst = translate_constants(root_dir() / 'includes/pst.hpp')
eval_ = translate_constants(root_dir() / 'src/eval.cpp')
endgames = translate_constants(root_dir() / 'src/endgames.cpp')
