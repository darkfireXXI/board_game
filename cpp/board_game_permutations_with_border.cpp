#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <thread>
#include <unordered_set>
#include <vector>

#include "board_game_permutations_utils.h"

namespace fs = std::filesystem;

Board zero_board(const Board &board) {
  int size = board.size();
  int min_edge = std::numeric_limits<int>::max();

  // find min of all edge values (top, bottom, left, right)
  for (int i = 0; i < size; ++i) {
    min_edge = std::min(min_edge, board[0][i]);
    min_edge = std::min(min_edge, board[size - 1][i]);
    if (i > 0 && i < size - 1) {
      min_edge = std::min(min_edge, board[i][0]);
      min_edge = std::min(min_edge, board[i][size - 1]);
    }
  }

  // create a new board with values subtracted
  Board result(board);
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      result[i][j] -= min_edge;
    }
  }

  return result;
}

Board drop_board(Board board) {
  int size = (int)board.size();
  int rings = size / 2 + (size % 2 != 0);
  for (int i = 0; i < rings; ++i) {
    int val = -2 * (i + 1);
    for (int c = i; c < size - i; ++c) {
      board[i][c] = val;
      board[size - i - 1][c] = val;
    }
    for (int r = i; r < size - i; ++r) {
      board[r][i] = val;
      board[r][size - i - 1] = val;
    }
  }
  return board;
}

using Board = std::vector<std::vector<int>>;

std::optional<Board> increment_board(Board board) {
  int size = board.size();

  for (int r = 0; r < size; ++r) {
    for (int c = 0; c < size; ++c) {
      board[r][c] += 1;

      bool local_valid =
          std::abs(board[std::max(0, r - 1)][c] - board[r][c]) <= 2 &&
          std::abs(board[std::min(size - 1, r + 1)][c] - board[r][c]) <= 2 &&
          std::abs(board[r][std::max(0, c - 1)] - board[r][c]) <= 2 &&
          std::abs(board[r][std::min(size - 1, c + 1)] - board[r][c]) <= 2;

      bool boundary_valid = true;
      for (int i = 0; i < size; ++i) {
        if (board[0][i] > 2 || board[size - 1][i] > 2 || board[i][0] > 2 ||
            board[i][size - 1] > 2) {
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

int calculate_rounds(const Board &dropped_board, bool print_boards = false) {
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

bool check_board_is_new_combo(Board board,
                              const std::unordered_set<std::string> &results) {
  Board zeroed_board = zero_board(board);

  std::vector<Board> rotations(4);
  std::vector<std::string> hashes(4);

  rotations[0] = zeroed_board;
  hashes[0] = hash_board(zeroed_board);

  for (int i = 1; i < 4; ++i) {
    rotations[i] = rotate_board_90(rotations[i - 1]);
    hashes[i] = hash_board(rotations[i]);
  }

  for (std::string h : hashes) {
    if (results.count(h) > 0) {
      return false;
    }
  }

  return true;
}

std::unordered_map<std::string, Board>
generate_all_board_increments(Board input_board) {
  std::unordered_set<std::string> board_hashes;
  std::unordered_map<std::string, Board> board_increments;
  int size = input_board.size();

  Board board = input_board;

  for (int r = 0; r < size; ++r) {
    for (int c = 0; c < size; ++c) {
      board[r][c] += 1;

      bool local_valid =
          std::abs(board[std::max(0, r - 1)][c] - board[r][c]) <= 2 &&
          std::abs(board[std::min(size - 1, r + 1)][c] - board[r][c]) <= 2 &&
          std::abs(board[r][std::max(0, c - 1)] - board[r][c]) <= 2 &&
          std::abs(board[r][std::min(size - 1, c + 1)] - board[r][c]) <= 2;

      bool boundary_valid = true;
      for (int i = 0; i < size; ++i) {
        if (board[0][i] > 2 || board[size - 1][i] > 2 || board[i][0] > 2 ||
            board[i][size - 1] > 2) {
          boundary_valid = false;
          break;
        }
      }

      if (local_valid && boundary_valid) {
        if (check_board_is_new_combo(board, board_hashes)) {
          std::string board_hash = hash_board(zero_board(board));
          board_hashes.insert(board_hash);
          board_increments[board_hash] = board;
        }
      }

      board[r][c] -= 1;
    }
  }

  return board_increments;
}

int main(int argc, char *argv[]) {
  int size = 2, n_jobs = 1;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-s" || arg == "--size") && i + 1 < argc) {
      size = std::stoi(argv[++i]);
    } else if ((arg == "-n" || arg == "--n-jobs") && i + 1 < argc) {
      n_jobs = std::stoi(argv[++i]);
    }
  }

  fs::create_directory("new_increments");
  fs::create_directory("results");

  Board initial_board = generate_initial_board(size);
  Board dropped_board = drop_board(initial_board);
  int rounds = calculate_rounds(dropped_board, false);

  Board zeroed_board = zero_board(dropped_board);
  std::unordered_set<std::string> results = {hash_board(zeroed_board)};

  std::vector<std::string> result_files;
  std::vector<Board> last_round_increments = {dropped_board};
  std::vector<Board> new_increments;
  std::vector<std::string> increment_files;
  std::vector<std::string> new_increment_files;

  std::time_t start = std::time(nullptr);

  for (int round = 1; round <= rounds; ++round) {
    std::time_t round_start = std::time(nullptr);

    for (std::size_t i = 0; i < last_round_increments.size(); ++i) {
      Board last_round_new_increment = last_round_increments[i];
      std::unordered_map<std::string, Board> board_increments =
          generate_all_board_increments(last_round_new_increment);

      for (std::unordered_map<std::string, Board>::iterator it =
               board_increments.begin();
           it != board_increments.end(); ++it) {
        Board board_increment = it->second;
        bool is_new_combo = check_board_is_new_combo(board_increment, results);

        if (is_new_combo) {
          Board zeroed_board = zero_board(board_increment);
          results.insert(hash_board(zeroed_board));
          new_increments.push_back(board_increment);
        }
      }
    }

    int new_board_count =
        get_file_item_count(increment_files, "new_increments") +
        new_increments.size();
    int result_count =
        get_file_item_count(result_files, "results") + results.size();

    std::time_t now = std::time(nullptr);
    display_round_stats(round, rounds, start, round_start, new_board_count,
                        result_count);

    last_round_increments = new_increments;
    new_increments.clear();
  }

  std::time_t end = std::time(nullptr);

  double seconds_elapsed = end - start;
  double hours_elapsed = seconds_elapsed / 3600.0;

  std::cout << std::fixed << std::setprecision(1);

  std::cout << hours_elapsed << " hours or ";
  std::cout << seconds_elapsed << " seconds";
  std::cout << std::endl;

  int result_count = get_file_item_count(result_files, "results");
  std::cout << (result_count + results.size()) << " total unique boards found";
  std::cout << std::endl;
}
