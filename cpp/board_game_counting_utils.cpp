// Shared utilities: compact binary hashing, rotation-without-copy, file I/O
// for disk spilling, multi-threaded dedup checking, and display helpers.

#include "board_game_counting_utils.h"
#include "board_game_no_border_utils.h"

// --- Bloom filter implementation ---

std::pair<uint64_t, uint64_t>
BloomFilter::hash_pair(const std::string& key) const
{
  uint64_t h1 = std::hash<std::string>{}(key);
  uint64_t h2 = 14695981039346656037ULL;
  for (unsigned char c : key) {
    h2 ^= c;
    h2 *= 1099511628211ULL;
  }
  return { h1, h2 };
}

BloomFilter::BloomFilter(size_t expected_items, double fp_rate)
{
  double ln2 = std::log(2.0);
  num_bits_ = static_cast<size_t>(std::ceil(
    -static_cast<double>(expected_items) * std::log(fp_rate) / (ln2 * ln2)));
  num_hashes_ =
    static_cast<int>(std::round(static_cast<double>(num_bits_) /
                                static_cast<double>(expected_items) * ln2));
  if (num_hashes_ < 1)
    num_hashes_ = 1;
  bits_.resize((num_bits_ + 63) / 64, 0);
}

void
BloomFilter::insert(const std::string& key)
{
  auto [h1, h2] = hash_pair(key);
  for (int i = 0; i < num_hashes_; ++i) {
    uint64_t idx = (h1 + static_cast<uint64_t>(i) * h2) % num_bits_;
    bits_[idx / 64] |= (1ULL << (idx % 64));
  }
}

bool
BloomFilter::possibly_contains(const std::string& key) const
{
  auto [h1, h2] = hash_pair(key);
  for (int i = 0; i < num_hashes_; ++i) {
    uint64_t idx = (h1 + static_cast<uint64_t>(i) * h2) % num_bits_;
    if (!(bits_[idx / 64] & (1ULL << (idx % 64))))
      return false;
  }
  return true;
}

size_t
BloomFilter::memory_bytes() const
{
  return bits_.size() * sizeof(uint64_t);
}

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

// Compact binary hash: one byte per cell, value biased by +128 so that
// negative heights map to positive chars. The result is a row-major string
// of length R*C — ~3× smaller than the old "val|val|..." text format.
std::string
hash_board(const Board& board)
{
  int rows = board.size();
  int cols = board[0].size();
  std::string hash(rows * cols, '\0');
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < cols; ++c)
      hash[r * cols + c] = static_cast<char>(board[r][c] + 128);
  return hash;
}

// Hash with height-offset normalization applied on-the-fly (no board copy).
// Subtracts zero_offset from each cell before encoding.
std::string
hash_board_zeroed(const Board& board, int zero_offset)
{
  int rows = board.size();
  int cols = board[0].size();
  std::string hash(rows * cols, '\0');
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < cols; ++c)
      hash[r * cols + c] = static_cast<char>(board[r][c] - zero_offset + 128);
  return hash;
}

// Rotation-without-copy: reads cells in rotated index order to produce a
// zeroed hash directly — avoids allocating a rotated Board entirely.
// 90° CW: new[r][c] = old[N-1-c][r] (square boards only)
std::string
hash_board_zeroed_rot90(const Board& board, int zero_offset)
{
  int n = board.size();
  std::string hash(n * n, '\0');
  for (int r = 0; r < n; ++r)
    for (int c = 0; c < n; ++c)
      hash[r * n + c] =
        static_cast<char>(board[n - 1 - c][r] - zero_offset + 128);
  return hash;
}

// 180°: new[r][c] = old[rows-1-r][cols-1-c] (works for any aspect ratio)
std::string
hash_board_zeroed_rot180(const Board& board, int zero_offset)
{
  int rows = board.size();
  int cols = board[0].size();
  std::string hash(rows * cols, '\0');
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < cols; ++c)
      hash[r * cols + c] = static_cast<char>(board[rows - 1 - r][cols - 1 - c] -
                                             zero_offset + 128);
  return hash;
}

// 270° CW: new[r][c] = old[c][N-1-r] (square boards only)
std::string
hash_board_zeroed_rot270(const Board& board, int zero_offset)
{
  int n = board.size();
  std::string hash(n * n, '\0');
  for (int r = 0; r < n; ++r)
    for (int c = 0; c < n; ++c)
      hash[r * n + c] =
        static_cast<char>(board[c][n - 1 - r] - zero_offset + 128);
  return hash;
}

// Inverse of hash_board: decodes the compact binary string back to a Board.
// Cast to unsigned char first to avoid sign-extension before subtracting bias.
Board
board_hash_to_array(const std::string& board_hash_str,
                    const int& rows,
                    const int& columns)
{
  Board board = generate_initial_board(rows, columns);
  for (int i = 0; i < rows * columns; ++i)
    board[i / columns][i % columns] =
      static_cast<unsigned char>(board_hash_str[i]) - 128;
  return board;
}

// Splits a board list into up to n chunks for parallel dispatch.
// Chunks may overlap slightly at boundaries due to integer rounding.
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

// Disk spill: writes hash strings to a timestamped file. Used when the
// in-memory set exceeds MAX_IN_MEM to keep memory bounded.
std::string
write_to_file(const std::vector<std::string>& results,
              const std::string& folder_name)
{
  std::string filename =
    folder_name + "_" + std::to_string(get_current_time_ms()) + ".bin";
  fs::path filepath = fs::current_path() / folder_name / filename;

  std::ofstream file(filepath);
  for (const std::string& result : results) {
    file << result << "\n";
  }

  return filename;
}

// Overload: hashes boards before writing (for increment spills).
std::string
write_to_file(const std::vector<Board>& new_increments,
              const std::string& folder_name)
{
  std::string filename =
    folder_name + "_" + std::to_string(get_current_time_ms()) + ".bin";
  fs::path filepath = fs::current_path() / folder_name / filename;

  std::ofstream file(filepath);
  for (const Board& new_increment : new_increments) {
    file << hash_board(new_increment) << "\n";
  }

  return filename;
}

// Dedup against on-disk result files. Iterates in reverse chronological order
// (newest files first) since recent rounds are most likely to contain
// duplicates of current candidates. Exits early once all candidates are
// resolved.
void
check_results_vs_files(
  const std::vector<std::string>& result_files,
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  std::vector<std::vector<uint8_t>>& is_new_checks,
  const int& n_jobs,
  const long long& max_in_mem)
{
  // Check if any candidates still need file verification
  auto has_uncertain = [&]() {
    for (size_t nj = 0; nj < static_cast<size_t>(n_jobs); ++nj)
      for (size_t j = 0; j < is_new_checks[nj].size(); ++j)
        if (is_new_checks[nj][j] == 1)
          return true;
    return false;
  };

  if (!has_uncertain())
    return;

  // Reverse iteration: newest files first (temporal locality)
  for (auto it = result_files.rbegin(); it != result_files.rend(); ++it) {
    const std::string& result_file = *it;

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
    for (size_t nj = 0; nj < static_cast<size_t>(n_jobs); ++nj) {
      std::vector<uint8_t> is_new_check_copy = is_new_checks[nj];
      check_futures.push_back(std::async(std::launch::async,
                                         check_results_vs_mp,
                                         std::cref(results_lists[nj]),
                                         std::move(is_new_check_copy),
                                         std::cref(temp_results)));
    }

    for (size_t nj = 0; nj < static_cast<size_t>(n_jobs); ++nj) {
      is_new_checks[nj] = check_futures[nj].get();
    }

    // Early exit: all candidates resolved, skip remaining files
    if (!has_uncertain())
      break;
  }
}

// Dedup against the in-memory result set (same parallel pattern as file check).
void
check_results_vs_results(
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  std::vector<std::vector<uint8_t>>& is_new_checks,
  const std::unordered_set<std::string>& results,
  const int& n_jobs)
{
  std::vector<std::future<std::vector<uint8_t>>> check_futures;
  check_futures.reserve(n_jobs);
  for (size_t nj = 0; nj < static_cast<size_t>(n_jobs); ++nj) {
    std::vector<uint8_t> is_new_check_copy = is_new_checks[nj];
    check_futures.push_back(std::async(std::launch::async,
                                       check_results_vs_mp,
                                       std::cref(results_lists[nj]),
                                       std::move(is_new_check_copy),
                                       std::cref(results)));
  }

  for (size_t nj = 0; nj < static_cast<size_t>(n_jobs); ++nj) {
    is_new_checks[nj] = check_futures[nj].get();
  }
}

// Per-thread worker: checks each candidate's hash against a set, flipping
// the is_new flag to 0 for any duplicate found.
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
                    long long start,
                    long long round_start,
                    long long new_board_count,
                    long long result_count)
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
