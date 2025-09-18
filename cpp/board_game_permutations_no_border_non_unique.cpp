#include "board_game_permutations_utils.h"

Board
zero_board(const Board& board)
{
  int rs = int(board.size());
  int cs = int(board[0].size());

  int first_square = board[0][0];

  Board zeroed_board(board);
  for (int r = 0; r < rs; ++r) {
    for (int c = 0; c < cs; ++c) {
      zeroed_board[r][c] -= first_square;
    }
  }

  return zeroed_board;
}

Board
find_most_extreme_board(Board board)
{
  int rs = int(board.size());
  int cs = int(board[0].size());

  for (int r = 0; r < rs; ++r) {
    for (int c = 0; c < cs; ++c) {
      board[r][c] += (r + c) * 2;
    }
  }

  return board;
}

std::optional<Board>
increment_board(Board board, const int& board_min, const int& board_max)
{
  int rs = int(board.size());
  int cs = int(board[0].size());

  for (int r = 0; r < rs; ++r) {
    for (int c = 0; c < cs; ++c) {
      board[r][c] += 1;

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - board[r][c]) <= 2 &&
        std::abs(board[std::min(rs - 1, r + 1)][c] - board[r][c]) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - board[r][c]) <= 2 &&
        std::abs(board[r][std::min(cs - 1, c + 1)] - board[r][c]) <= 2;

      bool boundary_valid = true;
      for (int r = 0; r < rs; ++r) {
        for (int c = 0; c < cs; ++c) {
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
                 bool print_boards = false)
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
  std::string hash = hash_rectangular_board(zero_board(board));
  if (results.count(hash) > 0) {
    return { false, "" };
  }

  return { true, hash };
}

std::unordered_map<std::string, Board>
generate_all_board_increments(Board input_board,
                              const int& board_min,
                              const int& board_max)
{
  int rs = static_cast<int>(input_board.size());
  int cs = static_cast<int>(input_board[0].size());
  std::unordered_set<std::string> board_hashes;
  std::unordered_map<std::string, Board> board_increments;

  Board board = input_board;

  for (int r = 0; r < rs; ++r) {
    for (int c = 0; c < cs; ++c) {
      board[r][c] += 1;

      const int val = board[r][c];

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - val) <= 2 &&
        std::abs(board[std::min(rs - 1, r + 1)][c] - val) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - val) <= 2 &&
        std::abs(board[r][std::min(cs - 1, c + 1)] - val) <= 2;

      bool boundary_valid = true;
      for (size_t r = 0; r < rs; ++r) {
        for (size_t c = 0; c < cs; ++c) {
          if (board[r][c] < board_min || board[r][c] > board_max) {
            boundary_valid = false;
            break;
          }
        }
      }

      if (local_valid && boundary_valid) {
        auto [is_new_combo, board_hash] =
          check_board_is_new_combo(board, board_hashes);
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

int
main(int argc, char* argv[])
{
  int rows = 2, columns = 2;

  for (size_t i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-r" || arg == "--rows") && i + 1 < argc) {
      rows = std::stoi(argv[++i]);
    } else if ((arg == "-c" || arg == "--columns") && i + 1 < argc) {
      columns = std::stoi(argv[++i]);
    }
  }

  Board initial_board =
    Board(std::vector<std::vector<int>>(rows, std::vector<int>(columns, 0)));
  Board most_extreme_board = find_most_extreme_board(initial_board);
  int board_min = std::numeric_limits<int>::max();
  int board_max = std::numeric_limits<int>::min();
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      board_min = std::min(board_min, most_extreme_board[r][c]);
      board_max = std::max(board_max, most_extreme_board[r][c]);
    }
  }
  int rounds =
    calculate_rounds(most_extreme_board, board_min, board_max, false);
  std::cout << rounds << " rounds for a " << rows << "x" << columns
            << " board\n";

  Board zeroed_board = zero_board(initial_board);
  std::unordered_set<std::string> results = { hash_rectangular_board(
    zeroed_board) };
  std::vector<Board> last_round_increments = { zeroed_board };
  std::vector<Board> new_increments;

  long long start = get_current_time_ms();

  for (size_t round = 1; round <= rounds; ++round) {
    long long round_start = get_current_time_ms();

    for (std::size_t i = 0; i < last_round_increments.size(); ++i) {
      Board last_round_new_increment = last_round_increments[i];
      std::unordered_map<std::string, Board> board_increments =
        generate_all_board_increments(
          last_round_new_increment, board_min, board_max);

      for (std::unordered_map<std::string, Board>::iterator it =
             board_increments.begin();
           it != board_increments.end();
           ++it) {
        Board board_increment = it->second;
        auto [is_new_combo, min_board_hash] =
          check_board_is_new_combo(board_increment, results);

        if (is_new_combo) {
          results.insert(min_board_hash);
          new_increments.push_back(board_increment);
        }
      }
    }

    display_round_stats(
      round, rounds, start, round_start, new_increments.size(), results.size());

    last_round_increments = new_increments;
    new_increments.clear();
  }

  long long end = get_current_time_ms();

  double seconds_elapsed = (end - start) / 1000.0;
  double hours_elapsed = seconds_elapsed / 3600.0;

  std::cout << std::fixed << std::setprecision(1);

  std::cout << hours_elapsed << " hours or ";
  std::cout << seconds_elapsed << " seconds";
  std::cout << std::endl;

  std::cout << results.size() << " total unique boards found";
  std::cout << std::endl;
}
