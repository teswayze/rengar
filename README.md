# Rengar
A chess engine with a simple evaluation function written in C++. [Play me on lichess!](https://lichess.org/@/MisterRengar)

## Features

### Search techniques
- Alpha beta pruning
- Iterative deepening
- Quiescence search
- Transposition tables
- Null move reductions
- Late move reductions

### Move ordering
- Killer heuristic
- Guard heruistic
- Piece square tables

### Evaluation
- Piece square tables
- Mobility
- Tapered evaluation
- Tempo bonus

## Make targets
- `release`, the default, will compile the engine binary
- `tune` will optimize move order parameters for accuracy on a specified set of games
- `unit` will compile and run unit tests via doctest
- `perft` will run validation checks on move generation code

## Self-play
To compare across different versions:
- Compile the engine from different branches
- Run a tournament with `analysis/selfplay.py`, specifying the opening book, the time control, and the list of branches
