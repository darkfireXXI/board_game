import board_game_counting_utils as bg_utils
import numpy as np


def zero_board(board):
    board -= np.min(board)
    return board


def find_most_extreme_board(board):
    rows, columns = board.shape
    for r in range(rows):
        for c in range(columns):
            board[r, c] = (r + c) * 2

    return board


def increment_board(board, board_min, board_max):
    rows, columns = board.shape
    for r in range(rows):
        for c in range(columns):
            board[r, c] += 1

            if (
                # ensure local physics is not broken
                abs(board[max(0, r - 1), c] - board[r, c]) <= 2
                and abs(board[min(rows - 1, r + 1), c] - board[r, c]) <= 2
                and abs(board[r, max(0, c - 1)] - board[r, c]) <= 2
                and abs(board[r, min(columns - 1, c + 1)] - board[r, c]) <= 2
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


def check_board_is_new_combo(board, results):
    rows, columns = board.shape
    zeroed_board = zero_board(board.copy())
    rotations = [zeroed_board]
    hashes = [bg_utils.hash_board(zeroed_board)]
    for i in range(1, 4):
        if rows == columns:
            rotated = np.rot90(rotations[-1])
            rotations.append(rotated)
            hashes.append(bg_utils.hash_board(rotated))
        elif i % 2 == 0:
            rotated = np.rot90(np.rot90(rotations[-1]))
            rotations.append(rotated)
            hashes.append(bg_utils.hash_board(rotated))

    min_board_hash = min(hashes)
    if min_board_hash in results:
        return False, None

    return True, min_board_hash


def check_board_is_new_perm(board, results):
    board_hash = bg_utils.hash_board(zero_board(board.copy()))
    if board_hash in results:
        return False, None

    return True, board_hash


def generate_all_board_increments_combo(board, board_min, board_max):
    board_increments = {}
    rows, columns = board.shape
    for r in range(rows):
        for c in range(columns):
            board[r, c] += 1

            if (
                # ensure local physics is not broken
                abs(board[max(0, r - 1), c] - board[r, c]) <= 2
                and abs(board[min(rows - 1, r + 1), c] - board[r, c]) <= 2
                and abs(board[r, max(0, c - 1)] - board[r, c]) <= 2
                and abs(board[r, min(columns - 1, c + 1)] - board[r, c]) <= 2
                # ensure boundary physics is not broken
                and np.all(board >= board_min)
                and np.all(board <= board_max)
            ):
                is_new_combo, min_board_hash = check_board_is_new_combo(board, set(board_increments.keys()))
                if is_new_combo:
                    board_increments[min_board_hash] = board.copy()

            board[r, c] -= 1

    return board_increments


def generate_all_board_increments_perm(board, board_min, board_max):
    board_increments = {}
    rows, columns = board.shape
    for r in range(rows):
        for c in range(columns):
            board[r, c] += 1

            if (
                # ensure local physics is not broken
                abs(board[max(0, r - 1), c] - board[r, c]) <= 2
                and abs(board[min(rows - 1, r + 1), c] - board[r, c]) <= 2
                and abs(board[r, max(0, c - 1)] - board[r, c]) <= 2
                and abs(board[r, min(columns - 1, c + 1)] - board[r, c]) <= 2
                # ensure boundary physics is not broken
                and np.all(board >= board_min)
                and np.all(board <= board_max)
            ):
                is_new_combo, board_hash = check_board_is_new_perm(board, set(board_increments.keys()))
                if is_new_combo:
                    board_increments[board_hash] = board.copy()

            board[r, c] -= 1

    return board_increments


def generate_boards_mp_combo(split_increments, board_min, board_max):
    rows, columns = split_increments[0].shape if len(split_increments) else (0, 0)
    new_boards = []
    for board in split_increments:
        increments = generate_all_board_increments_combo(board.copy(), board_min, board_max)
        for board_increment in increments.values():
            zeroed = zero_board(board_increment.copy())
            rotations = [zeroed]
            hashes = {bg_utils.hash_board(zeroed)}
            for i in range(1, 4):
                if rows == columns:
                    rotated = np.rot90(rotations[-1])
                    rotations.append(rotated)
                    hashes.add(bg_utils.hash_board(rotated))
                elif i % 2 == 0:
                    rotated = np.rot90(np.rot90(rotations[-1]))
                    rotations.append(rotated)
                    hashes.add(bg_utils.hash_board(rotated))

            new_boards.append((board_increment.copy(), min(hashes)))

    return new_boards


def generate_boards_mp_perm(split_increments, board_min, board_max):
    rows, columns = split_increments[0].shape if len(split_increments) else (0, 0)
    new_boards = []
    for board in split_increments:
        increments = generate_all_board_increments_perm(board.copy(), board_min, board_max)
        for board_increment in increments.values():
            board_hash = bg_utils.hash_board(zero_board(board_increment.copy()))
            new_boards.append((board_increment.copy(), board_hash))

    return new_boards
