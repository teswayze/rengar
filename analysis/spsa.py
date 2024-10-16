from dataclasses import dataclass

import numpy as np
import pandas as pd


@dataclass
class SpsaTrial:
    params_pos: dict[str, float]
    params_neg: dict[str, float]
    perturbation: np.ndarray


class SimpleOptimizer:
    def __init__(
        self, 
        params: dict[str, dict[str, float]],
        max_ratio: float = 1.25,
        learning_rate: float = 0.1,
    ):
        df = pd.DataFrame(params)
        self.param_names = list(df.columns)
        self.param_values = df.loc['default'].values
        self.param_min = df.loc['min'].values
        self.param_max = df.loc['max'].values

        self.step = max_ratio
        self.lr = learning_rate

    def generate_trial(self) -> SpsaTrial:
        diff = self.step ** np.random.uniform(-1, 1, size=len(self.param_names))
        trial = SpsaTrial(
            params_pos=dict(zip(self.param_names, self.param_values * diff)),
            params_neg=dict(zip(self.param_names, self.param_values / diff)),
            perturbation=diff ** self.lr,
        )
        return trial

    def record_trial(self, trial: SpsaTrial, result: float):
        self.param_values = np.clip(self.param_values * trial.perturbation ** result, self.param_min, self.param_max)

    def current_params(self) -> dict[str, float]:
        return dict(zip(self.param_names, self.param_values))
