from argparse import ArgumentParser
from dataclasses import dataclass, field
import random

import torch
from tqdm import tqdm

from architecture import initialize_rengar_network, clamp_weights
from data import EvaluationInputData, load_data_and_labels
from score import LossFunction, sorted_eval_score


@dataclass
class NetTrainer:
    net: torch.nn.Module
    optimizer: torch.optim.Optimizer
    scheduler: torch.optim.lr_scheduler.ReduceLROnPlateau
    train_indices: list[int]
    lr_stop: float

    num_batches: int = field(init=False)
    loss_fn: torch.nn.Module = field(init=False)
    validation_x: EvaluationInputData = field(init=False)
    validation_y: torch.FloatTensor = field(init=False)

    def __post_init__(self):
        self.num_batches = 0
        self.loss_fn = LossFunction(p=1.0, q=1.0)
        self.validation_x, self.validation_y = load_data_and_labels('selfplay_data/startpos_171.rg')
        print('num_batches,score,learning_rate,eval_max,eval_stdev')

    def run_batch(self) -> bool:
        data_idx = self.train_indices[self.num_batches % len(self.train_indices)]
        x, y = load_data_and_labels(f'selfplay_data/startpos_{data_idx}.rg')
        self.net.train()
        self.optimizer.zero_grad()
        loss = self.loss_fn(self.net(x), y)
        loss.backward()
        self.optimizer.step()
        clamp_weights(self.net)

        self.num_batches += 1
        if self.num_batches % 10 == 0:
            evaluation = self.net(self.validation_x)
            score = sorted_eval_score(evaluation, self.validation_y)
            self.scheduler.step(score)
            last_lr, = self.scheduler.get_last_lr()
            print(f'{self.num_batches},{round(score, 4)},{last_lr},'
                  f'{round(torch.max(torch.abs(evaluation)).item(), 2)},{round(torch.std(evaluation).item(), 2)}')
        
            return last_lr <= self.lr_stop
        
        return False


# Recommend: --lr-start 0.04 --decay 5e-5 --momentum 0.6
if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--input-file')
    parser.add_argument('--output-file', required=True)
    parser.add_argument('--lr-start', type=float, required=True)
    parser.add_argument('--lr-stop', type=float, default=1e-4)
    parser.add_argument('--decay', type=float, default=0.0)
    parser.add_argument('--momentum', type=float, default=0.9)
    parser.add_argument('--dropout', type=float, default=0.0)
    options = parser.parse_args()

    net = initialize_rengar_network(16, 16, 16, options.dropout, options.dropout)
    if options.input_file is not None:
        net.load_state_dict(torch.load(options.input_file))

    optimizer = torch.optim.Adam(
        params=net.parameters(), 
        lr=options.lr_start, 
        weight_decay=options.decay,
        betas=(options.momentum, 0.999),
    )

    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
        optimizer=optimizer,
        mode='max',
        patience=3,
        factor=0.5,
    )

    train_indices = list(range(324))
    train_indices.remove(171)
    random.shuffle(train_indices)

    trainer = NetTrainer(net, optimizer, scheduler, train_indices, options.lr_stop)

    done = False
    while not done:
        done = trainer.run_batch()

    torch.save(net.state_dict(), options.output_file)
