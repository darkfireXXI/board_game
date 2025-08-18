import multiprocessing as mp
import time

import numpy as np
from tqdm import tqdm


def generate_initial_board(size):
    initial_board = np.zeros((size, size))
    return initial_board


# def drop_board(board):
#     size = len(board)
#     rings = size // 2
#     if size % 2 != 0:
#         rings += 1

#     for i in range(rings):
#         # first row
#         board[i, (0 + i) : (size - i)] = -2 * (i + 1)
#         # last row
#         board[size - (i + 1), (0 + i) : (size - i)] = -2 * (i + 1)
#         # first column
#         board[(0 + i) : (size - i), i] = -2 * (i + 1)
#         # last column
#         board[(0 + i) : (size - i), size - (i + 1)] = -2 * (i + 1)

#     return board


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

def check_board_is_new_combo(board, board_sum, results, queue=None):
    size = len(board)

    sum_checks = []
    array_height_checks = []
    for i in [-2, -1, 1, 2]:
        sum_checks.append(size**2 * i)
        array_height_checks.append(np.zeros((size, size)) + i)

    sum_checks.insert(2, 0)
    array_height_checks.insert(2, np.zeros((size, size)))

    rot_boards = [board]
    for _ in range(3):
        rot_boards.append(np.rot90(rot_boards[-1]))

    for result in results:
        if board_sum in [result[1] + sum_check for sum_check in sum_checks]:
            for array_height_check in array_height_checks:
                for rot_board in rot_boards:
                    if np.all(rot_board == result[0] + array_height_check):
                        if queue is None:
                            return False
                        else:
                            queue.put(False)


    if queue is None:
        return True
    else:
        queue.put(True)


def split_list(list_, n):
    # n = 19
    # m = 3
    # x = list(range(n))
    split = max(int(len(list_) / n), 1)
    splits = []
    for i in range(n):
        # print(i + (i * split) , i + ((i + 1) * split) + 1)
        splits.append(list_[i + (i * split) : i + ((i + 1) * split) + 1])

    # print(split)
    # print(x)
    # print(split_results)
    return splits


# split_results_func()
# exit()


def check_board_is_new_combo_mp(board, board_sum, results, n_jobs=2):
    size = len(board)

    sum_checks = []
    array_height_checks = []
    for i in [-2, -1, 1, 2]:
        sum_checks.append(size**2 * i)
        array_height_checks.append(np.zeros((size, size)) + i)

    sum_checks.insert(2, 0)
    array_height_checks.insert(2, np.zeros((size, size)))

    rot_boards = [board]
    for _ in range(3):
        rot_boards.append(np.rot90(rot_boards[-1]))

    # try searching results reversed

    queue = mp.Queue()
    split_results = split_list(results, n_jobs)
    jobs = []
    for i in range(n_jobs):
        task = mp.Process(target=check_board_is_new_combo, args=(board, board_sum, split_results[i], queue))
        jobs.append(task)


    for task in jobs:
        task.start()

    for task in jobs:
        task.join()

    for _ in range(n_jobs):
        is_new_combo_task = queue.get()
        if not is_new_combo_task:
            return False

    return True

    # return all([queue.get() for _ in range(n_jobs)])



def generate_all_board_increments(board, board_min, board_max):
    print('here')
    print(board)
    board_increments = []
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
                board_sum = np.sum(board)
                is_new_combo = check_board_is_new_combo(board, board_sum, board_increments)
                if is_new_combo:
                    board_increments.append((board.copy(), board_sum))

            board[r, c] -= 1

    return board_increments


def display_round_stats(round_, rounds, start, round_start, now, results, new_results, all_results):
    round_start
    round_seconds_elapsed = round(now - round_start, 1)
    round_hours_elapsed = round(round_seconds_elapsed / 3600, 2)

    seconds_elapsed = round(now - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"Round {round_}\t\t{round_}/{rounds}\t{round(round_ / rounds * 100, 1)}%")
    # print("\tnew baords searched\tunique boards found this round\tunique boards found so far")
    # print(f"\t{len(results)}\t{len(new_results)}\t{len(all_results)}")
    print(f"\t{len(results)} new boards searched")
    print(f"\t{len(new_results)} unique boards found this round")
    print(f"\t{len(all_results)} unique boards found so far")
    print(f"\t{round_hours_elapsed} hours or {round_seconds_elapsed} seconds elapsed this round")
    # print(f"\t{round(seconds_elapsed / len(new_results), 2)}s / iteration")
    print(f"\t{hours_elapsed} hours or {seconds_elapsed} seconds elapsed so far")
    print(
        f"\t{round((round_seconds_elapsed / (seconds_elapsed + 1e-6)) * 100, 1)}% of total time spent on this round, "
        f"{round((seconds_elapsed) / (len(new_results) + 1), 2)} seconds/iteration"
        )
    print()


if __name__ in "__main__":

    size = 2
    initial_board = generate_initial_board(size=size)
    print(initial_board)
    # x = initial_board.copy()
    # print(np.all(initial_board == x))
    # print([(i, i // 2) for i in range(8)])
    # print(drop_board(initial_board))
    most_extreme_board = find_most_extreme_board(initial_board.copy())
    # print(board)
    board_min, board_max = most_extreme_board[0, 0], most_extreme_board[size - 1, size - 1]


    # increment_board(initial_board, board_min, board_max)

    # rounds = calculate_rounds(initial_board, board_min, board_max, print_boards=True)
    # print(rounds)
    # exit()

    board = initial_board.copy()
    rounds = calculate_rounds(board, board_min, board_max, print_boards=False)
    print(f"{rounds} rounds for a {size}x{size} board")

    board = initial_board.copy()
    print(board)

    # print(np.sum(initial_board))
    all_results = [(initial_board, np.sum(initial_board))]
    results = [(initial_board, np.sum(initial_board))]
    new_results = []

    start = time.time()

    for round_ in range(1, rounds + 1):
        round_start = time.time()
        for result in tqdm(results, leave=False):
            board_increments = generate_all_board_increments(result[0], board_min, board_max)
            # for x in board_increments:
            #     print(x)
            # exit()
            for board_increment in board_increments:
                new_board = board_increment[0]
                new_sum = np.sum(new_board)

                # might need to only cross check results of current round
                is_new_combo = check_board_is_new_combo(new_board, new_sum, all_results)
                # is_new_combo = check_board_is_new_combo_mp(new_board, new_sum, all_results, n_jobs=4)
                if is_new_combo:
                    print(new_board)
                    all_results.append((new_board, new_sum))
                    new_results.append((new_board, new_sum))

        now = time.time()
        display_round_stats(round_, rounds, start, round_start, now, results, new_results, all_results)

        results = new_results.copy()
        new_results = []

    end = time.time()

    seconds_elapsed = round(end - start, 1)
    hours_elapsed = round(seconds_elapsed / 3600, 2)

    print(f"{hours_elapsed} hours or {seconds_elapsed} seconds")

    print("final", len(all_results))

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

# start = time.time()

# for _ in range(n):
# 	# size = len(board)
# 	# sum_checks = [size** 2 * i for i in range(-2, 3)]
# 	# array_height_checks = [np.zeros((size, size)) + i for i in range(-2, 3)]


# 	sum_checks = []
# 	array_height_checks = []
# 	for i in [-2, -1, 1, 2]:
# 		sum_checks.append(size** 2 * i)
# 		array_height_checks.append(np.zeros((size, size)) + i)

# 	sum_checks.insert(2, 0)
# 	array_height_checks.insert(2, np.zeros((size, size)))


# end = time.time()

# print(sum_checks)

# print(f"avg {(end - start) / n} s per iter")
