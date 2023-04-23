import numpy as np

def left_pad(string: str, length: int):
    return ' '*(length - len(string)) + string


def print_cpp_2d_array_code(name: str, board_array: np.array):
    assert board_array.shape == (8, 8)
    max_length = max(len(str(i)) for i in board_array.flatten())
    print('const std::array<int, 64> ' + name + ' = {')
    for i in range(8):
        sub_arr = board_array[i]
        print('\t' + ', '.join(left_pad(str(j), max_length) for j in sub_arr) + ',')
    print('};')


def print_cpp_constant_code(name: str, value: int):
    print(f'const int {name} = {value};')
