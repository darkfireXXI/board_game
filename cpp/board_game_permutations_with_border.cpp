// Enumerates all unique board permutations for a bordered board via BFS.
// Same BFS as the combinations variant, but rotations are counted as distinct
// boards — only height-offset normalization (zeroing by min edge value) is
// used for dedup.

#include <thread>

#include "board_game_counting_utils.h"
#include "board_game_with_border_utils.h"

int
main(int argc, char* argv[])
{
  int rows = 2, columns = 2, n_jobs = 1, n_jobs_use = 1;

  for (size_t i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-r" || arg == "--rows") && i + 1 < argc) {
      rows = std::stoi(argv[++i]);
    } else if ((arg == "-c" || arg == "--columns") && i + 1 < argc) {
      columns = std::stoi(argv[++i]);
    } else if ((arg == "-n" || arg == "--n-jobs") && i + 1 < argc) {
      n_jobs = std::stoi(argv[++i]);
    }
  }

  const int CHUNK_SIZE = 35'000;
  const int MAX_IN_MEM = 20'000'000;

  fs::create_directory("new_increments");
  fs::create_directory("results");

  // Build the BFS starting state: the deepest possible bordered board.
  Board initial_board = generate_initial_board(rows, columns);
  Board dropped_board = drop_board(initial_board);
  int rounds = calculate_rounds(dropped_board, false);
  std::cout << rounds << " rounds for a " << rows << "x" << columns
            << " board\n";
  print_board(dropped_board);

  // BFS state: for permutations, rotations are distinct — larger result space.
  Board zeroed_board = zero_board(dropped_board);
  std::unordered_set<std::string> results = { hash_board(zeroed_board) };
  results.reserve(MAX_IN_MEM);
  std::vector<std::string> result_files;
  long long results_in_files = 0;
  std::vector<Board> last_round_increments = { dropped_board };
  std::vector<Board> new_increments;
  new_increments.reserve(MAX_IN_MEM);
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
    increment_files.emplace_back(filename);
  }

  long long start = get_current_time_ms();

  // BFS main loop: each round expands the frontier by trying +1 on every cell.
  for (size_t round = 1; round <= rounds; ++round) {
    long long round_start = get_current_time_ms();
    long long new_increments_in_files = 0;

    if (n_jobs > 1) {
      for (const std::string& filename : increment_files) {
        std::vector<Board> increments;
        increments.reserve(MAX_IN_MEM);

        fs::path filepath = fs::current_path() / "new_increments" / filename;

        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(file_size);
        file.read(buffer.data(), file_size);

        const char* data = buffer.data();
        size_t pos = 0;
        for (size_t i = 0; i < static_cast<size_t>(file_size); ++i) {
          if (data[i] == '\n') {
            size_t len = i - pos;
            if (len > 0) {
              std::string_view sv(data + pos, len);
              increments.push_back(
                board_hash_to_array(std::string(sv), rows, columns));
            }
            pos = i + 1;
          }
        }

        // Fall back to single-threaded for tiny batches (avoids thread
        // overhead)
        if (increments.size() < 100) {
          n_jobs_use = 1;
        }

        for (size_t i = 0; i < increments.size();
             i += CHUNK_SIZE * n_jobs_use) {
          size_t chunk_end =
            std::min(i + CHUNK_SIZE * n_jobs_use, increments.size());
          std::vector<Board> chunk(increments.begin() + i,
                                   increments.begin() + chunk_end);

          std::vector<std::vector<Board>> split_increments =
            split_list(chunk, n_jobs_use);

          std::vector<std::future<std::vector<std::pair<Board, std::string>>>>
            calc_futures;
          calc_futures.reserve(n_jobs_use);
          for (const std::vector<Board>& split : split_increments) {
            calc_futures.push_back(
              std::async(std::launch::async, generate_boards_mp_perm, split));
          }

          std::vector<std::vector<std::pair<Board, std::string>>> results_lists;
          results_lists.reserve(n_jobs_use);
          for (auto& calc_future : calc_futures) {
            results_lists.push_back(calc_future.get());
          }

          std::vector<std::vector<uint8_t>> is_new_checks =
            check_results_vs_files(
              result_files, results_lists, n_jobs_use, MAX_IN_MEM);

          for (size_t nj = 0; nj < n_jobs_use; ++nj) {
            for (size_t j = 0; j < results_lists[nj].size(); ++j) {
              if (is_new_checks[nj][j]) {
                const auto& [board_increment, min_board_hash] =
                  results_lists[nj][j];
                auto [it, inserted] = results.insert(min_board_hash);
                if (inserted) {
                  new_increments.push_back(std::move(board_increment));
                }
              }
            }
          }

          // dump excess results to txt file
          while (results.size() >= MAX_IN_MEM) {
            std::vector<std::string> items_to_write;
            items_to_write.reserve(MAX_IN_MEM);

            std::unordered_set<std::string>::iterator it = results.begin();
            size_t j = 0;
            while (j < MAX_IN_MEM && it != results.end()) {
              items_to_write.push_back(std::move(*it));
              it = results.erase(it);
              ++j;
            }

            std::string filename = write_to_file(items_to_write, "results");
            result_files.push_back(filename);
            results_in_files += j;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }

          // dump excess new increments to txt file
          if (new_increments.size() >= MAX_IN_MEM) {
            std::string filename =
              write_to_file(new_increments, "new_increments");
            new_increments_in_files += new_increments.size();
            new_increments.clear();
            new_increments.reserve(MAX_IN_MEM);
            new_increment_files.push_back(filename);
          }
        }
      }

      if (new_increments.size() > 0) {
        std::string filename = write_to_file(new_increments, "new_increments");
        new_increments_in_files += new_increments.size();
        new_increments.clear();
        new_increments.reserve(MAX_IN_MEM);
        new_increment_files.push_back(filename);
      }

      increment_files = new_increment_files;
      new_increment_files.clear();

      n_jobs_use = n_jobs;

    } else {
      // === SINGLE-THREADED PATH ===
      for (std::size_t i = 0; i < last_round_increments.size(); ++i) {
        Board last_round_new_increment = last_round_increments[i];
        std::unordered_map<std::string, Board> board_increments =
          generate_all_board_increments_perm(last_round_new_increment);

        for (std::unordered_map<std::string, Board>::iterator it =
               board_increments.begin();
             it != board_increments.end();
             ++it) {
          Board board_increment = it->second;
          auto [is_new_perm, min_board_hash] =
            check_board_is_new_perm(board_increment, results);

          if (is_new_perm) {
            results.insert(min_board_hash);
            new_increments.push_back(std::move(board_increment));
          }
        }
      }
    }

    long long new_board_count = new_increments_in_files + new_increments.size();
    long long result_count = results_in_files + results.size();

    display_round_stats(
      round, rounds, start, round_start, new_board_count, result_count);

    // Advance BFS frontier
    last_round_increments = new_increments;
    new_increments.clear();

    // Early termination: all reachable states found
    if (new_board_count == 0)
      break;
  }

  long long end = get_current_time_ms();

  double seconds_elapsed = (end - start) / 1000.0;
  double hours_elapsed = seconds_elapsed / 3600.0;

  std::cout << std::fixed << std::setprecision(1);

  std::cout << hours_elapsed << " hours or ";
  std::cout << seconds_elapsed << " seconds";
  std::cout << std::endl;

  std::cout << (results_in_files + results.size())
            << " total unique boards found";
  std::cout << std::endl;
}
