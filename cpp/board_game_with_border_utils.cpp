// With-border variant utilities.
// Key difference from no-border: edge cells must be within 2 of the implicit
// border at height 0, so zeroing uses the min edge value (not global min).

#include "board_game_with_border_utils.h"
#include "board_game_counting_utils.h"

// Normalize by the minimum edge-cell value. Only edge cells matter because
// the implicit border constrains them, and interior cells are transitively
// bounded by adjacency.
Board
zero_board(const Board& board)
{
  int rows = board.size();
  int columns = board[0].size();
  int min_edge = std::numeric_limits<int>::max();

  // Top/bottom rows: check all cells. Middle rows: only left/right edges.
  for (size_t r = 0; r < rows; ++r) {
    if (r == 0 || r == rows - 1) {
      for (size_t c = 0; c < columns; ++c) {
        min_edge = std::min(min_edge, board[r][c]);
      }
    } else {
      min_edge = std::min(min_edge, board[r][0]);
      min_edge = std::min(min_edge, board[r][columns - 1]);
    }
  }

  Board zeroed_board(board);
  for (size_t r = 0; r < rows; ++r) {
    for (size_t c = 0; c < columns; ++c) {
      zeroed_board[r][c] -= min_edge;
    }
  }

  return zeroed_board;
}

// Same edge-only minimum, but returns the offset value directly for use
// with the zero-without-copy hash functions.
int
compute_zero_offset(const Board& board)
{
  int rows = board.size();
  int columns = board[0].size();
  int min_edge = std::numeric_limits<int>::max();
  for (int r = 0; r < rows; ++r) {
    if (r == 0 || r == rows - 1) {
      for (int c = 0; c < columns; ++c)
        min_edge = std::min(min_edge, board[r][c]);
    } else {
      min_edge = std::min(min_edge, board[r][0]);
      min_edge = std::min(min_edge, board[r][columns - 1]);
    }
  }
  return min_edge;
}

// Concentric-ring extreme board. Each ring is 2 lower than the one outside it:
// edge cells = -2, next ring in = -4, etc. Uses 4-fold symmetry to fill all
// four quadrants simultaneously. This is the deepest possible bordered board.
Board
drop_board(Board board)
{
  int rows = board.size();
  int columns = board[0].size();
  int mid_row = (rows / 2) - (rows % 2 == 0);
  int mid_col = (columns / 2) - (columns % 2 == 0);

  for (int r = 0; r <= mid_row; ++r) {
    for (int c = 0; c <= mid_col; ++c) {
      int min_axis = std::min(r, c);
      board[r][c] = (min_axis + 1) * -2;
      board[rows - r - 1][c] = (min_axis + 1) * -2;
      board[r][columns - c - 1] = (min_axis + 1) * -2;
      board[rows - r - 1][columns - c - 1] = (min_axis + 1) * -2;
    }
  }

  return board;
}

// Try +1 on each cell; check adjacency (<=2 diff) and the border constraint
// (edge cells must stay <= 2, i.e. within 2 of the implicit border at 0).
// No board_min/board_max needed — the border constraint implicitly limits
// growth.
std::optional<Board>
increment_board(Board board)
{
  int rows = board.size();
  int columns = board[0].size();

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board[r][c] += 1;

      const int val = board[r][c];

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - val) <= 2 &&
        std::abs(board[std::min(rows - 1, r + 1)][c] - val) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - val) <= 2 &&
        std::abs(board[r][std::min(columns - 1, c + 1)] - val) <= 2;

      // Only edge cells are constrained by the border
      bool boundary_valid = true;
      if (r == 0 || r == rows - 1 || c == 0 || c == columns - 1) {
        boundary_valid = (val <= 2);
      }

      if (local_valid && boundary_valid) {
        return board;
      }

      board[r][c] -= 1;
    }
  }

  return std::nullopt;
}

int
calculate_rounds(const Board& dropped_board, bool print_boards)
{
  Board board = dropped_board;
  int rounds = 0;

  while (true) {
    std::optional<Board> result = increment_board(board);

    ++rounds;

    if (!result.has_value()) {
      break;
    }

    board = result.value();

    if (print_boards) {
      print_board(board);
    }
  }

  return rounds;
}

// Same rotation-canonical approach as no-border variant. The zero_offset
// here uses the edge-cell minimum rather than the global minimum.
std::tuple<bool, std::string>
check_board_is_new_combo(Board board,
                         const std::unordered_set<std::string>& results)
{
  int rows = board.size();
  int columns = board[0].size();
  int zero_offset = compute_zero_offset(board);

  std::vector<std::string> hashes;
  hashes.reserve(4);
  hashes.push_back(hash_board_zeroed(board, zero_offset));

  if (rows == columns) {
    hashes.push_back(hash_board_zeroed_rot90(board, zero_offset));
    hashes.push_back(hash_board_zeroed_rot180(board, zero_offset));
    hashes.push_back(hash_board_zeroed_rot270(board, zero_offset));
  } else {
    hashes.push_back(hash_board_zeroed_rot180(board, zero_offset));
  }

  std::sort(hashes.begin(), hashes.end());
  if (results.count(hashes[0]) > 0) {
    return { false, "" };
  }

  return { true, hashes[0] };
}

std::tuple<bool, std::string>
check_board_is_new_perm(Board board,
                        const std::unordered_set<std::string>& results)
{
  int zero_offset = compute_zero_offset(board);
  std::string board_hash = hash_board_zeroed(board, zero_offset);
  if (results.count(board_hash) > 0) {
    return { false, "" };
  }

  return { true, board_hash };
}

// Generate all valid +1 increments of a bordered board, deduped within batch.
std::unordered_map<std::string, Board>
generate_all_board_increments_combo(Board board)
{
  int rows = board.size();
  int columns = board[0].size();
  std::unordered_set<std::string> board_hashes;
  std::unordered_map<std::string, Board> board_increments;

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board[r][c] += 1;

      const int val = board[r][c];

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - val) <= 2 &&
        std::abs(board[std::min(rows - 1, r + 1)][c] - val) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - val) <= 2 &&
        std::abs(board[r][std::min(columns - 1, c + 1)] - val) <= 2;

      bool boundary_valid = true;
      if (r == 0 || r == rows - 1 || c == 0 || c == columns - 1) {
        boundary_valid = (val <= 2);
      }

      if (local_valid && boundary_valid) {
        auto [is_new_combo, min_board_hash] =
          check_board_is_new_combo(board, board_hashes);
        if (is_new_combo) {
          board_hashes.insert(min_board_hash);
          board_increments[min_board_hash] = board;
        }
      }

      board[r][c] -= 1;
    }
  }

  return board_increments;
}

std::unordered_map<std::string, Board>
generate_all_board_increments_perm(Board board)
{
  int rows = board.size();
  int columns = board[0].size();
  std::unordered_set<std::string> board_hashes;
  std::unordered_map<std::string, Board> board_increments;

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board[r][c] += 1;

      const int val = board[r][c];

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - val) <= 2 &&
        std::abs(board[std::min(rows - 1, r + 1)][c] - val) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - val) <= 2 &&
        std::abs(board[r][std::min(columns - 1, c + 1)] - val) <= 2;

      bool boundary_valid = true;
      if (r == 0 || r == rows - 1 || c == 0 || c == columns - 1) {
        boundary_valid = (val <= 2);
      }

      if (local_valid && boundary_valid) {
        auto [is_new_perm, board_hash] =
          check_board_is_new_perm(board, board_hashes);
        if (is_new_perm) {
          board_hashes.insert(board_hash);
          board_increments[board_hash] = board;
        }
      }

      board[r][c] -= 1;
    }
  }

  return board_increments;
}

// Multi-threaded entry point: reuses the map key as the canonical hash.
std::vector<std::pair<Board, std::string>>
generate_boards_mp_combo(const std::vector<Board>& split_increments)
{
  int rows = 0, columns = 0;
  if (!split_increments.empty()) {
    rows = split_increments[0].size();
    columns = split_increments[0][0].size();
  }

  std::vector<std::pair<Board, std::string>> new_boards;
  new_boards.reserve(split_increments.size() * rows * columns);

  for (const Board& board : split_increments) {
    std::unordered_map<std::string, Board> increments =
      generate_all_board_increments_combo(board);

    for (const auto& [canonical_hash, board_increment] : increments) {
      new_boards.emplace_back(board_increment, canonical_hash);
    }
  }

  return new_boards;
}

std::vector<std::pair<Board, std::string>>
generate_boards_mp_perm(const std::vector<Board>& split_increments)
{
  int rows = 0, columns = 0;
  if (!split_increments.empty()) {
    rows = split_increments[0].size();
    columns = split_increments[0][0].size();
  }

  std::vector<std::pair<Board, std::string>> new_boards;
  new_boards.reserve(split_increments.size() * rows * columns);

  for (const Board& board : split_increments) {
    std::unordered_map<std::string, Board> increments =
      generate_all_board_increments_perm(board);

    for (const auto& [perm_hash, board_increment] : increments) {
      new_boards.emplace_back(board_increment, perm_hash);
    }
  }

  return new_boards;
}
