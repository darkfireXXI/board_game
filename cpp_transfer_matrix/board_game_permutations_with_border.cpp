// Counts unique board permutations for a bordered board with the
// transfer-matrix method.
//
// Algorithm overview:
//   Same block-by-block tally sweep as the borderless variant, but heights are
//   absolute because the border sits at height 0: edge blocks must stay in
//   [-2, 2]. Boards that differ by a constant shift are still identified, so
//   the count is
//     count(edge blocks in [-2, 2]) - count(edge blocks in [-2, 1]),
//   which counts each valid shift class exactly once.
//
// Permutations treat rotations as distinct.
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
      "Permutations Array With Border", rows, columns, with_border, false);
    return 0;
  }

  long long start = get_current_time_ms();
  CountStats stats;
  BigCount count = count_permutations(rows, columns, with_border, stats);
  print_count("Permutations, with border",
              rows,
              columns,
              count,
              stats,
              get_current_time_ms() - start);
}
