#include "board_game_permutations_utils.h"

void print_board(const Board &board) {
  std::cout << "Board:\n";
  for (const std::vector row : board) {
    for (int val : row) {
      std::string str_int = std::to_string(val);
      std::string padding(4 - str_int.length(), ' ');
      std::cout << val << padding;
    }
    std::cout << '\n';
  }
  std::cout << std::endl;
}

Board rotate_board_90(Board board) {
  int size = board.size();

  // transpose
  for (int i = 0; i < size; ++i) {
    for (int j = i + 1; j < size; ++j) {
      std::swap(board[i][j], board[j][i]);
    }
  }

  // reverse each row
  for (int i = 0; i < size; ++i) {
    std::reverse(board[i].begin(), board[i].end());
  }

  return board;
}

Board generate_initial_board(const int &size) {
  return Board(size, std::vector<int>(size, 0));
}

std::string hash_board(const Board &board) {
  int size = board.size();
  std::string board_str;
  board_str.reserve(board.size() * board.size() * 3);

  for (int r = 0; r < size; ++r) {
    for (int c = 0; c < size; ++c) {
      // board_str += std::to_string(r) + ","+ std::to_string(c) + "," +
      // std::to_string(board[r][c]) + "|";
      board_str += std::to_string(board[r][c]) + "|";
    }
  }

  return board_str;
}

int get_file_item_count(std::vector<std::string> files,
                        std::string folder_name) {
  int count = 0;

  for (size_t i = 0; i < files.size(); ++i) {
    std::filesystem::path file_path =
        std::filesystem::current_path() / folder_name / files[i];
    std::ifstream file(file_path);

    if (!file.is_open()) {
      std::cerr << "Warning: Could not open file " << file_path << std::endl;
      continue;
    }

    std::string line;
    while (std::getline(file, line)) {
      ++count;
    }

    file.close();
  }

  return count;
}

void display_round_stats(int round, int rounds, std::time_t start,
                         std::time_t round_start, int new_board_count,
                         int result_count) {
  std::time_t now = std::time(nullptr);

  int round_seconds_elapsed = now - round_start;
  int total_seconds_elapsed = now - start;

  double round_hours_elapsed = round_seconds_elapsed / 3600.0;
  double total_hours_elapsed = total_seconds_elapsed / 3600.0;

  double percent_done = (round + 0.0) / rounds * 100.0;
  double time_spent_ratio =
      ((round_seconds_elapsed + 0.0) / (total_seconds_elapsed + 1e-6)) * 100.0;

  std::cout << std::fixed << std::setprecision(1);

  std::cout << "Round " << round << "\t\t" << round << "/" << rounds << "\t";
  std::cout << percent_done << "%" << std::endl;

  std::cout << "\t" << new_board_count << " unique boards found this round";
  std::cout << std::endl;

  std::cout << "\t" << result_count << " unique boards found so far";
  std::cout << std::endl;

  std::cout << "\t" << round_hours_elapsed << " hours or ";
  std::cout << round_seconds_elapsed << " seconds elapsed this round";
  std::cout << std::endl;

  std::cout << "\t" << total_hours_elapsed << " hours or ";
  std::cout << total_seconds_elapsed << " seconds elapsed so far";
  std::cout << std::endl;

  std::cout << "\t" << time_spent_ratio;
  std::cout << "% of total time spent on this round";
  std::cout << std::endl;
  std::cout << std::endl;
}
