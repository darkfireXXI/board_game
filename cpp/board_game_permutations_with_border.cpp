#include <chrono>
#include <thread>

#include "board_game_permutations_utils.h"

Board
zero_board(const Board& board)
{
  int size = board.size();
  int min_edge = std::numeric_limits<int>::max();

  // find min of all edge values (top, bottom, left, right)
  for (size_t i = 0; i < size; ++i) {
    min_edge = std::min(min_edge, board[0][i]);
    min_edge = std::min(min_edge, board[size - 1][i]);
    if (i > 0 && i < size - 1) {
      min_edge = std::min(min_edge, board[i][0]);
      min_edge = std::min(min_edge, board[i][size - 1]);
    }
  }

  // create a new board with values subtracted
  Board zeroed_board(board);
  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < size; ++j) {
      zeroed_board[i][j] -= min_edge;
    }
  }

  return zeroed_board;
}

Board
drop_board(Board board)
{
  int size = (int)board.size();
  int rings = size / 2 + (size % 2 != 0);
  for (size_t i = 0; i < rings; ++i) {
    int val = -2 * (i + 1);
    for (size_t c = i; c < size - i; ++c) {
      board[i][c] = val;
      board[size - i - 1][c] = val;
    }
    for (size_t r = i; r < size - i; ++r) {
      board[r][i] = val;
      board[r][size - i - 1] = val;
    }
  }
  return board;
}

std::optional<Board>
increment_board(Board board)
{
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
      for (size_t i = 0; i < size; ++i) {
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
  Board rotation = zero_board(board);

  std::vector<std::string> hashes(4);
  hashes[0] = hash_board(rotation);

  for (size_t i = 1; i < 4; ++i) {
    rotation = rotate_board_90(rotation);
    hashes[i] = hash_board(rotation);
  }

  std::sort(hashes.begin(), hashes.end());
  if (results.count(hashes[0]) > 0) {
    return { false, "" };
  }

  return { true, hashes[0] };
}

std::unordered_map<std::string, Board>
generate_all_board_increments(Board input_board)
{
  int size = input_board.size();
  std::unordered_set<std::string> board_hashes;
  std::unordered_map<std::string, Board> board_increments;

  Board board = input_board;

  for (int r = 0; r < size; ++r) {
    for (int c = 0; c < size; ++c) {
      board[r][c] += 1;

      const int val = board[r][c];

      bool local_valid =
        std::abs(board[std::max(0, r - 1)][c] - val) <= 2 &&
        std::abs(board[std::min(size - 1, r + 1)][c] - val) <= 2 &&
        std::abs(board[r][std::max(0, c - 1)] - val) <= 2 &&
        std::abs(board[r][std::min(size - 1, c + 1)] - val) <= 2;

      bool boundary_valid = true;
      for (size_t i = 0; i < size; ++i) {
        if (board[0][i] > 2 || board[size - 1][i] > 2 || board[i][0] > 2 ||
            board[i][size - 1] > 2) {
          boundary_valid = false;
          break;
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

std::vector<std::pair<Board, std::string>>
generate_boards_mp(const std::vector<Board>& split_increments)
{
  std::vector<std::pair<Board, std::string>> new_boards;
  int size = split_increments[0].size();
  new_boards.reserve(split_increments.size() * size * size);

  std::vector<std::string> hashes(4);
  for (const Board& board : split_increments) {
    std::unordered_map<std::string, Board> increments =
      generate_all_board_increments(board);

    for (const std::pair<const std::string, Board>& entry : increments) {
      const Board& board_increment = entry.second;
      Board rotation = zero_board(board_increment);

      hashes[0] = hash_board(rotation);

      for (size_t i = 1; i < 4; ++i) {
        rotation = rotate_board_90(rotation);
        hashes[i] = hash_board(rotation);
      }

    std:
      sort(hashes.begin(), hashes.end());
      new_boards.emplace_back(board_increment, hashes[0]);
    }
  }

  return new_boards;
}

int
main(int argc, char* argv[])
{
  int size = 2, n_jobs = 1;

  for (size_t i = 1; i < argc; ++i) {
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
  std::cout << rounds << " rounds for a " << size << "x" << size << " board\n";

  Board zeroed_board = zero_board(dropped_board);
  std::unordered_set<std::string> results = { hash_board(zeroed_board) };
  std::vector<std::string> result_files;
  std::vector<Board> last_round_increments = { dropped_board };
  std::vector<Board> new_increments;
  std::vector<std::string> increment_files;
  std::vector<std::string> new_increment_files;

  if (n_jobs > 1) {
    long long now = get_current_time_ms();

    std::string filename = "new_increments_" + std::to_string(now) + ".txt";
    fs::path full_path = fs::current_path() / "new_increments" / filename;

    std::ofstream file(full_path);
    for (const Board& increment : last_round_increments) {
      file << hash_board(increment) << "\n";
    }

    file.close();
    increment_files.push_back(filename);
  }

  int CHUNK_SIZE = 10'000;
  int MAX_IN_MEM = 20'000'000;

  long long start = get_current_time_ms();

  for (size_t round = 1; round <= rounds; ++round) {
    long long round_start = get_current_time_ms();

    if (n_jobs > 1) {
      for (const std::string& filename : increment_files) {
        fs::path filepath = fs::current_path() / "new_increments" / filename;
        std::ifstream file(filepath);
        std::vector<std::string> lines;
        lines.reserve(MAX_IN_MEM);
        std::string line;
        while (std::getline(file, line)) {
          lines.emplace_back(std::move(line));
        }

        std::vector<Board> increments;
        increments.reserve(lines.size());
        std::transform(lines.begin(),
                       lines.end(),
                       std::back_inserter(increments),
                       [&size](const std::string& hash_str) {
                         return board_hash_to_array(hash_str, size);
                       });

        for (size_t i = 0; i < increments.size(); i += CHUNK_SIZE * n_jobs) {
          size_t chunk_end =
            std::min(i + CHUNK_SIZE * n_jobs, increments.size());
          std::vector<Board> chunk(increments.begin() + i,
                                   increments.begin() + chunk_end);

          std::vector<std::vector<Board>> split_increments =
            split_list(chunk, n_jobs);

          std::vector<std::future<std::vector<std::pair<Board, std::string>>>>
            calc_futures;
          for (const std::vector<Board>& split : split_increments) {
            calc_futures.push_back(
              std::async(std::launch::async, generate_boards_mp, split));
          }

          std::vector<std::vector<std::pair<Board, std::string>>> results_lists;
          results_lists.reserve(n_jobs);
          for (auto& calc_future : calc_futures) {
            results_lists.push_back(calc_future.get());
          }

          // std::vector<std::vector<bool>> is_new_checks;
          // is_new_checks.reserve(n_jobs);
          // for (const auto &result_chunk : results_lists) {
          //   is_new_checks.emplace_back(result_chunk.size(), true);
          // }

          std::vector<std::vector<bool>> is_new_checks = check_results_vs_files(
            result_files, results_lists, n_jobs, MAX_IN_MEM);
          // for (const std::string &result_file : result_files) {
          //   fs::path filepath = fs::current_path() / "results" / result_file;
          //   std::ifstream file(filepath);

          //   std::unordered_set<std::string> temp_results;
          //   temp_results.reserve(MAX_IN_MEM);
          //   std::string line;
          //   while (std::getline(file, line)) {
          //     temp_results.insert(line);
          //   }

          //   std::vector<std::future<std::vector<bool>>> check_futures;
          //   for (size_t nj = 0; nj < n_jobs; ++nj) {
          //     check_futures.push_back(
          //         std::async(std::launch::async, check_results_vs_file_mp,
          //                    std::cref(results_lists[nj]), is_new_checks[nj],
          //                    std::cref(temp_results)));
          //   }

          //   for (size_t nj = 0; nj < n_jobs; ++nj) {
          //     is_new_checks[nj] = check_futures[nj].get();
          //   }
          // }

          for (size_t nj = 0; nj < n_jobs; ++nj) {
            for (size_t j = 0; j < results_lists[nj].size(); ++j) {
              if (is_new_checks[nj][j]) {
                const auto& [board_increment, min_board_hash] =
                  results_lists[nj][j];
                if (results.count(min_board_hash) == 0) {
                  results.insert(min_board_hash);
                  new_increments.push_back(board_increment);
                }
              }
            }
          }

          // dump excess results to txt file
          while (results.size() >= MAX_IN_MEM) {
            std::unordered_set<std::string> items_to_write;
            std::unordered_set<std::string>::iterator it = results.begin();
            size_t j = 0;
            while (j < MAX_IN_MEM && it != results.end()) {
              items_to_write.insert(*it);
              ++j;
              ++it;
            }

            std::string filename = write_to_file(items_to_write, "results");

            for (const std::string& item : items_to_write) {
              results.erase(item);
            }

            result_files.push_back(filename);

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }

          // dump excess new increments to txt file
          if (new_increments.size() >= MAX_IN_MEM) {
            std::string filename =
              write_to_file(new_increments, "new_increments");
            new_increments.clear();
            new_increment_files.push_back(filename);
          }
        }
      }

      if (new_increments.size() > 0) {
        std::string filename = write_to_file(new_increments, "new_increments");
        new_increments.clear();
        new_increment_files.push_back(filename);
      }

      increment_files = new_increment_files;
      new_increment_files.clear();

    } else {
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
    }

    long long new_board_count =
      get_file_item_count(increment_files, "new_increments");
    new_board_count += new_increments.size();
    long long result_count = get_file_item_count(result_files, "results");
    result_count += results.size();

    std::time_t now = std::time(nullptr);
    display_round_stats(
      round, rounds, start, round_start, new_board_count, result_count);

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

  int result_count = get_file_item_count(result_files, "results");
  std::cout << (result_count + results.size()) << " total unique boards found";
  std::cout << std::endl;
}
