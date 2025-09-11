#ifndef BOARD_GAME_PERMUTATIONS_UTILS_H
#define BOARD_GAME_PERMUTATIONS_UTILS_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <unordered_set>
#include <vector>

using Board = std::vector<std::vector<int>>;

namespace fs = std::filesystem;

long long
get_current_time_ms();

void
print_board(const Board& board);

Board
rotate_board_90(Board board);

Board
generate_initial_board(const int& size);

std::string
hash_board(const Board& board);

Board
board_hash_to_array(const std::string& board_hash_str, int size);

std::vector<std::vector<Board>>
split_list(const std::vector<Board>& list, int n);

std::string
write_to_file(const std::unordered_set<std::string>& results,
              const std::string& folder_name);

std::string
write_to_file(const std::vector<Board>& new_increments,
              const std::string& folder_name);

std::vector<std::vector<uint8_t>>
check_results_vs_files(
  const std::vector<std::string>& result_files,
  const std::vector<std::vector<std::pair<Board, std::string>>>& results_lists,
  const int& n_jobs,
  const long long& max_in_mem);

std::vector<uint8_t>
check_results_vs_file_mp(
  const std::vector<std::pair<Board, std::string>>& results_list,
  std::vector<uint8_t> is_new_check,
  const std::unordered_set<std::string>& temp_results);

long long
get_file_item_count(std::vector<std::string> files, std::string folder_name);

void
display_round_stats(int round,
                    int rounds,
                    std::time_t start,
                    std::time_t round_start,
                    int new_board_count,
                    int result_count);

#endif