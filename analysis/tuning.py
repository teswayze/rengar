from argparse import ArgumentParser
import random

import optuna
import torch

from architecture import initialize_rengar_network, clamp_weights
from data import load_data_and_labels
from score import LossFunction, sorted_eval_score


class StudyRunner:
    def __init__(self):
        self.x_test, self.y_test = load_data_and_labels('selfplay_data/startpos_171.rg')

    def objective(self, trial: optuna.Trial) -> float:
        net = initialize_rengar_network(
            s1=trial.suggest_int('s1', 16, 256, log=True),
            s2=trial.suggest_int('s2', 16, 256, log=True),
            s3=trial.suggest_int('s3', 16, 256, log=True),
            l1_dropout=trial.suggest_float('l1_dropout', low=0.0, high=0.5), 
            l2_dropout=trial.suggest_float('l2_dropout', low=0.0, high=0.5), 
        )
        optimizer = torch.optim.Adam(
            params=net.parameters(),
            lr=trial.suggest_float('lr', low=0.02, high=0.1, log=True),
        )
        loss_fn = LossFunction(p=1.0, q=1.0)

        train_indices = list(range(324))
        train_indices.remove(171)
        random.shuffle(train_indices)

        best_score = -1.0
        num_fails = 0
        for epoch_no in range(1, 51):
            x, y = load_data_and_labels(f'selfplay_data/startpos_{train_indices[epoch_no - 1]}.rg')
            
            net.train()
            optimizer.zero_grad()
            loss = loss_fn(net(x), y)
            loss.backward()
            optimizer.step()
            clamp_weights(net)
            
            net.eval()
            score = sorted_eval_score(net(self.x_test), self.y_test)
            if score > best_score:
                best_score = score
                num_fails = 0
            else:
                num_fails += 1
                if num_fails == 10:
                    return score

            trial.report(score, epoch_no)
            if epoch_no % 10 == 0 and trial.should_prune():
                return score

        return score


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--study-path', required=True)
    parser.add_argument('--study-name', required=True)
    parser.add_argument('--num-trials', type=int, required=True)
    options = parser.parse_args()

    study = optuna.create_study(
        direction="maximize", 
        storage=f"sqlite:///{options.study_path}", 
        study_name=options.study_name,
        load_if_exists=True,
    )
    runner = StudyRunner()
    study.optimize(runner.objective, n_trials=options.num_trials)
