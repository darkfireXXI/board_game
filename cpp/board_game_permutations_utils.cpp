#include "board_game_permutations_utils.h"

long long get_current_time_ms() {
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration duration = now.time_since_epoch();
  std::chrono::milliseconds millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  return millis.count();
}

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
      board_str += std::to_string(board[r][c]) + "|";
    }
  }

  return board_str;
}

Board board_hash_to_array(const std::string &board_hash_str, int size) {
  std::vector<int> flat_values;
  std::stringstream ss(board_hash_str);
  std::string token;

  while (std::getline(ss, token, '|')) {
    flat_values.push_back(std::stoi(token));
    // } catch (const std::invalid_argument &e) {
    //   throw std::runtime_error("Invalid number in board hash: " + token);
    // }
  }

  // if (flat_values.size() != size * size) {
  //   throw std::runtime_error(
  //       "Hash does not contain the right number of values for the board
  //       size.");
  // }

  // populate 2D board

  Board board(size, std::vector<int>(size));
  for (int i = 0; i < size * size; ++i) {
    int row = i / size;
    int col = i % size;
    board[row][col] = flat_values[i];
  }

  return board;
}

std::vector<std::vector<Board>> split_list(const std::vector<Board> &list,
                                           int n) {
  int total = list.size();
  int split = std::max(total / n, 1);

  std::vector<std::vector<Board>> splits;

  for (int i = 0; i < n; ++i) {
    int start = i + (i * split);
    int end = i + ((i + 1) * split) + 1;

    end = std::min(end, total);

    if (start >= total)
      break;

    splits.emplace_back(list.begin() + start, list.begin() + end);
  }

  return splits;
}

std::string write_to_file(const std::unordered_set<std::string> &results,
                          const std::string &folder_name) {
  std::string filename =
      folder_name + "_" + std::to_string(get_current_time_ms()) + ".txt";
  fs::path filepath = fs::current_path() / folder_name / filename;

  std::ofstream file(filepath);
  for (const std::string &result : results) {
    file << result << "\n";
  }

  return filename;
}

std::string write_to_file(const std::vector<Board> &new_increments,
                          const std::string &folder_name) {
  std::string filename =
      folder_name + "_" + std::to_string(get_current_time_ms()) + ".txt";
  fs::path filepath = fs::current_path() / folder_name / filename;

  std::ofstream file(filepath);
  for (const Board &new_increment : new_increments) {
    file << hash_board(new_increment) << "\n";
  }

  return filename;
}

std::vector<bool> check_results_vs_files(
    const std::vector<std::pair<Board, std::vector<std::string>>> &results_list,
    const std::vector<std::string> &result_files) {
  std::vector<bool> is_new_check(results_list.size(), true);

  for (const std::string &filename : result_files) {
    fs::path filepath = fs::current_path() / "results" / filename;
    std::ifstream file(filepath);

    std::unordered_set<std::string> temp_results;
    temp_results.reserve(10'000'000);
    std::string line;
    while (std::getline(file, line)) {
      temp_results.insert(line);
    }

    for (size_t i = 0; i < results_list.size(); ++i) {
      if (!is_new_check[i])
        continue;

      const std::vector<std::string> &rotated_hashes = results_list[i].second;
      for (const std::string &h : rotated_hashes) {
        if (temp_results.count(h) > 0) {
          is_new_check[i] = false;
        }
      }
    }
  }

  return is_new_check;
}

long long get_file_item_count(std::vector<std::string> files,
                              std::string folder_name) {
  long long count = 0;

  for (size_t i = 0; i < files.size(); ++i) {
    std::filesystem::path file_path =
        std::filesystem::current_path() / folder_name / files[i];
    std::ifstream file(file_path);

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
  long long now = get_current_time_ms();

  double round_seconds_elapsed = (now - round_start) / 1000.0;
  double total_seconds_elapsed = (now - start) / 1000.0;

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
