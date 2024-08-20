import numpy as np
import torch


def quantized_array_str_1d(array: np.ndarray, scaling_factor: float) -> str:
    def round_and_str(elem: float) -> str:
        return str(int(round(elem * scaling_factor)))
    
    return '{' + ', '.join(map(round_and_str, array)) + '}'


def quantized_array_str_2d(array: np.ndarray, scaling_factor: float, type_name: str, variable_name: str) -> str:
    prefix = 'const std::array ' + variable_name + ' = {\n\t' + type_name + '('
    arrs_1d = [quantized_array_str_1d(array[i], scaling_factor) for i in range(array.shape[0])]
    joiner = '),\n\t' + type_name + '('
    suffix = ')\n};\n\n'
    return prefix + joiner.join(arrs_1d) + suffix


def pst_weight_file_text(state_dict: dict[str, torch.Tensor]) -> str:
    text = '# include "pst.hpp"\n\n'
    for weight_kind in ['pst_fs', 'pst_va', 'pst_ha', 'pst_ra']:
        text += quantized_array_str_2d(
            state_dict[f'0.{weight_kind}.weight'].numpy()[:-1], 
            scaling_factor=128.0, 
            type_name='ThirtyTwoInt16s', 
            variable_name=weight_kind,
        )
    return text


if __name__ == '__main__':
    parameters = torch.load('networks/l1-32-l2-4.pt')
    with open('src/weights/pst.cpp', 'w') as f:
        f.write(pst_weight_file_text(parameters))
