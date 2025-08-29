import argparse
import itertools
import multiprocessing as mp
import time
from pathlib import Path

import board_game_permutations_utils as bg_utils
import numpy as np
from tqdm import tqdm


def zero_board(board):
    size = len(board)
    edges = np.concatenate([board[0, :], board[size - 1, :], board[:, 0], board[:, size - 1]])
    board -= min(edges)
    return board


def drop_board(board):
    size = len(board)
    rings = size // 2
    if size % 2 != 0:
        rings += 1

    for i in range(rings):
        # first row
        board[i, (0 + i) : (size - i)] = -2 * (i + 1)
        # last row
        board[size - (i + 1), (0 + i) : (size - i)] = -2 * (i + 1)
        # first column
        board[(0 + i) : (size - i), i] = -2 * (i + 1)
        # last column
        board[(0 + i) : (size - i), size - (i + 1)] = -2 * (i + 1)

    return board


def increment_board(board):
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
                and np.all(board[0, :] <= 2)
                and np.all(board[size - 1, :] <= 2)
                and np.all(board[:, 0] <= 2)
                and np.all(board[:, size - 1] <= 2)
            ):
                return board

            board[r, c] -= 1

    return None


def calculate_rounds(dropped_board, print_boards=False):
    board = dropped_board.copy()
    rounds = 0
    while True:
        board = increment_board(board)

        if print_boards:
            print(board)

        rounds += 1

        if board is None:
            break

    return rounds


def generate_all_board_increments(board):
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
                and np.all(board[0, :] <= 2)
                and np.all(board[size - 1, :] <= 2)
                and np.all(board[:, 0] <= 2)
                and np.all(board[:, size - 1] <= 2)
            ):
                is_new_combo = check_board_is_new_combo(board, set(board_increments.keys()))
                if is_new_combo:
                    board_increments[bg_utils.hash_board(zero_board(board.copy()))] = board.copy()

            board[r, c] -= 1

    return board_increments


def check_board_is_new_combo(board, results):
    zeroed_board = zero_board(board.copy())
    rot_boards = [zeroed_board] * 4
    rot_board_hashes = [bg_utils.hash_board(zeroed_board)] * 4
    for i in range(1, 4):
        rot_board = np.rot90(rot_boards[i - 1])
        rot_boards[i] = rot_board
        rot_board_hashes[i] = bg_utils.hash_board(rot_board)

    for rot_board_hash in rot_board_hashes:
        if rot_board_hash in results:
            return False

    return True


def generate_boards_mp(split_increments):
    new_boards = []
    for board in split_increments:
        increments = generate_all_board_increments(board.copy())
        for board_increment in increments.values():
            zeroed = zero_board(board_increment.copy())
            rotations = [zeroed]
            hashes = {bg_utils.hash_board(zeroed)}
            for _ in range(3):
                rotated = np.rot90(rotations[-1])
                rotations.append(rotated)
                hashes.add(bg_utils.hash_board(rotated))
            new_boards.append((board_increment.copy(), hashes))
    return new_boards


if __name__ in "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter,
        description="Calculate Board Game Permutations - With Boarder",
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

    Path("new_increments").mkdir(exist_ok=True)
    Path("results").mkdir(exist_ok=True)

    initial_board = bg_utils.generate_initial_board(size=size)
    dropped_board = drop_board(initial_board)
    rounds = calculate_rounds(dropped_board, print_boards=False)
    print(f"{rounds} rounds for a {size}x{size} board\n")

    zeroed_board = zero_board(dropped_board.copy())
    results = {bg_utils.hash_board(zeroed_board)}
    result_files = []
    last_round_increments = [dropped_board]
    new_increments = []
    increment_files = []
    new_increment_files = []

    if n_jobs > 1:
        filename = bg_utils.write_to_file(last_round_increments, "new_increments")
        increment_files.append(filename)

    CHUNK_SIZE = 500
    MAX_IN_MEM = 1_000

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
                        results_lists = pool.map(generate_boards_mp, split_increments)

                    results_list = list(itertools.chain.from_iterable(results_lists))

                    is_new_check = bg_utils.check_results_vs_files(results_list, result_files)

                    results_list = [r for check, r in zip(is_new_check, results_list) if check]
                    for board_increment, rotated_hashes in results_list:
                        if not any(h in results for h in rotated_hashes):
                            results.add(next(iter(rotated_hashes)))
                            new_increments.append(board_increment)

                    # dump excess results to txt file
                    if len(results) >= MAX_IN_MEM:
                        filename = bg_utils.write_to_file(results, "results")
                        results = set()
                        result_files.append(filename)

                    # dump excess new increments to txt file
                    if len(new_increments) >= MAX_IN_MEM:
                        filename = bg_utils.write_to_file(new_increments, "new_increments")
                        new_increments = []
                        new_increment_files.append(filename)

            filename = bg_utils.write_to_file(new_increments, "new_increments")
            new_increments = []
            new_increment_files.append(filename)

            increment_files = new_increment_files.copy()
            new_increment_files = []

        else:
            for last_round_new_increment in tqdm(last_round_increments, leave=False):
                board_increments = generate_all_board_increments(last_round_new_increment)
                for board_increment in board_increments.values():
                    is_new_combo = check_board_is_new_combo(board_increment, results)
                    if is_new_combo:
                        zeroed_board = zero_board(board_increment.copy())
                        results.add(bg_utils.hash_board(zeroed_board))
                        new_increments.append(board_increment.copy())

        new_board_count = bg_utils.get_file_item_count(increment_files, "new_increments") + len(new_increments)
        result_count = bg_utils.get_file_item_count(result_files, "results") + len(results)
        bg_utils.display_round_stats(round_, rounds, start, round_start, new_board_count, result_count)

        last_round_increments = new_increments.copy()
        new_increments = []

    end = time.time()

    seconds_elapsed = round(end - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"{hours_elapsed} hours or {seconds_elapsed} seconds")

    result_count = bg_utils.get_file_item_count(result_files, "results")
    print(f"{result_count + len(results)} total unique boards found")
