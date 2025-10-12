#include "board_game_counting_utils.h"
#include "board_game_no_border_utils.h"

long long
get_current_time_ms()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration duration = now.time_since_epoch();
  std::chrono::milliseconds millis =
    std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  return millis.count();
}

void
print_board(const Board& board)
{
  std::cout << '\n';
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

Board
generate_initial_board(const int& rows, const int& columns)
{
  return Board(rows, std::vector<int>(columns, 0));
}

Board
rotate_board_90(Board board)
{
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

Board
rotate_board_180(Board board)
{
  int rows = board.size();
  int mid_row = (rows / 2) + (rows % 2);

  // swap rows
  for (int r = 0; r < mid_row; ++r) {
    std::swap(board[r], board[rows - r - 1]);
  }

  // reverse each row
  for (int r = 0; r < rows; ++r) {
    std::reverse(board[r].begin(), board[r].end());
  }

  return board;
}

std::string
hash_board(const Board& board)
{
  int rows = int(board.size());
  int columns = int(board[0].size());

  std::string board_str;
  board_str.reserve(rows * columns * 3);

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      // board_str += std::to_string(r) + ","+ std::to_string(c) + "," +
      board_str += std::to_string(board[r][c]) + "|";
    }
  }

  return board_str;
}

Board
board_hash_to_array(const std::string& board_hash_str,
                    const int& rows,
                    const int& columns)
{
  std::vector<int> flat_values;
  std::stringstream ss(board_hash_str);
  std::string token;

  while (std::getline(ss, token, '|')) {
    flat_values.push_back(std::stoi(token));
  }

  Board board = generate_initial_board(rows, columns);
  for (int i = 0; i < rows * columns; ++i) {
    int row = i / columns;
    int col = i % columns;
    board[row][col] = flat_values[i];
  }

  return board;
}

std::vector<std::vector<Board>>
split_list(const std::vector<Board>& list, int n)
{
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

std::string
write_to_file(const std::vector<std::string>& results,
              const std::string& folder_name)
{
  std::string filename =
    folder_name + "_" + std::to_string(get_current_time_ms()) + ".txt";
  fs::path filepath = fs::current_path() / folder_name / filename;

  std::ofstream file(filepath);
  for (const std::string& result : results) {
    file << result << "\n";
  }

  return filename;
}

std::string
write_to_file(const std::vector<Board>& new_increments,
              const std::string& folder_name)
{
  std::string filename =
    folder_name + "_" + std::to_string(get_current_time_ms()) + ".txt";
  fs::path filepath = fs::current_path() / folder_name / filename;

  std::ofstream file(filepath);
  for (const Board& new_increment : new_increments) {
    file << hash_board(new_increment) << "\n";
  }

  return filename;
}

std::vector<std::vector<uint8_t>>
check_results_vs_files(
  const std::vector<std::string>& result_files,
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  const int& n_jobs,
  const long long& max_in_mem)
{
  std::vector<std::vector<uint8_t>> is_new_checks;
  is_new_checks.reserve(n_jobs);
  for (const auto& result_chunk : results_lists) {
    is_new_checks.emplace_back(result_chunk.size(), 1);
  }

  for (const std::string& result_file : result_files) {
    std::unordered_set<std::string> temp_results;
    temp_results.reserve(max_in_mem);

    fs::path filepath = fs::current_path() / "results" / result_file;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);

    const char* data = buffer.data();
    size_t start = 0;
    for (size_t i = 0; i < static_cast<size_t>(file_size); ++i) {
      if (data[i] == '\n') {
        size_t len = i - start;
        if (len > 0) {
          std::string_view sv(data + start, len);
          temp_results.emplace(sv);
        }
        start = i + 1;
      }
    }

    std::vector<std::future<std::vector<uint8_t>>> check_futures;
    check_futures.reserve(n_jobs);
    for (size_t nj = 0; nj < n_jobs; ++nj) {
      std::vector<uint8_t> is_new_check_copy = is_new_checks[nj];
      check_futures.push_back(std::async(std::launch::async,
                                         check_results_vs_mp,
                                         std::cref(results_lists[nj]),
                                         std::move(is_new_check_copy),
                                         std::cref(temp_results)));
    }

    for (size_t nj = 0; nj < n_jobs; ++nj) {
      is_new_checks[nj] = check_futures[nj].get();
    }
  }

  return is_new_checks;
}

std::vector<std::vector<uint8_t>>
check_results_vs_results(
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  std::vector<std::vector<uint8_t>>& is_new_checks,
  const std::unordered_set<std::string>& results,
  const int& n_jobs)
{
  std::vector<std::future<std::vector<uint8_t>>> check_futures;
  check_futures.reserve(n_jobs);
  for (size_t nj = 0; nj < n_jobs; ++nj) {
    std::vector<uint8_t> is_new_check_copy = is_new_checks[nj];
    check_futures.push_back(std::async(std::launch::async,
                                       check_results_vs_mp,
                                       std::cref(results_lists[nj]),
                                       std::move(is_new_check_copy),
                                       std::cref(results)));
  }

  for (size_t nj = 0; nj < n_jobs; ++nj) {
    is_new_checks[nj] = check_futures[nj].get();
  }

  return is_new_checks;
}

std::vector<uint8_t>
check_results_vs_mp(
  const std::vector<std::pair<Board, std::string>>& results_list,
  std::vector<uint8_t> is_new_check,
  const std::unordered_set<std::string>& results)
{
  for (size_t i = 0; i < results_list.size(); ++i) {
    if (is_new_check[i]) {
      const std::string& board_hash = results_list[i].second;
      if (results.count(board_hash) > 0) {
        is_new_check[i] = 0;
      }
    }
  }

  return is_new_check;
}

long long
get_file_item_count(std::vector<std::string> files, std::string folder_name)
{
  long long count = 0;

  if (folder_name == "new_increments") {
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
  } else if (folder_name == "results") {
    long long count_per_file = 0;
    if (files.size() > 0) {
      std::filesystem::path file_path =
        std::filesystem::current_path() / folder_name / files[0];
      std::ifstream file(file_path);

      std::string line;
      while (std::getline(file, line)) {
        ++count_per_file;
      }
    }
    count = files.size() * count_per_file;
  }

  return count;
}

void
display_round_stats(int round,
                    int rounds,
                    std::time_t start,
                    std::time_t round_start,
                    int new_board_count,
                    int result_count)
{
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
