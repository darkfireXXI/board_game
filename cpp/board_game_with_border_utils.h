#ifndef BOARD_GAME_PERMUTATIONS_UTILS_H
#define BOARD_GAME_PERMUTATIONS_UTILS_H

#include <unordered_set>

using Board = std::vector<std::vector<int>>;

Board
zero_board(const Board& board);

Board
drop_board(Board board);

std::optional<Board>
increment_board(Board board);

int
calculate_rounds(const Board& dropped_board, bool print_boards = false);

std::tuple<bool, std::string>
check_board_is_new_combo(Board board,
                         const std::unordered_set<std::string>& results);

std::tuple<bool, std::string>
check_board_is_new_perm(Board board,
                        const std::unordered_set<std::string>& results);

std::unordered_map<std::string, Board>
generate_all_board_increments_combo(Board board);

std::unordered_map<std::string, Board>
generate_all_board_increments_perm(Board board);

std::vector<std::pair<Board, std::string>>
generate_boards_mp_combo(const std::vector<Board>& split_increments);

std::vector<std::pair<Board, std::string>>
generate_boards_mp_perm(const std::vector<Board>& split_increments);
#endif