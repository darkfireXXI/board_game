#include "board_game_permutations_utils.h"

Board
zero_board(const Board& board)
{
  int rs = int(board.size());
  int cs = int(board[0].size());
  int min_edge = std::numeric_limits<int>::max();

  // find min of all edge values (top, bottom, left, right)
  for (int r = 0; r < rs; ++r) {
    min_edge = std::min(min_edge, board[r][0]);
    min_edge = std::min(min_edge, board[r][cs - 1]);
  }
  for (int c = 0; c < cs; ++c) {
    min_edge = std::min(min_edge, board[0][c]);
    min_edge = std::min(min_edge, board[rs - 1][c]);
  }

  // create a new board with values subtracted
  Board zeroed_board(board);
  for (int r = 0; r < rs; ++r) {
    for (int c = 0; c < cs; ++c) {
      zeroed_board[r][c] -= min_edge;
    }
  }

  return zeroed_board;
}

Board
drop_board(Board board)
{
  int rs = int(board.size());
  int cs = int(board[0].size());
  int r_mid = rs / 2 - (rs % 2 == 0);
  int c_mid = cs / 2 - (cs % 2 == 0);

  for (int r = 0; r <= r_mid; ++r) {
    for (int c = 0; c <= c_mid; ++c) {
      int min_axis = std::min(r, c);
      board[r][c] = (min_axis + 1) * -2;
      board[rs - r - 1][c] = (min_axis + 1) * -2;
      board[r][cs - c - 1] = (min_axis + 1) * -2;
      board[rs - r - 1][cs - c - 1] = (min_axis + 1) * -2;
    }
  }

  return board;
}

std::optional<Board>
increment_board(Board board)
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
      for (size_t r = 0; r < rs; ++r) {
        if (board[r][0] > 2 || board[r][cs - 1] > 2) {
          boundary_valid = false;
          break;
        }
      }
      for (size_t c = 0; c < cs; ++c) {
        if (board[0][c] > 2 || board[rs - 1][c] > 2) {
          boundary_valid = false;
          break;
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
calculate_rounds(const Board& dropped_board, bool print_boards = false)
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
generate_all_board_increments(Board input_board)
{
  int rs = int(input_board.size());
  int cs = int(input_board[0].size());
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
        if (board[r][0] > 2 || board[r][cs - 1] > 2) {
          boundary_valid = false;
          break;
        }
      }
      for (size_t c = 0; c < cs; ++c) {
        if (board[0][c] > 2 || board[rs - 1][c] > 2) {
          boundary_valid = false;
          break;
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
  Board dropped_board = drop_board(initial_board);
  print_board(dropped_board);
  int rounds = calculate_rounds(dropped_board, false);
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
        generate_all_board_increments(last_round_new_increment);

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
