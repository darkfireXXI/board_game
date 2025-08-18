import argparse
import multiprocessing as mp
import time

import numpy as np
from tqdm import tqdm


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
                is_new_combo = check_board_is_new_combo(board, board_increments)
                if is_new_combo:
                    board_increments[board_hash(zero_board(board.copy()))] = board.copy()

            board[r, c] -= 1

    return board_increments


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


def thing(split_results, new_results, all_results, core=-1):
    if core == 0:
        for result in tqdm(split_results, leave=False):
            board_increments = generate_all_board_increments(result)
            for board_increment in board_increments.values():
                is_new_combo = check_board_is_new_combo(board_increment, all_results)
                if is_new_combo:
                    zeroed_board = zero_board(board_increment.copy())
                    all_results[board_hash(zeroed_board)] = zeroed_board
                    new_results.append(board_increment.copy())

    else:
        for result in split_results:
            board_increments = generate_all_board_increments(result)
            for board_increment in board_increments.values():
                is_new_combo = check_board_is_new_combo(board_increment, all_results)
                if is_new_combo:
                    zeroed_board = zero_board(board_increment.copy())
                    all_results[board_hash(zeroed_board)] = zeroed_board
                    new_results.append(board_increment.copy())


def generate_boards_mp(split_increments):
    new_boards = []
    for board in split_increments:
        increments = generate_all_board_increments(board.copy())
        for board_increment in increments.values():
            zeroed = zero_board(board_increment.copy())
            rotations = [zeroed]
            hashes = {board_hash(zeroed)}
            for _ in range(3):
                rotated = np.rot90(rotations[-1])
                rotations.append(rotated)
                hashes.add(board_hash(rotated))
            new_boards.append((board_increment.copy(), hashes))
    return new_boards


if __name__ in "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter,
        description="Calculate Board Game Permutations",
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

    initial_board = generate_initial_board(size=size)
    dropped_board = drop_board(initial_board)
    rounds = calculate_rounds(dropped_board, print_boards=False)
    print(f"{rounds} rounds for a {size}x{size} board\n")

    zeroed_board = zero_board(dropped_board.copy())
    manager = mp.Manager()
    # results = {board_hash(zeroed_board): zeroed_board}
    results = {board_hash(zeroed_board)}
    last_round_increments = [initial_board]
    new_increments = []

    CHUNK_SIZE = 10_000

    start = time.time()

    for round_ in range(1, rounds + 1):
        round_start = time.time()

        if n_jobs > 1 and len(last_round_increments) >= CHUNK_SIZE:
            for i in tqdm(range(0, len(last_round_increments), CHUNK_SIZE * n_jobs), leave=False):
                split_increments = split_list(
                    last_round_increments[i : min(i + 10_000 * n_jobs, len(last_round_increments))], n=n_jobs
                )
                with mp.Pool(processes=n_jobs) as pool:
                    results_lists = pool.map(generate_boards_mp, split_increments)

                for boards in results_lists:
                    for board_increment, rotated_hashes in boards:
                        if not any(h in results for h in rotated_hashes):
                            # results[canonical_hash] = canonical_board
                            results.add(rotated_hashes[0])
                            new_increments.append(board_increment)

        else:
            for last_round_new_increment in tqdm(last_round_increments, leave=False):
                board_increments = generate_all_board_increments(last_round_new_increment)
                for board_increment in board_increments.values():
                    is_new_combo = check_board_is_new_combo(board_increment, results)
                    if is_new_combo:
                        zeroed_board = zero_board(board_increment.copy())
                        # results[board_hash(zeroed_board)] = zeroed_board
                        results.add(board_hash(zeroed_board))
                        new_increments.append(board_increment.copy())

        now = time.time()
        display_round_stats(round_, rounds, start, round_start, now, last_round_increments, new_increments, results)

        last_round_increments = new_increments.copy()
        new_increments = []

    end = time.time()

    seconds_elapsed = round(end - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"{hours_elapsed} hours or {seconds_elapsed} seconds")

    print("final", len(results))

# for x in all_results[:30]:
# 	print(x[0])
# k = 0
# for i, result in enumerate(all_results):
# 	for j, result1 in enumerate(all_results):
# 		if i != j and np.all(result[0] == result1[0]):
# 			print('bad', i, j)
# 			print(result[0])
# 			k += 1

# print(k)
# print(new_results)


# import time

# n = int(1e4)
# ar = np.random.random((5, 5))

# start = time.time()

# for _ in range(n):
# 	# size = len(board)
# 	# sum_checks = [size** 2 * i for i in range(-2, 3)]
# 	# array_height_checks = [np.zeros((size, size)) + i for i in range(-2, 3)]

#     # ar = np.random.random((5, 5))
#     # print(ar)
#     # exit()
#     zero_board(ar)


# 	# sum_checks = []
# 	# array_height_checks = []
# 	# for i in [-2, -1, 1, 2]:
# 	# 	sum_checks.append(size** 2 * i)
# 	# 	array_height_checks.append(np.zeros((size, size)) + i)

# 	# sum_checks.insert(2, 0)
# 	# array_height_checks.insert(2, np.zeros((size, size)))


# end = time.time()

# # print(sum_checks)

# print(f"avg {(end - start) / n} s per iter")


"""
Results

1x1 : 1, 0 seconds
2x2 : 23, 0 seconds
3x3 : ?
4x4 : ?
5x5 : ?
6x6 : ?
7x7 : ?
8x8 : ?
9x9 : ?
10x10 : ?
"""
