// Utilities for bordered board enumeration.
// An implicit border at height 0 surrounds the board, so edge cells are
// constrained to [-2, 2]. The most extreme board is concentric rings of
// decreasing depth. Zeroing normalizes by the minimum edge-cell value.

#ifndef BOARD_GAME_WITH_BORDER_UTILS_H
#define BOARD_GAME_WITH_BORDER_UTILS_H

#include <unordered_set>

using Board = std::vector<std::vector<int>>;

// Height-offset normalize: subtract the minimum edge-cell value from all cells.
Board
zero_board(const Board& board);

// Returns the minimum edge-cell value (used for zero-without-copy hashing).
int
compute_zero_offset(const Board& board);

// Generates the most extreme (lowest) board via concentric rings of
// (min_axis+1)*-2. This is the BFS starting point for bordered boards.
Board
drop_board(Board board);

// Tries to increment any single cell, checking adjacency (<=2 diff) and
// the border constraint (edge cells <= 2). No board_min/board_max needed
// because the border constraint implicitly caps growth.
std::optional<Board>
increment_board(Board board);

int
calculate_rounds(const Board& dropped_board, bool print_boards = false);

// Combination dedup: canonical key = lexicographic minimum of all rotation
// hashes.
std::tuple<bool, std::string>
check_board_is_new_combo(Board board,
                         const std::unordered_set<std::string>& results);

// Permutation dedup: only zeroed hash, no rotation equivalence.
std::tuple<bool, std::string>
check_board_is_new_perm(Board board,
                        const std::unordered_set<std::string>& results);

// Single-threaded increment generation (no board_min/board_max params needed).
std::unordered_map<std::string, Board>
generate_all_board_increments_combo(Board board);

std::unordered_map<std::string, Board>
generate_all_board_increments_perm(Board board);

// Multi-threaded wrappers for parallel chunk processing.
std::vector<std::pair<Board, std::string>>
generate_boards_mp_combo(const std::vector<Board>& split_increments);

std::vector<std::pair<Board, std::string>>
generate_boards_mp_perm(const std::vector<Board>& split_increments);

#endif