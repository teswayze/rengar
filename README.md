# Rengar
A chess engine using a small neural network written in C++. [Play me on lichess!](https://lichess.org/@/MisterRengar)

# Network architecture

The engine's network does 32-bit floating point arithmetic using Eigen. The network was trained using pytorch. 

## Symmetry properties
Inspired by large ELO gains from enforcing symmetry in the classical evaluation (see #9 and #17), Rengar's network has the following properties:

1. Flipping the board vertically, swapping colors, and changing the side to move negates the evaluation
2. Flipping the board horizontally maintains the same evaluation
3. Rotating the board 180 degrees, swapping colors, and changing the side to move negates the evaluation

Note that any two of these properties imply the third. Vertical reflection, horizontal reflection, 180 degree rotation, and the identity form a symmetry group isomorphic to `Z_2 X Z_2`.

Several other neural engines enforce verical symmetry (1) by duplicating the network's first hidden layer from the other player's perspective. I am not aware of any other neural engines that enforce horizontal symmetry (2).

### Decomposing the input space

As usual, the input layer of the network is constructed by embedding the board in a 736-dimensional space as an input vector of ones and zeros, where a 1 in a certain dimension might mean something like "there is a white pawn on f2". The symmetry group acts on this vector space by permuting the dimensions.

Rengar enforces symmetry by rotating that space so it can be decomposed into four 184-dimensional subspaces:

- `full_symm` is invariant under all 3 symmetry operations
- `vert_asym` is fixed by horizonal flip, but negated by vertical flip and 180 degree rotation
- `horz_asym` is fixed by vertical flip, but negated by horizontal flip and 180 degree rotation
- `rotl_asym` is fixed by 180 degree rotation, but negated by both vertical and horizontal flips

Note that the final evaluation should lie in the `vert_asym` space.

A purely linear evaluation function would have 184 parameters and could not use the other three spaces at all. A white pawn on c2 or f2 must get the same weight, while a black pawn on c7 or f7 must get the opposite weight. So we must use nonlinear activation to get the other spaces involved.

### Product pooling

The primary nonlinearity in Rengar's network comes from multiplication. 

Multiplying two expressions with symmetry group guarantees enforces guarantees on the product. In Rengar, I use the following:

- Multiplying a `full_symm` expression by a `vert_asym` expression yields a `vert_asym` expression
- Multiplying a `horz_asym` expression by a `rotl_asym` expression yields a `vert_asym` expression

The former allows you to taper the evaluation from the middlegame to the endgame, from open to closed positions, or to reduce the evaluation for OCB endgames. The latter allows you to learn king safety in SSC or OSC positions, judge who is winning a pawn race, or discern between good and bad bishops. Both products can be used to give a bishop pair advantage. 

## Architecture

### Input layer

TODO

# Search techniques

- Alpha beta pruning
- Iterative deepening
- Quiescence search
- Transposition tables
- Reverse futility pruning
- Null move reductions
- Late move reductions
- Killer heuristic
- History heuristic
- Guard heruistic

# Supported commands

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
- `eval` will show the evaluation of the current position, as well as the values of the hidden latera of the network
- `lookup` probes the hash table for the current position, showing the depth, score, and move if an entry is available
- `moveorder` will show the current bonuses given to moves to certain squares according to the history heuristic

## Build targets
- `release`, the default, will compile the engine binary
- `unit` will compile and run unit tests via doctest
- `perft` will run validation checks on move generation code
