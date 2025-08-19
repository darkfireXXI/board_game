import multiprocessing as mp
import time

import numpy as np
from tqdm import tqdm


def generate_initial_board(size):
    initial_board = np.zeros((size, size))
    return initial_board

def zero_board(board):
    size = len(board)
    edges = np.concatenate([board[0, :], board[size - 1, :], board[:, 0], board[:, size - 1]])
    board -= min(edges)
    return board


def board_hash(board):
    size = len(board)
    board_str = ""
    for r in range(size):
        for c in range(size):
            board_str += f"{r},{c},{board[r, c]}|"

    return board_str


def check_board_is_new_combo(board, results):
    zeroed_board = zero_board(board.copy())
    rot_boards = [zeroed_board]
    rot_board_hashes = [board_hash(zeroed_board)]
    for _ in range(3):
        rot_board = np.rot90(rot_boards[-1])
        rot_boards.append(rot_board)
        rot_board_hashes.append(board_hash(rot_board))

    for rot_board_hash in rot_board_hashes:
        # if results.get(rot_board_hash) is not None:
        if rot_board_hash in results:
            return False

    return True


def split_list(list_, n):
    split = max(int(len(list_) / n), 1)
    splits = []
    for i in range(n):
        splits.append(list_[i + (i * split) : i + ((i + 1) * split) + 1])

    return splits


def display_round_stats(round_, rounds, start, round_start, now, last_round_increments, new_increments, results):
    round_start
    round_seconds_elapsed = round(now - round_start, 1)
    round_hours_elapsed = round(round_seconds_elapsed / 3600, 2)

    seconds_elapsed = round(now - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"Round {round_}\t\t{round_}/{rounds}\t{round(round_ / rounds * 100, 1)}%")
    # print("\tnew baords searched\tunique boards found this round\tunique boards found so far")
    # print(f"\t{len(results)}\t{len(new_increments)}\t{len(results)}")
    print(f"\t{len(last_round_increments)} new boards searched")
    print(f"\t{len(new_increments)} unique boards found this round")
    print(f"\t{len(results)} unique boards found so far")
    print(f"\t{round_hours_elapsed} hours or {round_seconds_elapsed} seconds elapsed this round")
    # print(f"\t{round(seconds_elapsed / len(new_increments), 2)}s / iteration")
    print(f"\t{hours_elapsed} hours or {seconds_elapsed} seconds elapsed so far")
    print(
        f"\t{round((round_seconds_elapsed / (seconds_elapsed + 1e-6)) * 100, 1)}% of total time spent on this round, "
        f"{round(seconds_elapsed / (len(new_increments) + 1), 2)} seconds/iteration"
    )
    print()
