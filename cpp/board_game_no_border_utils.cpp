#include "board_game_no_border_utils.h"
#include "board_game_counting_utils.h"

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

std::optional<Board>
increment_board(Board board, const int& board_min, const int& board_max)
{
  int rows = board.size();
  int columns = board[0].size();

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board[r][c] += 1;

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - board[r][c]) <= 2 &&
        std::abs(board[std::min(rows - 1, r + 1)][c] - board[r][c]) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - board[r][c]) <= 2 &&
        std::abs(board[r][std::min(columns - 1, c + 1)] - board[r][c]) <= 2;

      bool boundary_valid = true;
      for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < columns; ++c) {
          if (board[r][c] < board_min || board[r][c] > board_max) {
            boundary_valid = false;
            break;
          }
        }
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

std::tuple<bool, std::string>
check_board_is_new_combo(Board board,
                         const std::unordered_set<std::string>& results)
{
  int rows = board.size();
  int columns = board[0].size();
  Board rotation = zero_board(board);

  std::vector<std::string> hashes;
  hashes.reserve(4);
  hashes.emplace_back(hash_board(rotation));

  for (size_t i = 1; i < 4; ++i) {
    if (rows == columns) {
      rotation = rotate_board_90(rotation);
      hashes.emplace_back(hash_board(rotation));
    } else if (i % 2 == 0) {
      rotation = rotate_board_180(rotation);
      hashes.emplace_back(hash_board(rotation));
    }
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
  std::string board_hash = hash_board(zero_board(board));
  if (results.count(board_hash) > 0) {
    return { false, "" };
  }

  return { true, board_hash };
}

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

      bool boundary_valid = true;
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < columns; ++c) {
          if (board[r][c] < board_min || board[r][c] > board_max) {
            boundary_valid = false;
            break;
          }
        }
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

      bool boundary_valid = true;
      for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < columns; ++c) {
          if (board[r][c] < board_min || board[r][c] > board_max) {
            boundary_valid = false;
            break;
          }
        }
      }

      if (local_valid && boundary_valid) {
        auto [is_new_combo, board_hash] =
          check_board_is_new_perm(board, board_hashes);
        if (is_new_combo) {
          board_hashes.insert(board_hash);
          board_increments[board_hash] = board;
        }
      }

      board[r][c] -= 1;
    }
  }

  return board_increments;
}

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

    for (const std::pair<const std::string, Board>& entry : increments) {
      const Board& board_increment = entry.second;

      std::vector<std::string> hashes;
      hashes.reserve(4);
      Board rotation = zero_board(board_increment);
      hashes.emplace_back(hash_board(rotation));

      for (size_t i = 1; i < 4; ++i) {
        if (rows == columns) {
          rotation = rotate_board_90(rotation);
          hashes.emplace_back(hash_board(rotation));
        } else if (i % 2 == 0) {
          rotation = rotate_board_180(rotation);
          hashes.emplace_back(hash_board(rotation));
        }
      }

    std:
      sort(hashes.begin(), hashes.end());
      new_boards.emplace_back(board_increment, hashes[0]);
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

  std::vector<std::string> hashes(4);
  for (const Board& board : split_increments) {
    std::unordered_map<std::string, Board> increments =
      generate_all_board_increments_perm(board, board_min, board_max);

    for (const std::pair<const std::string, Board>& entry : increments) {
      const Board& board_increment = entry.second;
      std::string board_hash = hash_board(zero_board(board_increment));
      new_boards.emplace_back(board_increment, board_hash);
    }
  }

  return new_boards;
}
