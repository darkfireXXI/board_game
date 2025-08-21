import argparse
import itertools
import multiprocessing as mp
import time
from pathlib import Path

import numpy as np
from tqdm import tqdm

import board_game_permutations_utils as bg_utils


def find_most_extreme_board(board):
    size = len(board)
    for r in range(size):
        for c in range(size):
            board[r, c] = (r + c) * 2

    return board


def increment_board(board, board_min, board_max):
    size = len(board)
    for r in range(size):
        for c in range(size):
            board[r, c] += 1

            if (
                # ensure local physics is not broken
                abs(board[max(0, r - 1), c] - board[r, c]) <= 2
                and abs(board[min(size - 1, r + 1), c] - board[r, c]) <= 2
                and abs(board[r, max(0, c - 1)] - board[r, c]) <= 2
                and abs(board[r, min(size - 1, c + 1)] - board[r, c]) <= 2
                # ensure boundary physics is not broken
                and np.all(board >= board_min)
                and np.all(board <= board_max)
            ):
                return board

            board[r, c] -= 1

    return None


def calculate_rounds(board, board_min, board_max, print_boards=False):
    rounds = 0
    while True:
        board = increment_board(board, board_min, board_max)

        if print_boards:
            print(board)

        rounds += 1

        if board is None:
            break

    return rounds


def generate_all_board_increments(board, board_min, board_max):
    board_increments = {}
    size = len(board)
    for r in range(size):
        for c in range(size):
            board[r, c] += 1

            if (
                # ensure local physics is not broken
                abs(board[max(0, r - 1), c] - board[r, c]) <= 2
                and abs(board[min(size - 1, r + 1), c] - board[r, c]) <= 2
                and abs(board[r, max(0, c - 1)] - board[r, c]) <= 2
                and abs(board[r, min(size - 1, c + 1)] - board[r, c]) <= 2
                # ensure boundary physics is not broken
                and np.all(board >= board_min)
                and np.all(board <= board_max)
            ):
                is_new_combo = bg_utils.check_board_is_new_combo(board, board_increments)
                if is_new_combo:
                    board_increments[bg_utils.board_hash(bg_utils.zero_board(board.copy()))] = board.copy()

            board[r, c] -= 1

    return board_increments


def generate_boards_mp(split_increments, board_min, board_max):
    new_boards = []
    for board in split_increments:
        increments = generate_all_board_increments(board.copy(), board_min, board_max)
        for board_increment in increments.values():
            zeroed = bg_utils.zero_board(board_increment.copy())
            rotations = [zeroed]
            hashes = {bg_utils.board_hash(zeroed)}
            for _ in range(3):
                rotated = np.rot90(rotations[-1])
                rotations.append(rotated)
                hashes.add(bg_utils.board_hash(rotated))
            new_boards.append((board_increment.copy(), hashes))

    return new_boards


if __name__ in "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter,
        description="Calculate Board Game Permutations - No Border",
    )
    parser.add_argument(
        "-s",
        "--size",
        metavar="\b",
        type=int,
        default=2,
        choices=list(range(1, 11)),
        help="board size [default = 2]",
        required=False,
    )
    parser.add_argument(
        "-n",
        "--n-jobs",
        metavar="\b",
        type=int,
        default=1,
        help="number of jobs [default = 1]",
        required=False,
    )

    args = parser.parse_args()

    size = args.size
    n_jobs = args.n_jobs

    initial_board = bg_utils.generate_initial_board(size=size)
    most_extreme_board = find_most_extreme_board(initial_board.copy())
    board_min, board_max = most_extreme_board[0, 0], most_extreme_board[size - 1, size - 1]
    rounds = calculate_rounds(initial_board.copy(), board_min, board_max, print_boards=False)
    print(f"{rounds} rounds for a {size}x{size} board\n")

    zeroed_board = bg_utils.zero_board(initial_board.copy())
    results = {bg_utils.board_hash(zeroed_board)}
    result_files = []
    last_round_increments = [initial_board]
    new_increments = []
    increment_files = []
    new_increment_files = []

    if n_jobs > 1:
        filename = f"new_increments_{(time.time())}.txt"
        with open(Path.cwd() / "new_increments" / filename, "w") as file:
            file.write("\n".join(bg_utils.board_hash(increment) for increment in last_round_increments))

        new_increment_files.append(filename)

    CHUNK_SIZE = 5_000
    MAX_IN_MEM = 10_000_000

    start = time.time()

    for round_ in range(1, rounds + 1):
        round_start = time.time()

        if n_jobs > 1:
            for filename in tqdm(increment_files, leave=False):
                with open(Path.cwd() / "new_increments" / filename, "r") as file:
                    increments = file.read().splitlines()

                increments = [bg_utils.board_hash_to_array(increment, size) for increment in increments]

                for i in tqdm(range(0, len(increments), CHUNK_SIZE * n_jobs), leave=False):
                    split_increments = bg_utils.split_list(
                        increments[i : min(i + CHUNK_SIZE * n_jobs, len(increments))], n=n_jobs
                    )
                    with mp.Pool(processes=n_jobs) as pool:
                        results_lists = pool.starmap(
                            generate_boards_mp, [(si, board_min, board_max) for si in split_increments]
                        )

                    results_list = list(itertools.chain.from_iterable(results_lists))

                    is_new_check = bg_utils.check_results_vs_files(results_list, result_files)

                    results_list = [r for check, r in zip(is_new_check, results_list) if check]
                    for board_increment, rotated_hashes in results_list:
                        if not any(h in results for h in rotated_hashes):
                            results.add(next(iter(rotated_hashes)))
                            new_increments.append(board_increment)

                    # dump excess results to txt file
                    if len(results) >= MAX_IN_MEM:
                        filename = f"results_{int(time.time() * 1e3)}.txt"
                        with open(Path.cwd() / "results" / filename, "w") as file:
                            file.write("\n".join(results))

                        results = set()
                        result_files.append(filename)

                    # dump excess new increments to txt file
                    if len(new_increments) >= MAX_IN_MEM:
                        filename = f"new_increments_{int(time.time() * 1e3)}.txt"
                        with open(Path.cwd() / "new_increments" / filename, "w") as file:
                            file.write("\n".join(bg_utils.board_hash(increment) for increment in new_increments))

                        new_increments = []
                        new_increment_files.append(filename)

            filename = f"new_increments_{int(time.time() * 1e3)}.txt"
            with open(Path.cwd() / "new_increments" / filename, "w") as file:
                file.write("\n".join(bg_utils.board_hash(increment) for increment in new_increments))

            new_increments = []
            new_increment_files.append(filename)

            increment_files = new_increment_files.copy()
            new_increment_files = []

        else:
            for last_round_new_increment in tqdm(last_round_increments, leave=False):
                board_increments = generate_all_board_increments(last_round_new_increment, board_min, board_max)
                for board_increment in board_increments.values():
                    is_new_combo = bg_utils.check_board_is_new_combo(board_increment, results)
                    if is_new_combo:
                        zeroed_board = bg_utils.zero_board(board_increment.copy())
                        results.add(bg_utils.board_hash(zeroed_board))
                        new_increments.append(board_increment.copy())

        new_board_count = bg_utils.get_file_item_count(increment_files, "new_increments") + len(new_increments)
        result_count = bg_utils.get_file_item_count(result_files, "results") + len(results)
        bg_utils.display_round_stats(
            round_, rounds, start, round_start, last_round_increments, new_board_count, result_count
        )

        last_round_increments = new_increments.copy()
        new_increments = []

    end = time.time()

    seconds_elapsed = round(end - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"{hours_elapsed} hours or {seconds_elapsed} seconds")

    result_count = bg_utils.get_file_item_count(result_files, "results")
    print(f"{result_count + len(results)} total unique boards found")
