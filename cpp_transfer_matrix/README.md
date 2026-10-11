# Counting Boards with a Transfer Matrix

The scripts in this directory count every valid board configuration exactly, for boards up to 10x10, in minutes. The brute-force scripts in [`../cpp_brute_force/`](../cpp_brute_force/) list every board individually. That took about two months of continuous running to reach 6 billion boards. A 10x10 board has around 10^56.

The trick is to **count boards without ever listing them**. This document builds the idea from scratch with small examples you can check by hand, and you don't need any prior background to follow it. If you know what a board is and can multiply, you can follow it.

## Contents

1. [The problem](#1-the-problem)
2. [Why listing boards can't work](#2-why-listing-boards-cant-work)
3. [The key idea: the future only sees the edge](#3-the-key-idea-the-future-only-sees-the-edge)
4. [Vocabulary](#4-vocabulary)
5. [Warm-up: a board one row tall](#5-warm-up-a-board-one-row-tall)
6. [Two rows tall: slices and the 5x5 table](#6-two-rows-tall-slices-and-the-5x5-table)
7. [Placing one block at a time](#7-placing-one-block-at-a-time)
8. [Full trace of a 2x2 board](#8-full-trace-of-a-2x2-board)
9. [Scaling up to 10x10](#9-scaling-up-to-10x10)
10. [The border variant](#10-the-border-variant)
11. [Combinations: Burnside's lemma](#11-combinations-burnsides-lemma)
12. [How the code is organized](#12-how-the-code-is-organized)
13. [Usage](#13-usage)
14. [Results and verification](#14-results-and-verification)
15. [Where this comes from](#15-where-this-comes-from)

---

## 1. The problem

A board is a grid of blocks. Each block has an integer **height**. The only physical rule is:

> **Two blocks that share a side may differ in height by at most 2.**

Diagonal blocks don't share a side, so they aren't constrained directly.

Two boards are considered the same if one is the other with every block raised or lowered by the same amount. A flat board at height 0 and a flat board at height 3 are one board. Beyond that, there are four variants to count:

| | Rotations count as different | Rotations count as the same |
| --- | --- | --- |
| **No border** | permutations, no border | combinations, no border |
| **With border** | permutations, with border | combinations, with border |

**With border** means the board sits inside a wall at height 0. Every block on the outer ring must therefore be within 2 of 0, i.e. in the range [-2, 2]. Interior blocks are only limited by their neighbors.

## 2. Why listing boards can't work

The brute-force approach starts from one board, tries raising each block by 1, keeps every new board it hasn't seen before, and repeats. It is correct, but it has to **store every board** to recognize repeats, so time and memory grow with the number of boards.

The counts grow by a factor of about 3.7 *per block*. A 4x4 board has about 10^9 permutations, a 6x6 about 10^20, and a 10x10 about 10^56. No computer can store 10^56 of anything. Even an enumerator that never stores boards and visits a billion per second would need about 10^40 years.

So we need a method whose cost doesn't depend on the number of boards.

## 3. The key idea: the future only sees the edge

Build a board one block at a time, in reading order: left to right along the top row, then the next row, and so on.

When the next block is placed, it only has to agree with the blocks it touches: the one directly **above** it and the one directly to its **left**. Everything placed earlier and further away is invisible to it, and to every block after it.

So suppose two half-finished boards have different histories but the same heights along the **edge** where new blocks will attach. They have exactly the same number of ways to be completed. We don't need to keep them apart. We can merge them into one entry and just remember **how many** partial boards share that edge.

That is the whole method:

> Keep a tally: for each possible edge shape, how many partial boards end with that edge. Place blocks one at a time, updating the tally. When the board is complete, add up the tally.

The tally has one entry per edge *shape*, and there are far fewer shapes than boards. For a 10-wide board the tally peaks at about 3.5 million entries, while the board count reaches 10^56.

## 4. Vocabulary

| Term | Meaning |
| --- | --- |
| **Block** | One square of the board, at position (row, column). |
| **Height** | The integer value of a block. |
| **Neighbors** | Two blocks that share a side (up, down, left, right). Diagonal blocks are *not* neighbors. |
| **Legal board** | Every pair of neighbors differs in height by at most 2. |
| **Shift-equivalent** | Two boards are the same if one equals the other plus a constant on every block. |
| **Placement order** | Blocks are placed one at a time in reading order. Block *k* is the *k*-th placed. |
| **Partial board** | The first *k* blocks with heights chosen, where every neighbor pair placed so far is legal. |
| **Edge** (also **frontier**) | For each column, the height of the lowest block placed so far in that column, written as a tuple such as `(2, 0)`. These are the only placed blocks that future blocks can touch. In the first row the edge grows one entry at a time; after that, each new block *replaces* the entry above it. |
| **Normalize** | Subtract the edge's minimum from every entry, so the lowest entry is 0. `(0, -2)` becomes `(2, 0)`. This is how shift-equivalence is applied. |
| **Gap** | The difference between neighboring entries of the edge, `edge[i+1] - edge[i]`. The edge `(2, 0)` has gap -2. |
| **Step** | The point in the edge where newly placed blocks meet older ones. Those two entries are *diagonal* on the board, not neighbors, so their gap can range from -4 to +4. Every other gap is between true neighbors and ranges from -2 to +2. |
| **Slot** | One normalized edge shape. All partial boards with the same normalized edge share a slot. |
| **Count** | How many partial boards are in a slot. |
| **Tally** | The full map from slot to count. It holds everything the algorithm knows; individual boards are never stored. |
| **Allowed range** | The heights the next block can take: within 2 of the block above it (if there is one) and within 2 of the block to its left (if there is one). |
| **Choices** | How many heights are in the allowed range. |
| **Pin** | The very first block has no neighbors. It is fixed at height 0 so that each shift-equivalent family is counted once. |
| **Merge** | When two different partial boards produce the same normalized edge, their counts are added into one slot. This is what makes the method fast. |

## 5. Warm-up: a board one row tall

Lay blocks in a single row. Each new block touches only the block to its left, so it can be at that height -2, -1, 0, +1 or +2: five choices, every time.

```
1x1: [0]              → 1 board (pinned)
1x2: [0][?]           → 5
1x3: [0][?][?]        → 5 × 5 = 25
1xN                   → 5^(N-1)
```

That matches the first row of the permutations table: 1, 5, 25, 125, ... Here the "tally" has a single slot, because every edge looks the same: one block.

## 6. Two rows tall: slices and the 5x5 table

This section shows the classic form of the transfer matrix. The code doesn't use it directly; Section 7 explains why. It is still the clearest way to see the idea.

### Slices

Take a board two blocks tall and build it left to right, one **slice** at a time. A slice is one column of the board: a top block and a bottom block.

```
 slice: 1   2   3   4 ...
       [ ] [ ] [ ] [ ]      ← top row
       [ ] [ ] [ ] [ ]      ← bottom row
```

The two blocks in a slice are neighbors, so `d = bottom - top` is between -2 and +2. Up to a height shift, a slice is completely described by `d`, so there are **5 slice shapes**.

### Attaching one slice to another

Say the current last slice has shape `d` and we attach a new slice of shape `d'`. Take the slice with top 0 and bottom 2, so `d = 2`. The new slice has top `t'` and bottom `b' = t' + d'`:

```
         slice 1   slice 2
top:        0         t'      t' within 2 of 0
bottom:     2         b'      b' within 2 of 2
```

For each `d'`, count the values of `t'` that work:

| new shape d' | new slice (t', b') | valid t' | count |
| --- | --- | --- | --- |
| 2 | (t', t'+2) | -2, -1, 0, 1, 2 | 5 |
| 1 | (t', t'+1) | -1, 0, 1, 2 | 4 |
| 0 | (t', t') | 0, 1, 2 | 3 |
| -1 | (t', t'-1) | 1, 2 | 2 |
| -2 | (t', t'-2) | 2 | 1 |

In general, the top blocks allow a window of 5 heights for `t'` and the bottom blocks allow another window of 5. The bottom window is offset by `d - d'`, so the windows overlap in **5 - |d - d'|** heights (or 0 if that is negative). That number depends only on the two *shapes*, never on actual heights.

### The table

Collect those counts for every pair of shapes:

```
               new slice d'
              -2  -1   0  +1  +2
last   d=-2 [  5   4   3   2   1 ]
slice  d=-1 [  4   5   4   3   2 ]
       d= 0 [  3   4   5   4   3 ]
       d=+1 [  2   3   4   5   4 ]
       d=+2 [  1   2   3   4   5 ]   ← the example above
```

This is the **transfer matrix** T. Each entry answers: how many ways can a slice of shape `d'` attach to a slice of shape `d`? The table never changes. It is built once and reused at every seam between slices.

### Pushing the tally through the table

Keep a tally with one count per slice shape: how many boards built so far end in that shape.

- **One slice.** Each shape can be built exactly one way (pin the top at 0), so the tally is (1, 1, 1, 1, 1), total **5**: the 2x1 count.
- **Each new slice.** For each new shape `d'`, look at every possible previous shape `d`. Multiply its count by `T[d][d']`, then add the results:

```
new[d'] = Σ over d of  old[d] × T[d][d']
```

Going from one slice to two, the old counts are all 1, so each new count is a column sum of T:

```
new[-2] = 1×5 + 1×4 + 1×3 + 1×2 + 1×1 = 15
new[-1] = 1×4 + 1×5 + 1×4 + 1×3 + 1×2 = 18
new[ 0] = 1×3 + 1×4 + 1×5 + 1×4 + 1×3 = 19
...
tally = (15, 18, 19, 18, 15), total 85   → the 2x2 count
```

Going from two slices to three, the old counts now weight each table entry:

```
new[-2] = 15×5 + 18×4 + 19×3 + 18×2 + 15×1 = 255
new[ 0] = 15×3 + 18×4 + 19×5 + 18×4 + 15×3 = 329
tally = (255, 310, 329, 310, 255), total 1,459   → the 2x3 count
```

Every slice after that is the same operation. The **table stays fixed and the tally grows**. In matrix notation, the 2xN count is `1ᵀ · T^(N-1) · 1`.

Note that this machine only ever builds boards that are **2 tall**. It moves along one row of the results table (2x1, 2x2, 2x3, ...). A 3-tall board needs a different machine: a slice of 3 blocks has two internal gaps, so 5 × 5 = 25 shapes and a 25x25 table. A *k*-tall board has 5^(k-1) slice shapes.

### Why the growth factor converges

After many slices, each new slice multiplies the total by almost exactly the same factor: T's largest eigenvalue, about 17.18. This is the "Row Growth Factor" that the top-level README observed converging. The second-order factor converges for the same reason, and its limit is the per-block growth constant.

## 7. Placing one block at a time

A 10-tall slice has 5^9 ≈ 1.95 million shapes. The slice table would need 1.95 million × 1.95 million ≈ 4 trillion entries, which is far too many to store.

The fix is to stop attaching whole slices and attach **one block at a time**. The rule for a single block is tiny and never needs to be written down as a table:

> The new block may take any height within 2 of the block above it and within 2 of the block to its left.

That is at most 5 choices.

Between blocks, the tally is keyed by the **edge**: for each column, the most recently placed block. In the middle of a row the edge is part new row and part old row, with one **step** where they meet:

```
            c=0  c=1  c=2  c=3  c=4
row r-1:     ·    ·    U    A    B
row r  :     X    Y    ?
```

The edge is `[X, Y, U, A, B]`. The new block `?` must be within 2 of `Y` (left) and `U` (above). After it is placed, `U` can never be touched again, so the edge becomes `[X, Y, ?, A, B]`.

Each block placement does the same thing to every slot in the tally:

1. Work out the allowed range from the neighbors in the edge.
2. For each allowed height, form the new edge and normalize it.
3. Add the slot's whole count into the new edge's slot.

The 5x5 table from Section 6 is exactly two of these block placements bundled together. The full trace below shows that.

## 8. Full trace of a 2x2 board

Blocks are placed in this order:

```
[1][2]
[3][4]
```

These are the tallies the algorithm actually produces at each step. The pseudocode is in [Section 12](#12-how-the-code-is-organized).

### Block 1: pinned

No neighbors, so it is fixed at 0.

| edge | count |
| --- | --- |
| (0) | 1 |

### Block 2: one neighbor (left)

Allowed range is -2..2. It is in the first row, so the edge grows. Normalization appears here: `(0, -2)` becomes `(2, 0)`.

| edge | gap | count |
| --- | --- | --- |
| (2, 0) | -2 | 1 |
| (1, 0) | -1 | 1 |
| (0, 0) | 0 | 1 |
| (0, 1) | +1 | 1 |
| (0, 2) | +2 | 1 |

5 slots, 5 partial boards: every 1x2 board.

### Block 3: one neighbor (above)

Block 3's only neighbor is block 1. Block 2 is diagonal to it and is **not** checked, so every slot has 5 choices. The new block replaces block 1 in the edge, so the edge is now (block 3, block 2). Those two are diagonal, so the gap can range from -4 to +4: this is the step.

Merging happens here. Slot `(0, 0)` receives one partial board from each of the 5 incoming slots, so its count climbs 1 → 2 → 3 → 4 → 5. For example, these two partial boards have the same edge `(0, 0)` and become indistinguishable to block 4:

```
[2][0]        [0][2]
[0]           [2]
```

Result:

| edge | gap | count |
| --- | --- | --- |
| (4, 0) | -4 | 1 |
| (3, 0) | -3 | 2 |
| (2, 0) | -2 | 3 |
| (1, 0) | -1 | 4 |
| (0, 0) | 0 | 5 |
| (0, 1) | +1 | 4 |
| (0, 2) | +2 | 3 |
| (0, 3) | +3 | 2 |
| (0, 4) | +4 | 1 |

9 slots, 25 partial boards.

### Block 4: two neighbors (above and left)

Block 4 touches block 2 (above) and block 3 (left). Both are checked. Those two differ by the slot's gap `g`, so the windows overlap in **5 - |g|** heights. Each choice carries the slot's full count forward:

| edge | count | × choices | = |
| --- | --- | --- | --- |
| (4, 0) | 1 | × 1 | 1 |
| (3, 0) | 2 | × 2 | 4 |
| (2, 0) | 3 | × 3 | 9 |
| (1, 0) | 4 | × 4 | 16 |
| (0, 0) | 5 | × 5 | 25 |
| (0, 1) | 4 | × 4 | 16 |
| (0, 2) | 3 | × 3 | 9 |
| (0, 3) | 2 | × 2 | 4 |
| (0, 4) | 1 | × 1 | 1 |
| | | **total** | **85** |

The edge is now the full bottom row, so gaps are back to -2..+2. Grouped by that gap, the final tally is (15, 18, 19, 18, 15). That is the same tally the 5x5 table produced, which confirms that blocks 3 and 4 together performed one step of the table.

### Continuing to 3x2

Add a third row:

```
[1][2]
[3][4]
[5][6]
```

**Block 5** has one neighbor (block 3), so each slot spreads its count into the 5 slots whose gap is within 2 of its own:

```
new gap:           -4   -3   -2   -1    0   +1   +2   +3   +4
from gap -2 (15):   15   15   15   15   15
from gap -1 (18):        18   18   18   18   18
from gap  0 (19):             19   19   19   19   19
from gap +1 (18):                  18   18   18   18   18
from gap +2 (15):                       15   15   15   15   15
                   ---------------------------------------------
after block 5:      15   33   52   70   85   70   52   33   15     (total 425 = 85 × 5)
```

**Block 6** has two neighbors, so multiply each slot by its choices (5 - |gap|) and sum:

```
15×1 + 33×2 + 52×3 + 70×4 + 85×5 + 70×4 + 52×3 + 33×2 + 15×1 = 1,459
```

That is the 3x2 count. It equals the 2x3 count, since it is the same board turned sideways.

### The pattern

- A block with **one** placed neighbor always has 5 choices. Each slot spreads its count into 5 new slots.
- A block with **two** placed neighbors has 5 - |gap| choices. That depends on the slot, and it is where the board's structure enters.
- **Merging** happens whenever different histories produce the same edge.

## 9. Scaling up to 10x10

Nothing changes except size:

- The edge has 10 entries instead of 2.
- There are 100 block placements instead of 4.
- The tally peaks at **3,515,625 slots**. Of the 9 gaps in a 10-wide edge, 8 are between true neighbors (5 values each) and 1 is the step (9 values), so the count is 5^8 × 9.
- Each block placement makes one pass over the tally with at most 5 choices per slot. That is about 1.75 billion small additions over the whole board.

**Cost:** the work grows with *(number of blocks) × (number of edge shapes)*. The edge-shape count grows by about 5× for each extra block of width, but only linearly with board length. It doesn't depend on the number of boards at all. So 10x10 takes minutes, 10x1000 would take hours, and 30x30 (5^28 edge shapes) would be out of reach.

**Orientation:** the edge is one row wide, so the scripts always sweep along the longer side and keep the edge as short as possible. Counts don't change when a board is transposed.

**Big numbers:** 10x10 counts are around 10^56, far beyond 64-bit integers. The scripts use a small 256-bit integer type, which throws an error rather than silently overflowing.

## 10. The border variant

With a border, edge blocks must be in [-2, 2] **absolutely**, so the tally can no longer normalize heights. It keeps actual heights, and blocks on the outer ring are additionally limited to [-2, 2]. This roughly doubles the number of slots: 10x10 peaks around 6.7 million.

Boards that differ by a constant shift are still considered the same. A shift is allowed as long as the outer ring stays inside [-2, 2]. To count each shift family once, use this identity. Suppose a family's outer-ring heights span a range of `s` (from lowest to highest, at most 4):

- It has **5 - s** shifted copies whose outer ring fits in [-2, 2].
- It has **4 - s** shifted copies whose outer ring fits in [-2, 1].

The difference is exactly 1 for every family. So:

```
shift families with border = count(outer ring in [-2, 2]) - count(outer ring in [-2, 1])
```

Each term is an ordinary tally sweep with absolute heights. Check: a 1x1 board has `s = 0`, so 5 - 4 = 1.

## 11. Combinations: Burnside's lemma

Combinations treat a board and its rotations as the same. Dividing the permutation count by 4 is almost right, but not exactly. A symmetric board looks identical under some rotations, so it would be undercounted.

**Burnside's lemma** gives the exact count: average, over all rotations, the number of boards that each rotation leaves unchanged.

```
square board:     combinations = (all + fixed_180 + 2 × fixed_90) / 4
rectangular board: combinations = (all + fixed_180) / 2
```

- `all` is the permutation count. The 0° rotation leaves every board unchanged.
- `fixed_180` counts boards that look the same after a 180° turn.
- `fixed_90` counts boards that look the same after a 90° turn. A 270° turn fixes the same boards, hence the factor of 2.
- Rectangles only have 0° and 180°, because a 90° turn changes their shape.

**Check on 2x2:** 85 permutations, 5 boards survive a 180° turn and 1 survives a 90° turn. That gives (85 + 5 + 2 × 1) / 4 = **23**, which matches the brute-force result.

**Shift-equivalence doesn't spoil this.** Suppose rotating a board gave the same board shifted by a constant `c`. Rotating all the way back around would then give `h = h + k·c` for some k ≥ 2, so `c = 0`. The fixed boards are exactly the boards that are truly symmetric.

### Counting symmetric boards by folding

A board that is unchanged by a rotation is determined by one block from each **orbit**: a set of blocks the rotation cycles between. For 180°, block (r, c) is paired with block (rows-1-r, columns-1-c), so it is enough to choose the top half of the board.

So build a smaller "folded" graph:

- Each orbit becomes a single vertex.
- Two vertices are connected if any of their blocks are neighbors on the board.
- A vertex is on the border if its blocks are on the outer ring. Rotation maps the outer ring to itself, so all blocks of an orbit agree.

Symmetric boards are exactly the legal height assignments on this folded graph. They are counted with **the same tally sweep**: the code's engine works on any graph, with vertices placed in order and the frontier being every placed vertex that still has an unplaced neighbor.

For example, folding a 2x2 board by 180° pairs block 1 with block 4 and block 2 with block 3. The folded graph is two vertices joined by an edge. Pin one vertex at 0, and the other has 5 choices, so `fixed_180 = 5`.

These symmetric counts are tiny compared with the totals. At 10x10 without a border, `fixed_180 + 2 × fixed_90` is about 1.7 × 10^28, against a total around 7.5 × 10^56. That is why combinations ≈ permutations / 4 to many digits, but the scripts compute the exact value.

## 12. How the code is organized

```
cpp_transfer_matrix/
├── board_game_transfer_matrix_utils.h/.cpp   counting engine + helpers
├── board_game_permutations_no_border.cpp     entry point
├── board_game_permutations_with_border.cpp   entry point
├── board_game_combinations_no_border.cpp     entry point
└── board_game_combinations_with_border.cpp   entry point
```

The four entry points only parse arguments and call one of two functions:

- `count_permutations(rows, columns, with_border)` builds the grid graph and counts its shift families.
- `count_combinations(rows, columns, with_border)` applies Burnside's lemma using the grid graph plus the 180° and 90° folded graphs.

Both rely on `count_height_assignments`, the engine. In pseudocode:

```
tally = { empty edge: 1 }
for each vertex v, in placement order:
    new_tally = {}
    for each (edge, count) in tally:
        allowed = heights within 2 of every placed neighbor of v
                  (and within [-2, 2] or [-2, 1] if v is on the border)
                  (the very first vertex with no border is pinned to 0)
        for each height x in allowed:
            new_edge = edge with v's entry set to x,
                       minus vertices that no longer have unplaced neighbors
            if no border: new_edge = normalize(new_edge)
            new_tally[new_edge] += count
    tally = new_tally
answer = sum of all counts in tally
```

Implementation notes:

- **Step plans.** Before counting, `plan_steps` walks the placement order once. For each vertex it records where its placed neighbors sit in the edge, and which edge entries survive to the next step. The inner loop is then just array copies.
- **Slot keys.** An edge is packed into a 16-byte key, one byte per height. This allows edges up to 16 entries wide.
- **Hashing.** Keys are hashed with a splitmix64 mix. On macOS, `std::hash<uint64_t>` is the identity function. These structured keys would then pile into a handful of buckets, and a 10x10 run would take hours instead of minutes.
- **Memory.** Two tallies exist at once: the current one and the one being built. Measured peak memory is about 175 bytes per peak slot without a border and about 290 with one.

## 13. Usage

Build from the repository root:

```bash
make compile_transfer_matrix
```

Count one board:

```bash
./cpp_transfer_matrix/board_game_permutations_no_border -r 10 -c 10
./cpp_transfer_matrix/board_game_combinations_with_border -r 4 -c 7
```

Print a Markdown results table for every size up to R x C, in the same layout as the top-level README. Per-size timing goes to stderr:

```bash
./cpp_transfer_matrix/board_game_permutations_no_border -t -r 10 -c 10
```

| flag | meaning |
| --- | --- |
| `-r`, `--rows` | board rows (default 2) |
| `-c`, `--columns` | board columns (default 2) |
| `-t`, `--table` | print a table for all sizes up to rows x columns |

Approximate cost on an Apple-silicon laptop (single thread):

| board | no border | with border |
| --- | --- | --- |
| 8x8 | 1.4 s, 28 MB | 4.0 s, 103 MB |
| 9x9 | 14 s, 124 MB | 42 s, 527 MB |
| 10x10 | 122 s, 612 MB | 314 s, 1.9 GB |

Combinations cost roughly 1.4–1.5× the matching permutation run, because of the extra folded-graph sweeps: 10x10 took 188 s and 691 MB without a border, and 456 s and 2.1 GB with one. Each extra block of board width multiplies time by about 10 and memory by about 5. **Before trying 11x11, expect roughly 20–30 minutes and 5–10 GB of RAM without a border, and more with one.**

## 14. Results and verification

### Agreement with brute force

Every count the brute-force scripts produced (recorded in `All_Calcs.xlsx`) is reproduced exactly, across all four variants, with one exception: **2x8 permutations, no border**. Brute force recorded 2,182,057,437, while this method gives **2,182,058,437**, exactly 1,000 more. Two further independent checks agree with 2,182,058,437: a column-by-column absolute-height count and a plain depth-first enumeration of every board. The brute-force figure is most likely a transcription typo or a run that lost boards.

### 10x10

| variant | exact count | approx. |
| --- | --- | --- |
| permutations, no border | 751,446,302,681,050,365,478,027,416,872,495,407,843,190,771,097,924,033,841 | 7.51 × 10^56 |
| combinations, no border | 187,861,575,670,262,591,369,506,854,222,386,252,907,469,016,171,654,814,662 | 1.88 × 10^56 |
| permutations, with border | 54,601,738,082,914,428,488,213,712,370,730,043,240,371,492,708,377,110,095 | 5.46 × 10^55 |
| combinations, with border | 13,650,434,520,728,607,122,053,428,094,618,225,749,856,819,154,133,172,617 | 1.37 × 10^55 |

The 100th root of each permutation count gives the effective number of positions per block: **3.705** without a border and **3.608** with one. The top-level README's 3.54 estimate was a slight underestimate, as it anticipated. All four counts are below the Shannon number (10^120), but far above estimates of Catan's setup count.

## 15. Where this comes from

This is the **transfer-matrix method**, a standard technique in combinatorics and statistical physics. It is used to count domino tilings, map colorings and self-avoiding walks, and to compute partition functions of lattice models. In physics terms, these boards are a *solid-on-solid* height model: a surface of columns whose neighboring heights can't differ by too much. The block-by-block version is often called *broken-profile* dynamic programming. Burnside's lemma, also called the Cauchy–Frobenius lemma, is the standard tool for counting objects up to symmetry.
