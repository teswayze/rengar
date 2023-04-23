import numpy as np

def left_pad(string: str, length: int):
    return ' '*(length - len(string)) + string


def print_nicely(board_array: np.array):
    max_length = max(len(str(i)) for i in board_array.flatten())
    for i in board_array.shape[0]:
        sub_arr = board_array[i]
        print('\t' + ', '.join(left_pad(str(j), max_length) for j in sub_arr) + ',')
