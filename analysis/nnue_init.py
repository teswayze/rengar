import numpy as np


def format_vector(arr: np.ndarray) -> str:
    assert arr.shape == (16,)
    return 'vector_set(' + ', '.join(map(str, arr)) + ')'


def print_random_vector(var_name: str, max_abs_val: int) -> str:
    arr = np.random.randint(-max_abs_val, max_abs_val + 1, size=16)
    return 'Vector ' + var_name + ' = ' + format_vector(arr) + ';\n\n'


def print_random_vector_array(var_name: str, max_abs_val: int, length: int):
    arr = np.random.randint(-max_abs_val, max_abs_val + 1, size=(length, 16))
    output = 'std::array<Vector, ' + str(length) + '> ' + var_name + ' = {\n'
    for i in range(length):
        output += '\t' + format_vector(arr[i]) + ',\n'
    output += '};\n\n'
    return output


if __name__ == '__main__':
    np.random.seed(0)

    text = '# include <array>\n# include "linalg.hpp"\n\n'

    text += print_random_vector_array('w_l0_pst_fs', 63, 184)
    text += print_random_vector_array('w_l0_pst_va', 63, 184)
    text += print_random_vector_array('w_l0_pst_ha', 63, 184)
    text += print_random_vector_array('w_l0_pst_ra', 63, 184)
    text += print_random_vector('w_l0_tempo_va', 63)

    text += print_random_vector('w_l1_bias_fs', 63)
    text += print_random_vector_array('w_l1_fs_fs', 63, 16)
    text += print_random_vector_array('w_l1_absva_fs', 63, 16)
    text += print_random_vector_array('w_l1_absha_fs', 63, 16)
    text += print_random_vector_array('w_l1_absra_fs', 63, 16)
    text += print_random_vector_array('w_l1_va_va', 63, 16)
    text += print_random_vector_array('w_l1_fsxva_va', 63, 16)
    text += print_random_vector_array('w_l1_haxra_va', 63, 16)

    text += print_random_vector('w_l2_va', 63)
    text += print_random_vector('w_l2_fsxva', 63)

    with open('src/nnue_weights.cpp', 'w') as f:
        f.write(text)
