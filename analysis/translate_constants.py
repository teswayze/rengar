import re
from dataclasses import dataclass, field

import numpy as np


@dataclass
class Constants:
    ints: dict[str, int] = field(default_factory=dict)
    board_arrays: dict[str, np.ndarray] = field(default_factory=dict)


def translate_constants(file_path: str):
    with open(file_path) as f:
        text = f.read()

    output = Constants()

    int_pattern = r'const int (\S+) = ([0-9]+);'
    chars_searched = 0
    while (match := re.search(int_pattern, text[chars_searched:])):
        const_name, const_value = match.groups()
        output.ints[const_name] = int(const_value)
        chars_searched += match.span()[1]

    array_pattern = r'const std::array<int, 64> (.+?) = \{\s*' + r'(-?[0-9]+),\s*' * 64 + r'\};'
    while (match := re.search(array_pattern, text[chars_searched:])):
        const_name = match.group(1)
        output.board_arrays[const_name] = np.array([int(match.group(2+i)) for i in range(64)])
        chars_searched += match.span()[1]

    return output
