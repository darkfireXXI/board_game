import argparse
import multiprocessing as mp
import time
from pathlib import Path

import board_game_counting_utils as bg_utils
import board_game_no_border_utils as nb_utils
import numpy as np
from tqdm import tqdm

if __name__ in "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter,
        description="Calculate Board Game Combinations No Border"
        "\nUnique boards, discounting rotations and height shifts.",
    )
    parser.add_argument(
        "-r",
        "--rows",
        metavar="\b",
        type=int,
        default=2,
        choices=list(range(1, 11)),
        help="board rows [default = 2]",
        required=False,
    )
    parser.add_argument(
        "-c",
        "--columns",
        metavar="\b",
        type=int,
        default=2,
        choices=list(range(1, 11)),
        help="board columns [default = 2]",
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

    rows = args.rows
    columns = args.columns
    n_jobs = args.n_jobs

    Path("new_increments").mkdir(exist_ok=True)
    Path("results").mkdir(exist_ok=True)

    initial_board = bg_utils.generate_initial_board(rows, columns)
    most_extreme_board = nb_utils.find_most_extreme_board(initial_board.copy())
    board_min, board_max = np.min(most_extreme_board), np.max(most_extreme_board)
    rounds = nb_utils.calculate_rounds(initial_board.copy(), board_min, board_max, print_boards=False)
    print(f"{rounds} rounds for a {rows}x{columns} board\n")
    print(f"\n{most_extreme_board}\n")

    zeroed_board = nb_utils.zero_board(initial_board.copy())
    results = {bg_utils.hash_board(zeroed_board)}
    result_files = []
    last_round_increments = [initial_board]
    new_increments = []
    increment_files = []
    new_increment_files = []

    if n_jobs > 1:
        filename = f"new_increments_{(time.time())}.txt"
        with open(Path.cwd() / "new_increments" / filename, "w") as file:
            file.write("\n".join(bg_utils.hash_board(increment) for increment in last_round_increments))

        increment_files.append(filename)

    CHUNK_SIZE = 25_000
    MAX_IN_MEM = 20_000_000

    start = time.time()

    for round_ in range(1, rounds + 1):
        round_start = time.time()

        if n_jobs > 1:
            for filename in tqdm(increment_files, leave=False):
                with open(Path.cwd() / "new_increments" / filename, "r") as file:
                    increments = file.read().splitlines()

                increments = [bg_utils.board_hash_to_array(increment, rows, columns) for increment in increments]

                for i in tqdm(range(0, len(increments), CHUNK_SIZE * n_jobs), leave=False):
                    split_increments = bg_utils.split_list(
                        increments[i : min(i + CHUNK_SIZE * n_jobs, len(increments))], n=n_jobs
                    )
                    with mp.Pool(processes=n_jobs) as pool:
                        results_lists = pool.starmap(
                            nb_utils.generate_boards_mp_combo, [(si, board_min, board_max) for si in split_increments]
                        )

                    is_new_checks = bg_utils.check_results_vs_files(result_files, results_lists, n_jobs)

                    for nj in range(n_jobs):
                        for j in range(len(results_lists[nj])):
                            if is_new_checks[nj][j]:
                                board_increment, min_board_hash = results_lists[nj][j]
                                if min_board_hash not in results:
                                    results.add(min_board_hash)
                                    new_increments.append(board_increment)

                    # dump excess results to txt file
                    while len(results) >= MAX_IN_MEM:
                        results = list(results)
                        filename = bg_utils.write_to_file(results[:MAX_IN_MEM], "results")
                        results = set(results[MAX_IN_MEM:])
                        result_files.append(filename)
                        time.sleep(0.01)

                    # dump excess new increments to txt file
                    if len(new_increments) >= MAX_IN_MEM:
                        filename = bg_utils.write_to_file(new_increments, "new_increments")
                        new_increments = []
                        new_increment_files.append(filename)

            if len(new_increments):
                filename = bg_utils.write_to_file(new_increments, "new_increments")
                new_increments = []
                new_increment_files.append(filename)

            increment_files = new_increment_files.copy()
            new_increment_files = []

        else:
            for last_round_new_increment in tqdm(last_round_increments, leave=False):
                board_increments = nb_utils.generate_all_board_increments_combo(
                    last_round_new_increment, board_min, board_max
                )
                for board_increment in board_increments.values():
                    is_new_combo, min_board_hash = nb_utils.check_board_is_new_combo(board_increment, results)
                    if is_new_combo:
                        results.add(min_board_hash)
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
