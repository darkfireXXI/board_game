// Counts unique board combinations (rotations identified) for a bordered
// board with the transfer-matrix method.
//
// Algorithm overview:
//   Burnside's lemma over the rotations, as in the borderless variant:
//     square:    (all + fixed_180 + 2 * fixed_90) / 4
//     rectangle: (all + fixed_180) / 2
//   Every term uses the bordered shift-class count
//     count(edge blocks in [-2, 2]) - count(edge blocks in [-2, 1]).
//   Rotation maps edge blocks to edge blocks, so folded graphs keep the border
//   rule on exactly the orbits that touch it.
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

  const bool with_border = true;

  if (table) {
    print_table(
      "Combinations Array With Border", rows, columns, with_border, true);
    return 0;
  }

  long long start = get_current_time_ms();
  CountStats stats;
  BigCount count = count_combinations(rows, columns, with_border, stats);
  print_count("Combinations, with border",
              rows,
              columns,
              count,
              stats,
              get_current_time_ms() - start);
}
