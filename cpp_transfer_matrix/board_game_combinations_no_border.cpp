// Counts unique board combinations (rotations identified) for a borderless
// board with the transfer-matrix method.
//
// Algorithm overview:
//   Burnside's lemma: the number of boards up to rotation is the average, over
//   the rotations, of how many boards each rotation leaves unchanged.
//     square:    (all + fixed_180 + 2 * fixed_90) / 4
//     rectangle: (all + fixed_180) / 2   (90° changes the board's shape)
//   "all" is the permutation count. Rotation-symmetric boards are counted with
//   the same tally sweep on a folded graph where each orbit of blocks under
//   the rotation becomes a single vertex.
//
// Usage:
//   -r, --rows R      board rows (default 2)
//   -c, --columns C   board columns (default 2)
//   -t, --table       print a Markdown table for every size up to R x C

#include "board_game_transfer_matrix_utils.h"

int
main(int argc, char* argv[])
{
  int rows = 2, columns = 2;
  bool table = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-r" || arg == "--rows") && i + 1 < argc) {
      rows = std::stoi(argv[++i]);
    } else if ((arg == "-c" || arg == "--columns") && i + 1 < argc) {
      columns = std::stoi(argv[++i]);
    } else if (arg == "-t" || arg == "--table") {
      table = true;
    }
  }

  const bool with_border = false;

  if (table) {
    print_table(
      "Combinations Array No Border", rows, columns, with_border, true);
    return 0;
  }

  long long start = get_current_time_ms();
  CountStats stats;
  BigCount count = count_combinations(rows, columns, with_border, stats);
  print_count("Combinations, no border",
              rows,
              columns,
              count,
              stats,
              get_current_time_ms() - start);
}
