// No-border variant utilities.
// Key difference from with-border: no edge constraint, so cells can grow
// without limit. Zeroing uses the global minimum across all cells.

#include "board_game_no_border_utils.h"
#include "board_game_counting_utils.h"

// Normalize board heights so the smallest cell is 0.
Board
zero_board(const Board& board)
{
  int rows = board.size();
  int columns = board[0].size();

  int min_square = std::numeric_limits<int>::max();

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      min_square = std::min(min_square, board[r][c]);
    }
  }

  Board zeroed_board(board);
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      zeroed_board[r][c] -= min_square;
    }
  }

  return zeroed_board;
}

int
compute_zero_offset(const Board& board)
{
  int rows = board.size();
  int columns = board[0].size();
  int min_val = std::numeric_limits<int>::max();
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < columns; ++c)
      min_val = std::min(min_val, board[r][c]);
  return min_val;
}

// Diagonal ramp: board[r][c] = (r+c)*2. This is the lowest possible board
// where every adjacent pair differs by exactly 2 — the BFS starting point.
Board
find_most_extreme_board(Board board)
{
  int rows = board.size();
  int columns = board[0].size();
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board[r][c] += (r + c) * 2;
    }
  }

  return board;
}

// Try +1 on each cell in row-major order; return the first valid increment.
// Validity: all 4 neighbors differ by <=2 (adjacency), and the cell doesn't
// exceed board_max (the max height of the extreme board — a hard ceiling).
std::optional<Board>
increment_board(Board board, const int& board_min, const int& board_max)
{
  int rows = board.size();
  int columns = board[0].size();

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board[r][c] += 1;

      const int val = board[r][c];

      // Only check the 4 direct neighbors of the modified cell
      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - val) <= 2 &&
        std::abs(board[std::min(rows - 1, r + 1)][c] - val) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - val) <= 2 &&
        std::abs(board[r][std::min(columns - 1, c + 1)] - val) <= 2;

      if (local_valid && val <= board_max) {
        return board;
      }

      board[r][c] -= 1;
    }
  }

  return std::nullopt;
}

int
calculate_rounds(const Board& most_extreme_board,
                 const int& board_min,
                 const int& board_max,
                 bool print_boards)
{
  Board board = most_extreme_board;
  int rounds = 0;

  while (true) {
    std::optional<Board> result = increment_board(board, board_min, board_max);

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

// Canonical key for combinations: compute zeroed hashes for all valid rotations
// (4 for square boards, 2 for rectangular — 90°/270° change the aspect ratio),
// then pick the lexicographic minimum. Two boards that are rotations of each
// other will always produce the same canonical key.
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

// Permutations treat rotations as distinct — just zero and hash directly.
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

// Try +1 on every cell of the board; collect all valid, locally-unique
// increments. board_hashes deduplicates within this single parent board
// (e.g., incrementing different cells may produce rotation-equivalent results).
std::unordered_map<std::string, Board>
generate_all_board_increments_combo(Board board,
                                    const int& board_min,
                                    const int& board_max)
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

      if (local_valid && val <= board_max) {
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
generate_all_board_increments_perm(Board board,
                                   const int& board_min,
                                   const int& board_max)
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

      if (local_valid && val <= board_max) {
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

// Multi-threaded entry point: processes a chunk of parent boards and collects
// all their valid increments as (board, canonical_hash) pairs. The hash is
// reused from the map key — no recomputation needed.
std::vector<std::pair<Board, std::string>>
generate_boards_mp_combo(const std::vector<Board>& split_increments,
                         const int& board_min,
                         const int& board_max)
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
      generate_all_board_increments_combo(board, board_min, board_max);

    for (const auto& [canonical_hash, board_increment] : increments) {
      new_boards.emplace_back(board_increment, canonical_hash);
    }
  }

  return new_boards;
}

std::vector<std::pair<Board, std::string>>
generate_boards_mp_perm(const std::vector<Board>& split_increments,
                        const int& board_min,
                        const int& board_max)
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
      generate_all_board_increments_perm(board, board_min, board_max);

    for (const auto& [perm_hash, board_increment] : increments) {
      new_boards.emplace_back(board_increment, perm_hash);
    }
  }

  return new_boards;
}
