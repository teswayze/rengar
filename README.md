# Rengar
A chess engine with a simple evaluation function written in C++. [Play me on lichess!](https://lichess.org/@/MisterRengar)

## Supported commands

Rengar supports the following UCI commands:
- `go` will start the search. Rengar supports the following search options:
    - `depth`: Search to the specified depth
    - `nodes`: Search for at least this many nodes and stop after finishing at the current depth
    - `movetime`: Search for at least this amount of time in milliseconds and stop after finishing at the current depth
    - `wtime` and `btime`: The time remaining in the game. Currently Rengar will search at greater depths until it has used at least 1/64 of it's remaining time and then pick the best move. The opponent time is ignored.
    - `winc`, `binc`, and `movestogo`: Supported but ignored
- `position` will allow you to set the root board state. Keeping with UCI you can specify either:
    - `position startpos` to get the starting position
    - `position fen <position in FEN notation>` to specify an arbitrary position
    - Adding `moves <move1> <move2> ...` to the end of the command will make moves from the specified position
- `setoption` supports the following configurations:
    - `setoption name hashbits value <n>` sets the hash table to a size of 2^n. The default value is 24, so as each entry uses 16 bytes, this would allocate 256MB for the hash table. You can verify this with the `hashstats` command.
- `ucinewgame` will wipe the hash table
- `debug on` will show an info log after a search to each depth; `debug off` only shows the log before terminating the search
- `uci` shows information about the engine
- `isready` if the engine is ready for commands it will respond with `readyok`
- `quit` will terminate the program
Rengar also supports the following commands which may be useful for debugging
- You can use the `moves` command without specifying `position` first to make moves from the current board state
- `show` will print the current board state from white's perspective, using capital letters for white pieces and lowercase letters for black ones
- `legal` will show the list of legal moves in the order that Rengar would consider them in a search
- `forcing` will show the moves that would be considered in quiescence search
- `hashstats` will print some information about the hash table, including the size as well as the number of hits, puts, and misses since it has been initialized
- `searchstats` will print the counts of various node types encountered during the most recent search
- `eval` will show some components of the static evaluation function from the current position

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
