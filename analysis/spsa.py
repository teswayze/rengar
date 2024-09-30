from dataclasses import dataclass

import numpy as np


@dataclass
class SpsaTrial:
    params_pos: dict[str, float]
    params_neg: dict[str, float]
    perturbation: np.ndarray


class SpsaOptimizer:
    def __init__(
        self, 
        initial_params: dict[str, float],
        initial_step: dict[str, float],
        alpha: float = 0.68,
        gamma: float = 0.17,
        starting_n: int = 10,
    ):
        self.param_names = list(initial_params.keys())
        self.param_values = np.array([initial_params[k] for k in self.param_names])

        self.alpha = alpha
        self.gamma = gamma
        self.trial_no = starting_n
        self.base_step = np.array([initial_step[k] for k in self.param_names]) * starting_n ** gamma

    def generate_trial(self) -> SpsaTrial:
        coin_flips = np.random.randint(0, 2, len(self.param_names)) * 2 - 1
        cn_deltan = self.trial_no ** -self.gamma * self.base_step * coin_flips
        an = self.trial_no ** -self.alpha * self.base_step ** 2
        trial = SpsaTrial(
            params_pos=dict(zip(self.param_names, self.param_values + cn_deltan)),
            params_neg=dict(zip(self.param_names, self.param_values - cn_deltan)),
            perturbation=an / (2 * cn_deltan),
        )
        self.trial_no += 1
        return trial

    def record_trial(self, trial: SpsaTrial, result: float):
        self.param_values += trial.perturbation * result

    def current_params(self) -> dict[str, float]:
        return dict(zip(self.param_names, self.param_values))
