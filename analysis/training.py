from argparse import ArgumentParser
from dataclasses import dataclass
import random

import torch
from tqdm import tqdm

from architecture import initialize_rengar_network, clamp_weights
from data import load_data_and_labels
from score import LossFunction


@dataclass
class NetTrainer:
    net: torch.nn.Module
    optimizer: torch.optim.Optimizer
    loss_fn: torch.nn.Module

    def run_epoch(self, data_idx: int):
        x, y = load_data_and_labels(f'selfplay_data/startpos_{data_idx}.rg')
        self.net.train()
        self.optimizer.zero_grad()
        loss = self.loss_fn(self.net(x), y)
        loss.backward()
        self.optimizer.step()
        clamp_weights(self.net)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--output-file', required=True)
    parser.add_argument('--lr', type=float, default=0.04)
    parser.add_argument('--dropout', type=float, default=0.35)
    parser.add_argument('--num-epochs', type=int, default=50)
    options = parser.parse_args()

    train_indices = list(range(324))
    train_indices.remove(171)
    random.shuffle(train_indices)

    net = initialize_rengar_network(16, 16, 16, options.dropout, options.dropout)
    optimizer = torch.optim.Adam(params=net.parameters(), lr=options.lr)
    loss_fn = LossFunction(p=1.0, q=1.0)
    trainer = NetTrainer(net, optimizer, loss_fn)

    for ix in tqdm(train_indices[:options.num_epochs]):
        trainer.run_epoch(ix)

    torch.save(net.state_dict(), options.output_file)
