// Shared utilities for board game enumeration.
// Provides hashing (compact binary encoding), board I/O, rotation helpers,
// multi-threaded dedup checking, and disk-spill management used by all four
// main scripts (combinations/permutations × with/without border).

#ifndef BOARD_GAME_COUNTING_UTILS_H
#define BOARD_GAME_COUNTING_UTILS_H

#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <unordered_set>
#include <vector>

// R×C grid of integer heights — each cell represents terrain elevation.
using Board = std::vector<std::vector<int>>;

namespace fs = std::filesystem;

// --- Bloom filter ---
// Probabilistic set membership test: possibly_contains() may return true for
// items never inserted (false positive) but never returns false for items that
// were inserted. Used to skip expensive disk file checks for candidates that
// are definitely new.

class BloomFilter
{
  std::vector<uint64_t> bits_;
  size_t num_bits_;
  int num_hashes_;

  std::pair<uint64_t, uint64_t> hash_pair(const std::string& key) const;

public:
  BloomFilter(size_t expected_items, double fp_rate = 0.01);
  void insert(const std::string& key);
  bool possibly_contains(const std::string& key) const;
  size_t memory_bytes() const;
};

// --- General helpers ---

long long
get_current_time_ms();

void
print_board(const Board& board);

// Creates an all-zero R×C board (the BFS starting point before
// extremification).
Board
generate_initial_board(const int& rows, const int& columns);

// Physical rotation helpers (allocate new boards). Used only in legacy paths;
// the hash_board_zeroed_rot* functions below are preferred for dedup.
Board
rotate_board_90(Board board);

Board
rotate_board_180(Board board);

// --- Compact binary hashing ---
// Each cell is encoded as one byte: (value + 128). This gives ~3× smaller
// hash strings vs the old "val|val|..." text format. Values stay in [0,255]
// and avoid '\n' (the file delimiter) for any realistic board height.

std::string
hash_board(const Board& board);

// Hash with height-offset normalization: subtracts zero_offset before encoding.
std::string
hash_board_zeroed(const Board& board, int zero_offset);

// Zeroed hash reading cells in rotated order — no board copy allocated.
// rot90/rot270 are only valid for square boards.
std::string
hash_board_zeroed_rot90(const Board& board, int zero_offset);

std::string
hash_board_zeroed_rot180(const Board& board, int zero_offset);

std::string
hash_board_zeroed_rot270(const Board& board, int zero_offset);

// Inverse of hash_board: decodes a compact hash string back into a Board.
Board
board_hash_to_array(const std::string& board_hash_str,
                    const int& rows,
                    const int& columns);

// --- Multi-threading and disk spill ---

// Divides a board list into n roughly-equal chunks for parallel processing.
std::vector<std::vector<Board>>
split_list(const std::vector<Board>& list, int n);

// Spill to disk: writes hashes/boards to a timestamped file when the in-memory
// set exceeds MAX_IN_MEM. Returns the filename for later re-reading.
std::string
write_to_file(const std::vector<std::string>& results,
              const std::string& folder_name);

std::string
write_to_file(const std::vector<Board>& new_increments,
              const std::string& folder_name);

// Checks candidate boards against on-disk result files. Iterates files in
// reverse chronological order (newest first — most likely to contain recent
// duplicates) and exits early once all candidates are resolved.
// Modifies is_new_checks in-place: flips 1→0 for duplicates found on disk.
void
check_results_vs_files(
  const std::vector<std::string>& result_files,
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  std::vector<std::vector<uint8_t>>& is_new_checks,
  const int& n_jobs,
  const long long& max_in_mem);

// Checks candidates against the in-memory result set in parallel.
// Modifies is_new_checks in-place: flips 1→0 for duplicates found in memory.
void
check_results_vs_results(
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  std::vector<std::vector<uint8_t>>& is_new_checks,
  const std::unordered_set<std::string>& results,
  const int& n_jobs);

// Per-thread worker: marks candidates as duplicates if found in the given set.
std::vector<uint8_t>
check_results_vs_mp(
  const std::vector<std::pair<Board, std::string>>& results_list,
  std::vector<uint8_t> is_new_check,
  const std::unordered_set<std::string>& temp_results);

// Counts items in spilled files by reading line-by-line (increments) or
// sampling one file and multiplying (results, since all files are same size).
long long
get_file_item_count(std::vector<std::string> files, std::string folder_name);

void
display_round_stats(int round,
                    int rounds,
                    long long start,
                    long long round_start,
                    long long new_board_count,
                    long long result_count);

#endif