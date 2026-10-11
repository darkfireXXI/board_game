// Utilities for borderless board enumeration.
// No edge constraint — cells can grow arbitrarily. The most extreme board is a
// diagonal ramp: board[r][c] = (r+c)*2. Zeroing normalizes by global minimum.

#ifndef BOARD_GAME_NO_BORDER_UTILS_H
#define BOARD_GAME_NO_BORDER_UTILS_H

#include <unordered_set>

using Board = std::vector<std::vector<int>>;

// Height-offset normalize: subtract the global minimum from all cells.
Board
zero_board(const Board& board);

// Returns the global minimum cell value (used for zero-without-copy hashing).
int
compute_zero_offset(const Board& board);

// Generates the most extreme (lowest) board: a diagonal ramp of (r+c)*2.
// This is the BFS starting point — all valid boards are reachable from here.
Board
find_most_extreme_board(Board board);

// Tries to increment any single cell by +1, checking adjacency (<=2 diff)
// and that the cell doesn't exceed board_max. Returns the first valid
// increment found, or nullopt if the board is fully saturated.
std::optional<Board>
increment_board(Board board, const int& board_min, const int& board_max);

// Counts BFS rounds by repeatedly incrementing from the extreme board until
// no more increments are possible.
int
calculate_rounds(const Board& most_extreme_board,
                 const int& board_min,
                 const int& board_max,
                 bool print_boards = false);

// Combination dedup: hashes all rotations (4 for square, 2 for rectangular),
// uses the lexicographic minimum as the canonical key.
std::tuple<bool, std::string>
check_board_is_new_combo(Board board,
                         const std::unordered_set<std::string>& results);

// Permutation dedup: only zeroed hash, no rotation equivalence.
std::tuple<bool, std::string>
check_board_is_new_perm(Board board,
                        const std::unordered_set<std::string>& results);

// Single-threaded: generates all valid +1 increments of a board, deduped
// within the batch. Returns map of canonical_hash -> board.
std::unordered_map<std::string, Board>
generate_all_board_increments_combo(Board board,
                                    const int& board_min,
                                    const int& board_max);

std::unordered_map<std::string, Board>
generate_all_board_increments_perm(Board board,
                                   const int& board_min,
                                   const int& board_max);

// Multi-threaded wrappers: process a chunk of boards and collect all their
// increments as (board, canonical_hash) pairs for cross-checking.
std::vector<std::pair<Board, std::string>>
generate_boards_mp_combo(const std::vector<Board>& split_increments,
                         const int& board_min,
                         const int& board_max);

std::vector<std::pair<Board, std::string>>
generate_boards_mp_perm(const std::vector<Board>& split_increments,
                        const int& board_min,
                        const int& board_max);

#endif