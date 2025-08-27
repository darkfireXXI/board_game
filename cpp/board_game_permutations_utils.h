#ifndef BOARD_GAME_PERMUTATIONS_UTILS_H
#define BOARD_GAME_PERMUTATIONS_UTILS_H

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <vector>

using Board = std::vector<std::vector<int>>;

void print_board(const Board &board);

Board rotate_board_90(Board board);

Board generate_initial_board(const int &size);

std::string hash_board(const Board &board);

int get_file_item_count(std::vector<std::string> files,
                        std::string folder_name);

void display_round_stats(int round, int rounds, std::time_t start,
                         std::time_t round_start, int new_board_count,
                         int result_count);

#endif