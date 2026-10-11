// Shared utilities for transfer-matrix board counting.
//
// Instead of listing every board (see cpp_brute_force/), these scripts count
// boards by sweeping across the grid one block at a time while keeping a tally
// of how many partial boards share each "edge" (the placed blocks that future
// blocks can still touch). Boards are never stored individually, so the cost
// depends on the number of distinct edge shapes, not the number of boards.
// See cpp_transfer_matrix/README.md for a full walkthrough.
//
// One counting engine handles all four variants:
//   - permutations: count height assignments on the plain grid graph.
//   - combinations: Burnside's lemma, which also needs the number of boards
//     unchanged by 180° and 90° rotation. Those boards are counted with the
//     same engine on a "folded" graph where each rotation orbit of cells
//     becomes a single vertex.
//   - no border: frontier heights are normalized (minimum subtracted), so
//     boards differing by a constant height shift share a tally slot.
//   - with border: absolute heights are kept and edge cells are bounded. Shift
//     classes are recovered as count(edges in [-2,2]) - count(edges in
//     [-2,1]).

#ifndef BOARD_GAME_TRANSFER_MATRIX_UTILS_H
#define BOARD_GAME_TRANSFER_MATRIX_UTILS_H

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Maximum height difference between orthogonally adjacent blocks.
const int MAX_STEP = 2;

// Edge cells of a bordered board must stay within this range of the border.
const int BORDER_LOW = -2;
const int BORDER_HIGH = 2;

// Maximum number of frontier heights a tally slot can hold (one byte each).
const int MAX_FRONTIER = 16;

// --- Big integer ---
// Unsigned 256-bit integer. 10x10 counts reach ~10^56, far beyond 64 bits.
// Only the operations the counter needs are implemented. Addition throws on
// overflow, which bounds boards to roughly 105 cells.

class BigCount
{
  uint64_t limbs_[4] = { 0, 0, 0, 0 };

public:
  BigCount(uint64_t value = 0);
  void add(const BigCount& other);
  BigCount subtract(const BigCount& other) const;
  // Divides in place by a small divisor and returns the remainder.
  uint32_t divide(uint32_t divisor);
  bool is_zero() const;
  // Decimal string, optionally with thousands separators ("1,459").
  std::string to_string(bool commas = false) const;
};

// --- Graph description ---
// The counter works on any graph whose vertices are numbered in placement
// order. For a plain board, vertex r*columns+c is the block at (r, c) and
// vertices are placed in reading order. For a folded (rotation-quotient)
// board, each vertex stands for every block in one rotation orbit.

struct HeightGraph
{
  int num_vertices = 0;
  std::vector<std::vector<int>> neighbors;
  // True if the vertex's blocks touch the outer border of the board.
  std::vector<bool> on_border;
};

HeightGraph
build_grid_graph(const int& rows, const int& columns);

// Folds the board so each orbit of the rotation becomes one vertex.
// quarter_turns = 2 folds by 180°; quarter_turns = 1 folds by 90° (square
// boards only). Orbit representatives are numbered in reading order of their
// earliest block, which keeps the frontier about one row wide.
HeightGraph
build_rotation_quotient_graph(const int& rows,
                              const int& columns,
                              const int& quarter_turns);

// --- Counting engine ---

struct CountStats
{
  size_t peak_slots = 0;
  int max_frontier = 0;
};

// Counts height assignments where neighbors differ by at most MAX_STEP.
//   with_border = false: counts shift classes (the first vertex is pinned to 0
//     and frontier heights are normalized).
//   with_border = true: counts absolute assignments with border vertices in
//     [border_low, border_high].
BigCount
count_height_assignments(const HeightGraph& graph,
                         const bool& with_border,
                         const int& border_low,
                         const int& border_high,
                         CountStats& stats);

// Shift classes of valid boards on the graph, with or without the border.
BigCount
count_shift_classes(const HeightGraph& graph,
                    const bool& with_border,
                    CountStats& stats);

// Unique boards counting rotations as distinct.
BigCount
count_permutations(int rows,
                   int columns,
                   const bool& with_border,
                   CountStats& stats);

// Unique boards up to rotation, via Burnside's lemma:
//   square:    (all + fixed_180 + 2 * fixed_90) / 4
//   rectangle: (all + fixed_180) / 2
BigCount
count_combinations(int rows,
                   int columns,
                   const bool& with_border,
                   CountStats& stats);

// --- Display helpers ---

long long
get_current_time_ms();

void
print_count(const std::string& label,
            const int& rows,
            const int& columns,
            const BigCount& count,
            const CountStats& stats,
            const long long& elapsed_ms);

// Prints a Markdown table (rows 1..max_rows by columns 1..max_columns) in the
// same layout as the tables in the top-level README.
void
print_table(const std::string& title,
            const int& max_rows,
            const int& max_columns,
            const bool& with_border,
            const bool& combinations);

#endif
