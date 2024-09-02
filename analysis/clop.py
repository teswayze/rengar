import numpy as np


def quadratic_regression(x: np.ndarray, y: np.ndarray, w: np.ndarray) -> tuple[float, float, float]:
    n, = x.shape
    assert n > 0
    assert y.shape == (n,)
    assert w.shape == (n,)

    design_matrix = np.concatenate([x ** 2 * w ** 0.5, x * w ** 0.5, w ** 0.5]).reshape((3, n)).T
    a, b, c = np.linalg.lstsq(design_matrix, y * w ** 0.5, rcond=None)[0]
    if a > 0:
        a = 0.0
        b, c = np.linalg.lstsq(design_matrix[:, 1:], y * w ** 0.5, rcond=None)[0]
    return a, b, c


class ClopSampler:
    def __init__(self, parameter_bounds: dict[str, dict[str, int]]):
        self._parameter_bounds = parameter_bounds
        
        self._trial_parameters = []
        self._trial_labels = []
        self._reset_param_weights()

    def _reset_param_weights(self):
        self._parameter_weights = {k: np.ones(v['max'] - v['min'] + 1) for k, v in self._parameter_bounds.items()}

    def _compute_trial_weights(self) -> np.ndarray:
        w = np.ones(len(self._trial_parameters))
        for k, v in self._parameter_weights.items():
            w *= v[[params[k] - self._parameter_bounds[k]['min'] for params in self._trial_parameters]]
        return w

    def record_trial_and_update(self, parameters: dict[str, int], label: int):
        self._trial_parameters.append(parameters)
        self._trial_labels.append(label)

        self._reset_param_weights()
        w_sum = len(self._trial_labels)
        while True:
            w_trials = self._compute_trial_weights()
            for k, v in self._parameter_bounds.items():
                pass  # TODO
