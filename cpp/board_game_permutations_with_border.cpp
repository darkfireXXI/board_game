// Enumerates all unique board permutations for a bordered board via BFS.
//
// Algorithm overview:
//   Same BFS structure as the combinations variant, but rotations are counted
//   as distinct boards — only height-offset normalization (zeroing by min edge
//   value) is used for dedup. This produces more unique results than combos.
//
// Deduplication strategy (3-tier):
//   1. In-memory hash set: O(1) lookup for the most recent results.
//   2. Bloom filter: probabilistic filter over ALL known results (in-memory +
//      on-disk). If Bloom says "definitely not seen," we skip the expensive
//      disk check entirely. False positives go to the disk-check path.
//   3. Disk files: when in-memory set exceeds MAX_IN_MEM, results spill to
//      disk. Candidates that survive tiers 1-2 are batch-checked against files.
//
// Memory management:
//   Survivor vectors (candidates awaiting disk check) are periodically flushed
//   when they exceed MAX_SURVIVORS to bound peak RAM usage.

#include <thread>

#include "board_game_counting_utils.h"
#include "board_game_with_border_utils.h"

int
main(int argc, char* argv[])
{
  int rows = 2, columns = 2, n_jobs = 1, n_jobs_to_use = 1;

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

  // --- Tuning constants ---
  // CHUNK_SIZE: number of boards each thread processes per batch.
  // MAX_IN_MEM: max results kept in the in-memory hash set before spilling to
  //   disk. Larger = fewer disk files = faster file checks, but more RAM.
  // MAX_SURVIVORS: max candidates accumulated before triggering a flush to
  //   prevent unbounded memory growth. Larger = fewer flushes = better batch
  //   amortization of disk I/O, but more RAM.
  // BLOOM_EXPECTED_ITEMS: expected total unique boards for Bloom filter sizing.
  //   At 1% FPR this uses ~228 MB. Oversizing is safe (wastes some RAM);
  //   undersizing increases false positives (more unnecessary disk checks).
  const int CHUNK_SIZE = 35'000;
  const int MAX_IN_MEM = 30'000'000;
  const int MAX_SURVIVORS = 7'500'000;
  const long long BLOOM_EXPECTED_ITEMS = 200'000'000;

  fs::create_directory("new_increments");
  fs::create_directory("results");

  // --- BFS starting state ---
  // Generate the most extreme (lowest) board: all cells start at their minimum
  // valid height given the border constraint. This is the BFS root.
  Board initial_board = generate_initial_board(rows, columns);
  Board dropped_board = drop_board(initial_board);
  int rounds = calculate_rounds(dropped_board, false);
  std::cout << rounds << " rounds for a " << rows << "x" << columns
            << " board\n";
  print_board(dropped_board);

  // --- Deduplication state initialization ---
  // Zero the board (subtract min edge value) to get its canonical form, then
  // hash it. The Bloom filter and in-memory set both start with this seed.
  Board zeroed_board = zero_board(dropped_board);
  std::string initial_hash = hash_board(zeroed_board);
  BloomFilter bloom(BLOOM_EXPECTED_ITEMS);
  bloom.insert(initial_hash);
  std::cout << "Bloom filter: " << bloom.memory_bytes() / (1024 * 1024)
            << " MB\n";
  std::unordered_set<std::string> results = { initial_hash };
  results.reserve(MAX_IN_MEM);
  std::vector<std::string> result_files;
  long long results_in_files = 0;
  std::vector<Board> last_round_increments = { dropped_board };
  std::vector<Board> new_increments;
  new_increments.reserve(MAX_IN_MEM);
  std::vector<std::string> increment_files;
  std::vector<std::string> new_increment_files;

  // For multi-threaded mode, serialize the initial frontier to disk so worker
  // threads can read it independently without shared-memory coordination.
  if (n_jobs > 1) {
    long long now = get_current_time_ms();

    std::string filename = "new_increments_" + std::to_string(now) + ".bin";
    fs::path full_path = fs::current_path() / "new_increments" / filename;

    std::ofstream file(full_path);
    for (const Board& increment : last_round_increments) {
      file << hash_board(increment) << "\n";
    }

    file.close();
    increment_files.emplace_back(filename);
  }

  long long start = get_current_time_ms();

  // === BFS MAIN LOOP ===
  // Each round expands the frontier: for every board from the previous round,
  // try incrementing each cell by +1, validate constraints, deduplicate, and
  // collect genuinely new boards as the next round's frontier.
  for (size_t round = 1; round <= rounds; ++round) {
    long long round_start = get_current_time_ms();
    long long new_increments_in_files = 0;
    long long time_file_read = 0, time_board_gen = 0, time_mem_check = 0;
    long long time_file_check = 0, time_insertion = 0, time_disk_spill = 0;
    long long bloom_skipped = 0;

    // === MULTI-THREADED PATH ===
    if (n_jobs > 1) {
      // Candidates are triaged into two buckets after in-memory dedup:
      //   bloom_confirmed: Bloom says "definitely new" — skip disk check,
      //     insert directly. (No false negatives in a Bloom filter.)
      //   all_survivors: Bloom says "maybe seen" — must check against disk
      //     files to confirm. These are batched for efficiency.
      std::vector<std::pair<Board, std::string>> all_survivors;
      std::vector<std::pair<Board, std::string>> bloom_confirmed;

      // --- flush_survivors: periodic batch-flush to bound memory ---
      // When accumulated candidates exceed MAX_SURVIVORS, this lambda:
      //   1. Checks all_survivors against on-disk result files (batch I/O).
      //   2. Inserts confirmed-new boards into results set + Bloom filter.
      //   3. Spills results/increments to disk if they exceed MAX_IN_MEM.
      // Batching amortizes file I/O: one scan per file eliminates many
      // candidates at once, rather than checking one-at-a-time.
      auto flush_survivors = [&]() {
        long long tf3 = get_current_time_ms();

        if (!all_survivors.empty() && !result_files.empty()) {
          // Distribute survivors across n_jobs lists for parallel file checking.
          std::vector<std::vector<std::pair<Board, std::string>>> survivor_lists(
            n_jobs);
          for (size_t si = 0; si < all_survivors.size(); ++si)
            survivor_lists[si % n_jobs].push_back(
              std::move(all_survivors[si]));
          all_survivors.clear();

          std::vector<std::vector<uint8_t>> survivor_checks;
          for (const auto& sl : survivor_lists)
            survivor_checks.emplace_back(sl.size(), 1);

          // Batch check against all on-disk result files. Each file is read
          // once and scanned against all remaining survivors. As survivors are
          // eliminated by earlier files, later files have fewer to check.
          check_results_vs_files(
            result_files, survivor_lists, survivor_checks, n_jobs, MAX_IN_MEM);

          time_file_check += get_current_time_ms() - tf3;
          long long tf4 = get_current_time_ms();

          // Insert bloom_confirmed candidates (Bloom said "definitely new").
          for (auto& [board_increment, min_board_hash] : bloom_confirmed) {
            auto [it, inserted] = results.insert(std::move(min_board_hash));
            if (inserted) {
              bloom.insert(*it);
              new_increments.push_back(std::move(board_increment));
            }
          }
          bloom_confirmed.clear();

          // Insert survivors that passed the disk file check.
          for (size_t nj = 0; nj < static_cast<size_t>(n_jobs); ++nj) {
            for (size_t sj = 0; sj < survivor_lists[nj].size(); ++sj) {
              if (survivor_checks[nj][sj]) {
                auto& [board_increment, min_board_hash] =
                  survivor_lists[nj][sj];
                auto [it, inserted] =
                  results.insert(std::move(min_board_hash));
                if (inserted) {
                  bloom.insert(*it);
                  new_increments.push_back(std::move(board_increment));
                }
              }
            }
          }

          time_insertion += get_current_time_ms() - tf4;
        } else {
          // No disk files yet — all candidates only need in-memory dedup.
          time_file_check += get_current_time_ms() - tf3;
          long long tf4 = get_current_time_ms();

          for (auto& [board_increment, min_board_hash] : bloom_confirmed) {
            auto [it, inserted] = results.insert(std::move(min_board_hash));
            if (inserted) {
              bloom.insert(*it);
              new_increments.push_back(std::move(board_increment));
            }
          }
          bloom_confirmed.clear();

          for (auto& [board_increment, min_board_hash] : all_survivors) {
            auto [it, inserted] = results.insert(std::move(min_board_hash));
            if (inserted) {
              bloom.insert(*it);
              new_increments.push_back(std::move(board_increment));
            }
          }
          all_survivors.clear();

          time_insertion += get_current_time_ms() - tf4;
        }

        // --- Disk spill: prevent in-memory set from exceeding RAM budget ---
        long long tf5 = get_current_time_ms();

        while (results.size() >= MAX_IN_MEM) {
          std::vector<std::string> items_to_write;
          items_to_write.reserve(MAX_IN_MEM);

          std::unordered_set<std::string>::iterator it = results.begin();
          size_t sj = 0;
          while (sj < MAX_IN_MEM && it != results.end()) {
            items_to_write.push_back(std::move(*it));
            it = results.erase(it);
            ++sj;
          }

          std::string fn = write_to_file(items_to_write, "results");
          result_files.push_back(fn);
          results_in_files += sj;

          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (new_increments.size() >= MAX_IN_MEM) {
          std::string fn = write_to_file(new_increments, "new_increments");
          new_increments_in_files += new_increments.size();
          new_increments.clear();
          new_increment_files.push_back(fn);
        }

        time_disk_spill += get_current_time_ms() - tf5;
      };

      // --- Process each increment file from the previous round ---
      for (const std::string& filename : increment_files) {
        std::vector<Board> increments;
        increments.reserve(MAX_IN_MEM);

        // Bulk file read: load entire file into memory buffer, then parse.
        // Avoids per-line I/O syscall overhead.
        long long t0 = get_current_time_ms();

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

        time_file_read += get_current_time_ms() - t0;

        if (increments.size() < 100) {
          n_jobs_to_use = 1;
        }

        // --- Process increments in chunks across threads ---
        for (size_t i = 0; i < increments.size();
             i += CHUNK_SIZE * n_jobs_to_use) {
          size_t chunk_end =
            std::min(i + CHUNK_SIZE * n_jobs_to_use, increments.size());
          std::vector<Board> chunk(increments.begin() + i,
                                   increments.begin() + chunk_end);

          std::vector<std::vector<Board>> split_increments =
            split_list(chunk, n_jobs_to_use);

          // STEP 1: Board generation (parallel).
          // Each thread takes a subset of parent boards and generates all valid
          // +1 increments, returning (board, canonical_hash) pairs. For perms,
          // the canonical hash is just the zeroed board hash (no rotation dedup).
          long long t1 = get_current_time_ms();

          std::vector<std::future<std::vector<std::pair<Board, std::string>>>>
            calc_futures;
          calc_futures.reserve(n_jobs_to_use);
          for (const std::vector<Board>& split : split_increments) {
            calc_futures.push_back(
              std::async(std::launch::async, generate_boards_mp_perm, split));
          }

          std::vector<std::vector<std::pair<Board, std::string>>> results_lists;
          results_lists.reserve(n_jobs_to_use);
          for (auto& calc_future : calc_futures) {
            results_lists.push_back(calc_future.get());
          }

          time_board_gen += get_current_time_ms() - t1;

          // STEP 2: In-memory dedup check.
          // Check each candidate's hash against the in-memory results set.
          // This is O(1) per lookup and eliminates most duplicates cheaply.
          std::vector<std::vector<uint8_t>> is_new_checks;
          is_new_checks.reserve(n_jobs_to_use);
          for (const auto& rl : results_lists)
            is_new_checks.emplace_back(rl.size(), 1);

          long long t2 = get_current_time_ms();

          check_results_vs_results(
            results_lists, is_new_checks, results, n_jobs_to_use);

          time_mem_check += get_current_time_ms() - t2;

          // STEP 3: Bloom filter triage.
          // Candidates that survived in-memory check are tested against the
          // Bloom filter (covers ALL known results, including on-disk).
          //   - Bloom negative ("definitely new"): goes to bloom_confirmed,
          //     will be inserted without any disk I/O.
          //   - Bloom positive ("maybe seen"): goes to all_survivors, must be
          //     batch-checked against disk files in flush_survivors.
          for (size_t nj = 0; nj < n_jobs_to_use; ++nj) {
            for (size_t j = 0; j < results_lists[nj].size(); ++j) {
              if (is_new_checks[nj][j]) {
                if (bloom.possibly_contains(results_lists[nj][j].second)) {
                  all_survivors.push_back(std::move(results_lists[nj][j]));
                } else {
                  bloom_confirmed.push_back(std::move(results_lists[nj][j]));
                  ++bloom_skipped;
                }
              }
            }
          }

          // Periodic flush to prevent unbounded memory growth.
          if (all_survivors.size() + bloom_confirmed.size() >= MAX_SURVIVORS)
            flush_survivors();
        }
      }

      // Final flush: process any remaining candidates after all chunks.
      flush_survivors();

      // Write any remaining new increments to disk for the next round.
      if (new_increments.size() > 0) {
        std::string filename = write_to_file(new_increments, "new_increments");
        new_increments_in_files += new_increments.size();
        new_increments.clear();
        new_increment_files.push_back(filename);
      }

      increment_files = new_increment_files;
      new_increment_files.clear();

      n_jobs_to_use = n_jobs;

    } else {
      // === SINGLE-THREADED PATH ===
      // Simpler: generate all increments and check directly against the
      // in-memory set. No disk spill, no Bloom filter, no batching needed.
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

    if (n_jobs > 1) {
      long long total_step_time = time_file_read + time_board_gen +
                                  time_mem_check + time_file_check +
                                  time_insertion + time_disk_spill;
      std::cout << "\tStep timing (seconds):\n";
      std::cout << "\t  file read:   " << time_file_read / 1000.0 << "\n";
      std::cout << "\t  board gen:   " << time_board_gen / 1000.0 << "\n";
      std::cout << "\t  mem check:   " << time_mem_check / 1000.0 << "\n";
      std::cout << "\t  file check:  " << time_file_check / 1000.0 << "\n";
      std::cout << "\t  insertion:   " << time_insertion / 1000.0 << "\n";
      std::cout << "\t  disk spill:  " << time_disk_spill / 1000.0 << "\n";
      std::cout << "\t  accounted:   " << total_step_time / 1000.0 << "\n";
      std::cout << "\t  bloom skip:  " << bloom_skipped << " candidates\n";
      std::cout << std::endl;
    }

    // Advance BFS frontier: this round's new boards become next round's input.
    last_round_increments = new_increments;
    new_increments.clear();

    // Early termination: no new boards means all reachable states are found.
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
