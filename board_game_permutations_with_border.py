import argparse
import multiprocessing as mp
import time

import numpy as np
from tqdm import tqdm

import board_game_permutations_utils as bg_utils


def generate_initial_board(size):
    initial_board = np.zeros((size, size))
    return initial_board


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
                is_new_combo = bg_utils.check_board_is_new_combo(board, board_increments)
                if is_new_combo:
                    board_increments[bg_utils.board_hash(bg_utils.zero_board(board.copy()))] = board.copy()

            board[r, c] -= 1

    return board_increments


def generate_boards_mp(split_increments):
    new_boards = []
    for board in split_increments:
        increments = generate_all_board_increments(board.copy())
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
        "-nb",
        "--no-border",
        default=False,
        action="store_true",
        help="whether or not to calculate permutations with a border",
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
    border = args.no_border
    n_jobs = args.n_jobs

    initial_board = bg_utils.generate_initial_board(size=size)
    dropped_board = drop_board(initial_board)
    rounds = calculate_rounds(dropped_board, print_boards=False)
    print(f"{rounds} rounds for a {size}x{size} board\n")

    zeroed_board = bg_utils.zero_board(dropped_board.copy())
    # results = {board_hash(zeroed_board): zeroed_board}
    results = {bg_utils.board_hash(zeroed_board)}
    last_round_increments = [initial_board]
    new_increments = []

    CHUNK_SIZE = 3_000

    start = time.time()

    for round_ in range(1, rounds + 1):
        round_start = time.time()

        if n_jobs > 1 and len(last_round_increments) >= CHUNK_SIZE:
            for i in tqdm(range(0, len(last_round_increments), CHUNK_SIZE * n_jobs), leave=False):
                split_increments = bg_utils.split_list(
                    last_round_increments[i : min(i + CHUNK_SIZE * n_jobs, len(last_round_increments))], n=n_jobs
                )
                with mp.Pool(processes=n_jobs) as pool:
                    results_lists = pool.map(generate_boards_mp, split_increments)

                for boards in results_lists:
                    for board_increment, rotated_hashes in boards:
                        if not any(h in results for h in rotated_hashes):
                            # results[canonical_hash] = canonical_board
                            results.add(next(iter(rotated_hashes)))
                            new_increments.append(board_increment)

        else:
            for last_round_new_increment in tqdm(last_round_increments, leave=False):
                board_increments = generate_all_board_increments(last_round_new_increment)
                for board_increment in board_increments.values():
                    is_new_combo = bg_utils.check_board_is_new_combo(board_increment, results)
                    if is_new_combo:
                        zeroed_board = bg_utils.zero_board(board_increment.copy())
                        # results[board_hash(zeroed_board)] = zeroed_board
                        results.add(bg_utils.board_hash(zeroed_board))
                        new_increments.append(board_increment.copy())

        now = time.time()
        bg_utils.display_round_stats(
            round_, rounds, start, round_start, now, last_round_increments, new_increments, results
        )

        last_round_increments = new_increments.copy()
        new_increments = []

    end = time.time()

    seconds_elapsed = round(end - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"{hours_elapsed} hours or {seconds_elapsed} seconds")

    print(f"{len(results)} total unique boards found")
