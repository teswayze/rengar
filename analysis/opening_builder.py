from __future__ import annotations

import json
from argparse import ArgumentParser
from dataclasses import dataclass, field
from queue import PriorityQueue
from time import sleep
from urllib.request import urlopen
from urllib.error import HTTPError

from chess import Board, Move

MASTERS_URL = 'https://explorer.lichess.ovh/masters'


def get_results_from_url(url: str) -> dict:
    while True:
        try:
            print(url)
            return json.loads(urlopen(url).read())
        except HTTPError as e:
            if e.code != 429:
                raise e
            print('Waiting 60 seconds')
            sleep(60)


@dataclass
class BookNode:
    fen: str
    name: str
    prev_moves: list[str]
    move_freq: dict[str, int]

    def create_child(self, move: str, node_library: dict[str, BookNode]) -> BookNode | None:
        b = Board(self.fen)
        move_uci = Move.from_uci(move)
        move_san = b.san(move_uci)
        b.push(move_uci)
        new_fen = b.fen()

        if new_fen.split()[0] in node_library:
            return None
        
        new_move_list = self.prev_moves + [move]
        url = MASTERS_URL + '?play=' + ','.join(new_move_list)
        result = get_results_from_url(url)

        if result['opening'] is not None:
            new_name = result['opening']['name'].replace(' ', '-').replace("'", "").replace(":", "").replace(",", "")
        else:
            new_name = self.name

        new_node = BookNode(
            fen=new_fen,
            name=new_name,
            prev_moves=new_move_list,
            move_freq={v['uci']: v['white'] + v['draws'] + v['black'] for v in result['moves']},
        )
        library[new_fen.split()[0]] = new_node
        return new_node

    def queue_potential_children(self, queue: PriorityQueue):
        for k, v in self.move_freq.items():
            queue.put((-v, k, self))

    def any_potential_child_has_been_seen(self, library: dict[str, BookNode]) -> bool:
        b = Board(self.fen)
        for move in self.move_freq.keys():
            b.push(Move.from_uci(move))
            if b.fen().split()[0] in library:
                return True
            b.pop()
        return False

    def __lt__(self, other):
        if self.fen.split()[0] == other.fen.split()[0]:
            raise RuntimeError(f"Should not happen. {self = } {other = }")
        return self.fen < other.fen


def create_root() -> BookNode:
    result = get_results_from_url(MASTERS_URL)
    return BookNode(
        fen=Board().fen(),
        name='Starting-Position',
        prev_moves=[],
        move_freq={v['uci']: v['white'] + v['draws'] + v['black'] for v in result['moves']},
    )


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--num-openings', required=True, type=int)
    options = parser.parse_args()
    
    root = create_root()
    queue = PriorityQueue()
    root.queue_potential_children(queue)
    library = {root.fen.split()[0]: root}
    leaves = {root.fen.split()[0]: root}

    while(len(leaves) < options.num_openings):
        while(len(leaves) < options.num_openings):
            _, move, node = queue.get()

            if node.fen.split()[0] in leaves:
                leaves.pop(node.fen.split()[0])

            child = node.create_child(move, library)
            if child is not None:
                leaves[child.fen.split()[0]] = child
                child.queue_potential_children(queue)

        print("Checking for possible transpositions...")
        prune_count = 0
        for k, v in list(leaves.items()):
            moveset = set(v.prev_moves)
            for other in list(leaves.values()):
                if moveset < set(other.prev_moves):
                    leaves.pop(k)
                    prune_count += 1
                    break
        print(f"Removed {prune_count} leaves")

    for x in sorted([' '.join(x.prev_moves) + ' | ' + x.name for x in leaves.values()]):
        print(x)

