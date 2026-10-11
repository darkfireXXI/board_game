// Counts unique board permutations for a borderless board with the
// transfer-matrix method.
//
// Algorithm overview:
//   Blocks are placed one at a time in reading order. A tally maps each
//   frontier shape (heights of placed blocks that unplaced blocks still touch)
//   to the number of partial boards sharing it. Each new block may take any
//   height within 2 of its placed neighbors; every choice moves the slot's
//   whole count into the slot for the resulting frontier. Summing the final
//   tally gives the board count.
//
// Permutations treat rotations as distinct; only constant height shifts are
// identified. With no border, the first block is pinned to 0 and frontier
// heights are normalized (minimum subtracted) after every placement.
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
      "Permutations Array No Border", rows, columns, with_border, false);
    return 0;
  }

  long long start = get_current_time_ms();
  CountStats stats;
  BigCount count = count_permutations(rows, columns, with_border, stats);
  print_count("Permutations, no border",
              rows,
              columns,
              count,
              stats,
              get_current_time_ms() - start);
}
