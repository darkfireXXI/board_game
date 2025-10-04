#ifndef BOARD_GAME_PERMUTATIONS_UTILS_H
#define BOARD_GAME_PERMUTATIONS_UTILS_H

#include <unordered_set>

using Board = std::vector<std::vector<int>>;

Board
zero_board(const Board& board);

Board
find_most_extreme_board(Board board);

std::optional<Board>
increment_board(Board board, const int& board_min, const int& board_max);

int
calculate_rounds(const Board& most_extreme_board,
                 const int& board_min,
                 const int& board_max,
                 bool print_boards = false);

std::tuple<bool, std::string>
check_board_is_new_combo(Board board,
                         const std::unordered_set<std::string>& results);

std::tuple<bool, std::string>
check_board_is_new_perm(Board board,
                        const std::unordered_set<std::string>& results);

std::unordered_map<std::string, Board>
generate_all_board_increments_combo(Board board,
                                    const int& board_min,
                                    const int& board_max);

std::unordered_map<std::string, Board>
generate_all_board_increments_perm(Board board,
                                   const int& board_min,
                                   const int& board_max);

std::vector<std::pair<Board, std::string>>
generate_boards_mp_combo(const std::vector<Board>& split_increments,
                         const int& board_min,
                         const int& board_max);

std::vector<std::pair<Board, std::string>>
generate_boards_mp_perm(const std::vector<Board>& split_increments,
                        const int& board_min,
                        const int& board_max);

#endif