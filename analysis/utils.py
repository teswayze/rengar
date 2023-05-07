import numpy as np

def left_pad(string: str, length: int):
    return ' '*(length - len(string)) + string


def print_cpp_2d_array_code(name: str, arr: np.array):
    if arr.ndim == 1:
        arr = arr.reshape((1, arr.size))
    assert arr.ndim == 2
    max_length = max(len(str(i)) for i in arr.flatten())
    print('const std::array<int, ' + str(arr.size) + '> ' + name + ' = {')
    for i in range(arr.shape[0]):
        sub_arr = arr[i]
        print('\t' + ', '.join(left_pad(str(j), max_length) for j in sub_arr) + ',')
    print('};')


def print_cpp_constant_code(name: str, value: int):
    print(f'const int {name} = {value};')
