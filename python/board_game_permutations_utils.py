import time
from pathlib import Path

import numpy as np


def generate_initial_board(size):
    initial_board = np.zeros((size, size))
    return initial_board


def hash_board(board):
    size = len(board)
    board_str = ""
    for r in range(size):
        for c in range(size):
            # board_str += f"{r},{c},{board[r, c]}|"
            board_str += f"{board[r, c]}|"

    return board_str


def board_hash_to_array(board_hash_str, size):
    board = np.zeros((size, size))
    squares = board_hash_str[:-1].split("|")
    for i, square in enumerate(squares):
        # r, c, v = square.split(",")
        r = i / size
        c = i % size
        board[int(r), int(c)] = int(float(square))

    return board


def split_list(list_, n):
    split = max(int(len(list_) / n), 1)
    splits = []
    for i in range(n):
        splits.append(list_[i + (i * split) : i + ((i + 1) * split) + 1])

    return splits


def write_to_file(items, folder_name):
    filename = f"{folder_name}_{int(time.time() * 1e3)}.txt"
    with open(Path.cwd() / folder_name / filename, "w") as file:
        if folder_name == "results":
            file.write("\n".join(items))
        elif folder_name == "new_increments":
            file.write("\n".join(hash_board(item) for item in items))
        else:
            pass

    return filename


def check_results_vs_files(results_list, result_files):
    is_new_check = [True] * len(results_list)
    for filename in result_files:
        with open(Path.cwd() / "results" / filename, "r") as file:
            temp_results = file.read().splitlines()

        temp_results = set(temp_results)

        for i, (board_increment, rotated_hashes) in enumerate(results_list):
            if is_new_check[i] and any(h in temp_results for h in rotated_hashes):
                is_new_check[i] = False

    return is_new_check


def get_file_item_count(files, folder_name):
    if folder_name == "new_increments":
        count = 0
        for filename in files:
            with open(Path.cwd() / folder_name / filename, "r") as file:
                temp = file.read().splitlines()
            count += len(temp)

    elif folder_name == "results":
        count_per_file = 0
        if files:
            with open(Path.cwd() / folder_name / files[0], "r") as file:
                temp = file.read().splitlines()
            count_per_file += len(temp)
        count = len(files) * count_per_file

    return count


def display_round_stats(round_, rounds, start, round_start, new_board_count, result_count):
    now = time.time()

    round_seconds_elapsed = round(now - round_start, 1)
    round_hours_elapsed = round(round_seconds_elapsed / 3600, 2)

    seconds_elapsed = round(now - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"Round {round_}\t\t{round_}/{rounds}\t{round(round_ / rounds * 100, 1)}%")
    print(f"\t{new_board_count} unique boards found this round")
    print(f"\t{result_count} unique boards found so far")
    print(f"\t{round_hours_elapsed} hours or {round_seconds_elapsed} seconds elapsed this round")
    print(f"\t{hours_elapsed} hours or {seconds_elapsed} seconds elapsed so far")
    print(f"\t{round((round_seconds_elapsed / (seconds_elapsed + 1e-6)) * 100, 1)}% of total time spent on this round")
    print()
