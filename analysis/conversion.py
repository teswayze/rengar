from argparse import ArgumentParser
import numpy as np
import torch


def array_str_1d(array: np.ndarray) -> str:
    return '{' + ', '.join(map('{:7.4f}'.format, array)) + '}'


def declare_array_of_vector(array: np.ndarray, type_name: str, variable_name: str) -> str:
    assert array.ndim == 2
    array_len = array.shape[0]
    prefix = f'const std::array<{type_name}, {array_len}> {variable_name} = ' + '{\n\t' + type_name
    arrs_1d = [array_str_1d(array[i]) for i in range(array_len)]
    joiner = f',\n\t{type_name}'
    suffix = '\n};\n\n'
    return prefix + joiner.join(arrs_1d) + suffix


def pst_weight_file_text(state_dict: dict[str, torch.Tensor]) -> str:
    text = '# include "pst.hpp"\n\n'
    for weight_kind in ['fs', 'va', 'ha', 'ra']:
        text += declare_array_of_vector(
            state_dict[f'0.pst_{weight_kind}.weight'].numpy()[:-1], 
            type_name='Eigen::Vector<float, 32>', 
            variable_name=f'w_l0_pst_{weight_kind}',
        )
    return text


def declare_vector(array: np.ndarray, type_name: str, variable_name: str) -> str:
    assert array.ndim == 1
    return f'const {type_name} {variable_name} = {array_str_1d(array)};\n\n'


def declare_matrix(array: np.ndarray, type_name: str, variable_name: str) -> str:
    assert array.ndim == 2
    prefix = f'const {type_name} {variable_name} ' + '{\n\t'
    arrs_1d = [array_str_1d(array[i]) for i in range(array.shape[0])]
    joiner = ',\n\t'
    suffix = '\n};\n\n'
    return prefix + joiner.join(arrs_1d) + suffix


def hidden_weight_file_text(state_dict: dict[str, torch.Tensor]) -> str:
    text = '# include "hidden.hpp"\n\n'
    text += declare_vector(state_dict['0.tempo_va.weight'].numpy().squeeze(), 'Eigen::Vector<float, 32>',  'w_l0_tempo_va')

    text += declare_vector(state_dict['3.fs.bias'].numpy(), 'Eigen::Vector4f',  'w_l1_fs_bias')
    for l1_w_kind in ['fs', 'absva', 'absha', 'absra', 'va', 'fsxva', 'haxra']:
        text += declare_matrix(
            state_dict[f'3.{l1_w_kind}.weight'].numpy(), 
            'Eigen::Matrix<float, 4, 32, Eigen::RowMajor>',
            f'w_l1_{l1_w_kind}',
        )
    
    for l2_w_kind in ['va', 'fsxva']:
        text += declare_vector(state_dict[f'6.{l2_w_kind}.weight'].numpy().squeeze(), 'Eigen::Vector4f', f'w_l2_{l2_w_kind}')
    
    return text


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--net-name', required=True)
    options = parser.parse_args()

    parameters = torch.load(f'networks/{options.net_name}.pt')

    with open('src/weights/pst.cpp', 'w') as f:
        f.write(pst_weight_file_text(parameters))

    with open('src/weights/hidden.cpp', 'w') as f:
        f.write(hidden_weight_file_text(parameters))
