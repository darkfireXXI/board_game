// Transfer-matrix counting engine: big integer, graph builders, the frontier
// tally sweep, Burnside's lemma for combinations, and display helpers.

#include "board_game_transfer_matrix_utils.h"

#include <algorithm>
#include <climits>
#include <iomanip>
#include <set>
#include <stdexcept>

// --- Big integer implementation ---

BigCount::BigCount(uint64_t value)
{
  limbs_[0] = value;
}

void
BigCount::add(const BigCount& other)
{
  unsigned __int128 carry = 0;
  for (int i = 0; i < 4; ++i) {
    carry += static_cast<unsigned __int128>(limbs_[i]) + other.limbs_[i];
    limbs_[i] = static_cast<uint64_t>(carry);
    carry >>= 64;
  }
  if (carry) {
    throw std::overflow_error("count exceeds 256 bits; board is too large");
  }
}

BigCount
BigCount::subtract(const BigCount& other) const
{
  BigCount result;
  __int128 borrow = 0;
  for (int i = 0; i < 4; ++i) {
    __int128 diff = static_cast<__int128>(limbs_[i]) - other.limbs_[i] - borrow;
    borrow = diff < 0 ? 1 : 0;
    result.limbs_[i] = static_cast<uint64_t>(diff);
  }
  if (borrow) {
    throw std::underflow_error("BigCount subtraction went negative");
  }
  return result;
}

uint32_t
BigCount::divide(uint32_t divisor)
{
  unsigned __int128 remainder = 0;
  for (int i = 3; i >= 0; --i) {
    unsigned __int128 current = (remainder << 64) | limbs_[i];
    limbs_[i] = static_cast<uint64_t>(current / divisor);
    remainder = current % divisor;
  }
  return static_cast<uint32_t>(remainder);
}

bool
BigCount::is_zero() const
{
  return limbs_[0] == 0 && limbs_[1] == 0 && limbs_[2] == 0 && limbs_[3] == 0;
}

std::string
BigCount::to_string(bool commas) const
{
  if (is_zero()) {
    return "0";
  }

  BigCount value = *this;
  std::string digits;
  while (!value.is_zero()) {
    digits.push_back(static_cast<char>('0' + value.divide(10)));
  }

  std::string result;
  for (size_t i = 0; i < digits.size(); ++i) {
    if (commas && i > 0 && i % 3 == 0) {
      result.push_back(',');
    }
    result.push_back(digits[i]);
  }
  std::reverse(result.begin(), result.end());
  return result;
}

// --- Graph builders ---

HeightGraph
build_grid_graph(const int& rows, const int& columns)
{
  HeightGraph graph;
  graph.num_vertices = rows * columns;
  graph.neighbors.resize(graph.num_vertices);
  graph.on_border.resize(graph.num_vertices);

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      int v = r * columns + c;
      graph.on_border[v] =
        r == 0 || r == rows - 1 || c == 0 || c == columns - 1;
      if (r > 0)
        graph.neighbors[v].push_back(v - columns);
      if (r < rows - 1)
        graph.neighbors[v].push_back(v + columns);
      if (c > 0)
        graph.neighbors[v].push_back(v - 1);
      if (c < columns - 1)
        graph.neighbors[v].push_back(v + 1);
    }
  }

  return graph;
}

HeightGraph
build_rotation_quotient_graph(const int& rows,
                              const int& columns,
                              const int& quarter_turns)
{
  if (quarter_turns != 1 && quarter_turns != 2) {
    throw std::invalid_argument("quarter_turns must be 1 (90°) or 2 (180°)");
  }
  if (quarter_turns == 1 && rows != columns) {
    throw std::invalid_argument("90° folding requires a square board");
  }

  int cells = rows * columns;

  // Rotates block index v by the requested amount.
  auto rotate = [&](int v) {
    int r = v / columns;
    int c = v % columns;
    if (quarter_turns == 2) {
      return (rows - 1 - r) * columns + (columns - 1 - c);
    }
    return c * columns + (rows - 1 - r);
  };

  // Each block's orbit representative is the orbit's earliest block in
  // reading order.
  std::vector<int> representative(cells);
  for (int v = 0; v < cells; ++v) {
    int earliest = v;
    for (int u = rotate(v); u != v; u = rotate(u)) {
      earliest = std::min(earliest, u);
    }
    representative[v] = earliest;
  }

  // Number representatives in reading order; that is the placement order.
  std::vector<int> vertex_of(cells, -1);
  HeightGraph graph;
  for (int v = 0; v < cells; ++v) {
    if (representative[v] == v) {
      vertex_of[v] = graph.num_vertices++;
    }
  }

  HeightGraph grid = build_grid_graph(rows, columns);
  std::vector<std::set<int>> neighbor_sets(graph.num_vertices);
  graph.on_border.resize(graph.num_vertices);

  for (int v = 0; v < cells; ++v) {
    int a = vertex_of[representative[v]];
    // Rotation maps border blocks to border blocks, so the whole orbit agrees.
    graph.on_border[a] = grid.on_border[v];
    for (int u : grid.neighbors[v]) {
      int b = vertex_of[representative[u]];
      // Adjacent blocks in the same orbit must have equal heights, which
      // always satisfies the step rule, so self-loops are dropped.
      if (a != b) {
        neighbor_sets[a].insert(b);
        neighbor_sets[b].insert(a);
      }
    }
  }

  graph.neighbors.resize(graph.num_vertices);
  for (int v = 0; v < graph.num_vertices; ++v) {
    graph.neighbors[v].assign(neighbor_sets[v].begin(), neighbor_sets[v].end());
  }

  return graph;
}

// --- Frontier keys ---
// A tally slot is identified by the heights of the frontier vertices, one byte
// each (height + 128), packed into two 64-bit words.

namespace {

struct FrontierKey
{
  uint64_t words[2] = { 0, 0 };

  bool operator==(const FrontierKey& other) const
  {
    return words[0] == other.words[0] && words[1] == other.words[1];
  }
};

// splitmix64 finalizer. std::hash<uint64_t> is the identity on libc++, which
// clusters these structured keys into a few buckets and slows lookups badly.
uint64_t
mix(uint64_t x)
{
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

struct FrontierKeyHash
{
  size_t operator()(const FrontierKey& key) const
  {
    return mix(key.words[0] ^ mix(key.words[1]));
  }
};

FrontierKey
encode_frontier(const int* heights, const int& size)
{
  FrontierKey key;
  for (int i = 0; i < size; ++i) {
    if (heights[i] < -128 || heights[i] > 127) {
      throw std::overflow_error("frontier height out of one-byte range");
    }
    uint64_t byte = static_cast<uint8_t>(heights[i] + 128);
    key.words[i / 8] |= byte << (8 * (i % 8));
  }
  return key;
}

void
decode_frontier(const FrontierKey& key, const int& size, int* heights)
{
  for (int i = 0; i < size; ++i) {
    heights[i] = static_cast<int>((key.words[i / 8] >> (8 * (i % 8))) & 0xFF);
    heights[i] -= 128;
  }
}

// Precomputed bookkeeping for placing one vertex.
struct StepPlan
{
  // Frontier slots holding this vertex's already-placed neighbors.
  std::vector<int> neighbor_slots;
  // Layout of the next frontier: each entry is the current slot it is copied
  // from, or -1 for the vertex being placed.
  std::vector<int> next_sources;
  bool on_border = false;
};

// The frontier is every placed vertex that still has an unplaced neighbor.
// Walks the placement order once to work out, for every step, where each
// neighbor sits in the frontier and which vertices drop out afterwards.
std::vector<StepPlan>
plan_steps(const HeightGraph& graph, CountStats& stats)
{
  std::vector<int> unplaced_neighbors(graph.num_vertices, 0);
  for (int v = 0; v < graph.num_vertices; ++v) {
    for (int u : graph.neighbors[v]) {
      if (u > v)
        ++unplaced_neighbors[v];
    }
  }

  std::vector<StepPlan> plans(graph.num_vertices);
  std::vector<int> frontier;

  for (int v = 0; v < graph.num_vertices; ++v) {
    StepPlan& plan = plans[v];
    plan.on_border = graph.on_border[v];

    for (int u : graph.neighbors[v]) {
      if (u < v) {
        int slot = static_cast<int>(
          std::find(frontier.begin(), frontier.end(), u) - frontier.begin());
        plan.neighbor_slots.push_back(slot);
        --unplaced_neighbors[u];
      }
    }

    std::vector<int> next_frontier;
    for (size_t s = 0; s < frontier.size(); ++s) {
      if (unplaced_neighbors[frontier[s]] > 0) {
        next_frontier.push_back(frontier[s]);
        plan.next_sources.push_back(static_cast<int>(s));
      }
    }
    if (unplaced_neighbors[v] > 0) {
      next_frontier.push_back(v);
      plan.next_sources.push_back(-1);
    }

    if (static_cast<int>(next_frontier.size()) > MAX_FRONTIER) {
      throw std::length_error("frontier wider than MAX_FRONTIER");
    }
    stats.max_frontier =
      std::max(stats.max_frontier, static_cast<int>(next_frontier.size()));
    frontier = next_frontier;
  }

  return plans;
}

} // namespace

// --- Counting engine ---

BigCount
count_height_assignments(const HeightGraph& graph,
                         const bool& with_border,
                         const int& border_low,
                         const int& border_high,
                         CountStats& stats)
{
  std::vector<StepPlan> plans = plan_steps(graph, stats);

  // tally: frontier shape -> number of partial boards with that frontier.
  // Start from one empty partial board.
  std::unordered_map<FrontierKey, BigCount, FrontierKeyHash> tally;
  std::unordered_map<FrontierKey, BigCount, FrontierKeyHash> next_tally;
  tally[FrontierKey()] = BigCount(1);

  int frontier_size = 0;
  int heights[MAX_FRONTIER];
  int next_heights[MAX_FRONTIER];

  for (int v = 0; v < graph.num_vertices; ++v) {
    const StepPlan& plan = plans[v];
    const int next_size = static_cast<int>(plan.next_sources.size());
    next_tally.clear();
    next_tally.reserve(tally.size() * 2);

    for (const auto& [key, count] : tally) {
      decode_frontier(key, frontier_size, heights);

      // Allowed heights: within MAX_STEP of every placed neighbor, and inside
      // the border range for border vertices.
      int low = INT_MIN / 2;
      int high = INT_MAX / 2;
      bool anchored = false;
      for (int slot : plan.neighbor_slots) {
        low = std::max(low, heights[slot] - MAX_STEP);
        high = std::min(high, heights[slot] + MAX_STEP);
        anchored = true;
      }
      if (with_border && plan.on_border) {
        low = std::max(low, border_low);
        high = std::min(high, border_high);
        anchored = true;
      }
      if (!anchored) {
        // Only the very first vertex of a no-border count may be unanchored:
        // pin it to 0 so each shift class is counted once.
        if (with_border || frontier_size > 0) {
          throw std::logic_error("vertex placed with no anchor; height would "
                                 "be unbounded");
        }
        low = 0;
        high = 0;
      }

      for (int x = low; x <= high; ++x) {
        for (int i = 0; i < next_size; ++i) {
          int source = plan.next_sources[i];
          next_heights[i] = source < 0 ? x : heights[source];
        }

        // No border: only relative heights matter, so subtract the minimum.
        if (!with_border && next_size > 0) {
          int lowest =
            *std::min_element(next_heights, next_heights + next_size);
          for (int i = 0; i < next_size; ++i)
            next_heights[i] -= lowest;
        }

        next_tally[encode_frontier(next_heights, next_size)].add(count);
      }
    }

    std::swap(tally, next_tally);
    frontier_size = next_size;
    stats.peak_slots = std::max(stats.peak_slots, tally.size());
  }

  BigCount total;
  for (const auto& [key, count] : tally) {
    total.add(count);
  }
  return total;
}

BigCount
count_shift_classes(const HeightGraph& graph,
                    const bool& with_border,
                    CountStats& stats)
{
  if (!with_border) {
    return count_height_assignments(graph, false, 0, 0, stats);
  }

  // A shift class whose border vertices span a range of s (s <= 4) has
  // (5 - s) shifted copies with border heights in [-2, 2] and (4 - s) in
  // [-2, 1]. The difference counts every valid class exactly once.
  BigCount wide =
    count_height_assignments(graph, true, BORDER_LOW, BORDER_HIGH, stats);
  BigCount narrow =
    count_height_assignments(graph, true, BORDER_LOW, BORDER_HIGH - 1, stats);
  return wide.subtract(narrow);
}

BigCount
count_permutations(int rows,
                   int columns,
                   const bool& with_border,
                   CountStats& stats)
{
  // The frontier is about one row wide, so sweep along the longer side.
  if (columns > rows)
    std::swap(rows, columns);

  return count_shift_classes(
    build_grid_graph(rows, columns), with_border, stats);
}

BigCount
count_combinations(int rows,
                   int columns,
                   const bool& with_border,
                   CountStats& stats)
{
  // Transposing commutes with 180° rotation, so the narrow orientation gives
  // the same counts with a smaller frontier.
  if (columns > rows)
    std::swap(rows, columns);

  // Burnside: average, over the rotation group, the number of boards each
  // rotation leaves unchanged. If rotating gave the same board shifted by c,
  // rotating back to the start would give h = h + k*c, so c = 0: the fixed
  // boards are exactly the rotation-symmetric ones, counted on folded graphs.
  BigCount total =
    count_shift_classes(build_grid_graph(rows, columns), with_border, stats);
  BigCount fixed_180 = count_shift_classes(
    build_rotation_quotient_graph(rows, columns, 2), with_border, stats);
  total.add(fixed_180);

  uint32_t group_size = 2;
  if (rows == columns) {
    BigCount fixed_90 = count_shift_classes(
      build_rotation_quotient_graph(rows, columns, 1), with_border, stats);
    // 90° and 270° fix the same boards.
    total.add(fixed_90);
    total.add(fixed_90);
    group_size = 4;
  }

  if (total.divide(group_size) != 0) {
    throw std::logic_error("Burnside sum not divisible by the group size");
  }
  return total;
}

// --- Display helpers ---

long long
get_current_time_ms()
{
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration duration = now.time_since_epoch();
  std::chrono::milliseconds millis =
    std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  return millis.count();
}

void
print_count(const std::string& label,
            const int& rows,
            const int& columns,
            const BigCount& count,
            const CountStats& stats,
            const long long& elapsed_ms)
{
  std::cout << label << " for a " << rows << "x" << columns << " board:\n";
  std::cout << "\t" << count.to_string(true) << "\n";
  std::cout << "\tpeak tally slots: " << stats.peak_slots << "\n";
  std::cout << "\twidest frontier:  " << stats.max_frontier << "\n";
  std::cout << std::fixed << std::setprecision(1);
  std::cout << "\ttime:             " << elapsed_ms / 1000.0 << " seconds\n";
}

void
print_table(const std::string& title,
            const int& max_rows,
            const int& max_columns,
            const bool& with_border,
            const bool& combinations)
{
  std::vector<std::vector<std::string>> cells(
    max_rows + 1, std::vector<std::string>(max_columns + 1));

  for (int r = 1; r <= max_rows; ++r) {
    for (int c = 1; c <= max_columns; ++c) {
      // Counts are symmetric under transposition; reuse the mirrored cell.
      if (c < r && c <= max_rows && r <= max_columns && !cells[c][r].empty()) {
        cells[r][c] = cells[c][r];
        continue;
      }

      long long start = get_current_time_ms();
      CountStats stats;
      BigCount count = combinations
                         ? count_combinations(r, c, with_border, stats)
                         : count_permutations(r, c, with_border, stats);
      cells[r][c] = count.to_string(true);

      std::cerr << std::fixed << std::setprecision(1) << "\t" << r << "x" << c
                << ": " << (get_current_time_ms() - start) / 1000.0
                << " seconds, peak tally slots " << stats.peak_slots << "\n";
    }
  }

  std::cout << "#### " << title << "\n\n";
  std::cout << "| Rows x Cols |";
  for (int c = 1; c <= max_columns; ++c)
    std::cout << " " << c << " |";
  std::cout << "\n| --- |";
  for (int c = 1; c <= max_columns; ++c)
    std::cout << " --- |";
  std::cout << "\n";

  for (int r = 1; r <= max_rows; ++r) {
    std::cout << "| **" << r << "** |";
    for (int c = 1; c <= max_columns; ++c)
      std::cout << " " << cells[r][c] << " |";
    std::cout << "\n";
  }
}
