# Board Game

A novel board game that addresses issues prominent in many games, but particularly classical games, such as chess and go.

## Origin Story
Something about [chess](https://en.wikipedia.org/wiki/Chess) never felt quite right to me. It's perhaps the most popular board game to ever exist. It's touted as intellectual sparring and revered for all the possibilities and complex gameplay. However, it greatly rewards memorization and studying. It's impossible to get far without learning the opening theory and new players are punished by early game traps. Particularly in the modern computer era it's all the more accessible to study various computer lines out 40 moves and curbstomp online players simply because one has memorized all the variations of a specific move sequence.

Chess and [go](https://en.wikipedia.org/wiki/Go_(game)) (and other games, although I'll be primarily targeting the first two) have these pitfalls because:

1. The initial starting position is always the same
2. Therefore games often become tests of memorization since optimal move sequences can be studied ahead of time
3. This in turn makes it so that expert gameplay explores a relatively small subset of what are often incredibly wide game trees

When all games start from the same position, all possible ways a game can begin can be calculated ahead of time. Thus players figure out the best strategy to win before even starting a game. [As Bobby Fischer said "e4 is best by test"](https://en.wikipedia.org/wiki/Bobby_Fischer#Opening_theory), so it's no surprise that at the professional and even advanced amateur levels, the first dozen or more moves of a chess game are essentially scripted. These few specific opening lines are memorized dozens of moves into the future. Undoubtedly there is still strategy, but when so much of the game can be solved ahead of time it begs the question if it's really a true test of skill.

The [Shannon Number](https://en.wikipedia.org/wiki/Shannon_number) (10^120) is widely considered a lower bound of how many possible chess games could exist. However, all of those possible games still start from the standard chess starting position. What if there was a game where each game had its _own_ unique game tree? If there were a sufficiently large number of unique starting positions, it wouldn't be possible, let alone worth the effort, to precompute all the lines and openings. You would be better off honing your strategic thinking and trying to play well, which brings us back to a game that is a true intellectually rigorous test.

This isn't a new idea. [Catan](https://en.wikipedia.org/wiki/Catan) does this very well as each game starts with randomized resource locations, which is perhaps in part why it's been so successful and popular. Similarly, [Stratego](https://en.wikipedia.org/wiki/Stratego) has player selected (ie. randomized) positions and therefore each game has its own unique game tree.

These opinions are neither novel nor niche. [Bobby Fischer invented Fischer Random Chess](https://en.wikipedia.org/wiki/Chess960) to **reduce the emphasis on opening preparation and to encourage creativity in play** by shuffling the back rank pieces. Magnus Carlsen is also now pushing Fischer Random with this brilliant headline [_"The Greatest Chess Player of All Time Is Bored With Chess"_](https://www.wsj.com/sports/chess-magnus-carlsen-fabiano-caruano-world-championship-e54c9fc5).

What does it say about a game when the best players of all time think it's stale?

## Motivation: Crush the Shannon Number

From chess's single starting position there are Shannon Number possible games. I want a game with Shannon Number possible starting positions. If each starting position has its own equivalently large game tree, then the total number of possible games across all starting positions would be Shannon Number squared (>10^240). Having so many different setups would greatly diminish the value of opening theory and memorizing lines.

## Concept: Board Games are Flat, Real Life is 3D

It's in the name, board game, board, boards are flat. Real life is 3D. Why isn't there a game where players can ascend mountains and dive into valleys? [Warhammer](https://en.wikipedia.org/wiki/Warhammer_(game)) has great environments, but they're fixed, not to mention it's not the most accessible game. My goal is something as simple as chess in terms of learning and setup, but the "board" can be easily changed to simulate a wide variety of terrains.

## Woodshop vs Laptop

The original board game idea probably came to me around 2015. Since then it's mostly been a random thought in my head that pops up about twice a year. All this time I had thought it would be a physical wooden contraption that would bring this idea to life. This is important as it meant I came up with the game physics based on what I thought I could cut, drill, etc. to achieve a physical object that could move the way I wanted. Unfortunately, laptops are much easier to come by than woodshops and since I had recently learned to code, I thought why not give it a shot. Both coding up the game logic and counting the setup permutations to see if it's greater than the Shannon Number (10^120). 

## Board Physics

The board game is 10 by 10 square grid made of 100 blocks. Each block can move a maximum of 2 units relative to its orthogonal adjacent blocks (up, down, left, right - **not** diagonals). Each unit of height is marked by a notch on the block, discretizing height into whole-number increments. If I've already lost you just scroll down to the [Wood Version](#wood-version) section for a visual aid.

A wooden version would need to be enclosed by a thin wall to contain all the blocks. Now that I've 3D printed this board with a different interlocking mechanism, or for a virtual version, the border is not necessary. This leads to different numbers of permutations depending on whether or not there is a border.

Set atop a flat surface, all blocks are flush, thus having a height of 0. The border holding the pieces together is a boundary set at height = 0. Therefore, it follows that the outermost blocks (the 10x10 ring) can be at a maximum height = 2 or minimum height = -2. Thus, any edge block can have a height in the set of [-2, -1, 0, 1, 2] providing the neighboring blocks permit it. This condition is that each block can only have at most 2 units of height difference between itself and any one of its 4 adjacent neighbors.  

Now it might help to start visualizing a bird's eye view of the board with each square displaying a number representing its height. This is the board sitting flat on a table, all blocks at height 0: 

<pre>
[[0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.],  
 [0. 0. 0. 0. 0. 0. 0. 0. 0. 0.]]
</pre>

 If we imagine the board like this with x and y as indices the neighbor condition can be written as:

<pre>
abs(board[x, y] - board[x - 1, y]) <= 2
abs(board[x, y] - board[x + 1, y]) <= 2
abs(board[x, y] - board[x, y - 1]) <= 2
abs(board[x, y] - board[x, y + 1]) <= 2
</pre>

Now with these constraints and the border boundary condition we would like to randomize the board to get something that looks like this:

<pre>
[[ 0.  0.  1.  1.  0.  0.  0. -1.  0.  1.]  
 [-1.  1. -1. -1.  0. -2. -1.  1.  1. -1.]
 [-1. -1.  0. -1. -1.  0.  1.  1.  1.  1.]
 [-1.  0. -2. -1.  1.  0.  1.  1. -1. -1.]
 [-2.  0.  0. -1.  1. -1. -1.  1.  0.  0.]
 [-1.  1.  2.  1.  1. -1. -1.  0. -1.  0.]
 [ 1.  0.  0.  0.  0. -1.  1.  2.  0.  0.]
 [-1. -1.  0. -1.  1.  0. -1.  1.  2.  0.]
 [-2. -2.  0.  1. -1.  1.  1.  1.  0. -1.]
 [ 0. -1.  1.  1. -1.  0.  0.  0.  2.  0.]]
</pre>

I implore you to check that none of our in game physics are broken. Take a minute to think about it because what comes next might be a surprise. Let's say you have the 0/flat board on a table. All blocks have height = 0. Now you pick it up and hold it out over the air... what happens?  

You get the playing field known as the Mariana Trench (left) because with nothing to support the block each block will fall its maximum 2 units from its neighbors (mechanism to physically allow this is a mechanical engineering problem for another time). At the other extreme you have the field known as Mt. Everest (right):

<pre>
[[ -2.  -2.  -2.  -2.  -2.  -2.  -2.  -2.  -2.  -2.]   [[ 2.  2.  2.  2.  2.  2.  2.  2.  2.  2.]
 [ -2.  -4.  -4.  -4.  -4.  -4.  -4.  -4.  -4.  -2.]    [ 2.  4.  4.  4.  4.  4.  4.  4.  4.  2.]
 [ -2.  -4.  -6.  -6.  -6.  -6.  -6.  -6.  -4.  -2.]    [ 2.  4.  6.  6.  6.  6.  6.  6.  4.  2.]
 [ -2.  -4.  -6.  -8.  -8.  -8.  -8.  -6.  -4.  -2.]    [ 2.  4.  6.  8.  8.  8.  8.  6.  4.  2.]
 [ -2.  -4.  -6.  -8. -10. -10.  -8.  -6.  -4.  -2.]    [ 2.  4.  6.  8. 10. 10.  8.  6.  4.  2.]
 [ -2.  -4.  -6.  -8. -10. -10.  -8.  -6.  -4.  -2.]    [ 2.  4.  6.  8. 10. 10.  8.  6.  4.  2.]
 [ -2.  -4.  -6.  -8.  -8.  -8.  -8.  -6.  -4.  -2.]    [ 2.  4.  6.  8.  8.  8.  8.  6.  4.  2.]
 [ -2.  -4.  -6.  -6.  -6.  -6.  -6.  -6.  -4.  -2.]    [ 2.  4.  6.  6.  6.  6.  6.  6.  4.  2.]
 [ -2.  -4.  -4.  -4.  -4.  -4.  -4.  -4.  -4.  -2.]    [ 2.  4.  4.  4.  4.  4.  4.  4.  4.  2.]
 [ -2.  -2.  -2.  -2.  -2.  -2.  -2.  -2.  -2.  -2.]]   [ 2.  2.  2.  2.  2.  2.  2.  2.  2.  2.]]
</pre>

Without a border, blocks are no longer constrained by an edge wall, allowing for even more extreme terrains. The borderless version includes the Mariana Trench and Mt. Everest configurations, but its most extreme form is a diagonal ramp:

<pre>
[[ 0.  2.  4.  6.  8. 10. 12. 14. 16. 18.]
 [ 2.  4.  6.  8. 10. 12. 14. 16. 18. 20.]
 [ 4.  6.  8. 10. 12. 14. 16. 18. 20. 22.]
 [ 6.  8. 10. 12. 14. 16. 18. 20. 22. 24.]
 [ 8. 10. 12. 14. 16. 18. 20. 22. 24. 26.]
 [10. 12. 14. 16. 18. 20. 22. 24. 26. 28.]
 [12. 14. 16. 18. 20. 22. 24. 26. 28. 30.]
 [14. 16. 18. 20. 22. 24. 26. 28. 30. 32.]
 [16. 18. 20. 22. 24. 26. 28. 30. 32. 34.]
 [18. 20. 22. 24. 26. 28. 30. 32. 34. 36.]]
</pre>
 
### Wood Version
Turns out I have CAD drawings (January 2016) for the wood version and a rendering:  
![](https://github.com/darkfireXXI/Board_game/blob/images/Render1.png)

## Board Permutation Counting

Now that we've established the board physics we want to figure out how many different permutations there are between the two extremities shown above? Both for the board with and without a border.

We should also clarify that throughout the rest of this the words combinations and permutations will have these definitions in our context:

**Combination:** a unique arrangement of a board, excluding 90 degree rotations and fixed height shifts

**Permutation**: a unique arrangement of a board, excluding fixed height shifts

Permutations are more relevant to gameplay. A single board with no rotational symmetry provides 4 unique play experiences depending on which side player 1 is positioned.

Let's start small. A 1x1 board; one square with values -2 <= height <= 2. We can increment through the heights and rotate it how we like, but in the end there's only one unique permutation. Next we can take a 2x2 board. We start at the lower extremity with each square's height value = -2. With two nested for loops we can increment each square on the board once. How many times do we have to do this?  

<pre>
(4 squares * 2 height) - (4 squares * -2 height) = 16 total moves from the lowest board to the highest
</pre>

We record the starting board as a unique board result. Then we increment one square one unit and check to see if this new board is already recorded among the unique board results. The verification checks include 90° rotations and checking height offsets. A board with all values at 0 is the same as a board with all values at 2. If the new board doesn't match any of the unique board results we add it to the list.

This would be the first of 4 squares to check in the 2x2 example. We would then return to our starting board and try moving the additional 3 squares and repeating the checks, adding new unique boards if found.

Now those 16 moves calculated above should really be thought of as 16 rounds. In each round, we move one block up one unit up, check if the resulting board is unique, then return to the starting board and repeat for the remaining blocks. In the subsequent round, the starting board is no longer the board of heights all -2. We now have several starting boards, which are all the unique board results found in the previous round.

We repeat this until all the rounds and permutations are exhausted.

For perspective with the full size 10x10 board...

<pre>
sum(Mt. Everest board) - sum(Mariana Trench Board) = 880 total rounds of moves from the lowest board to the highest
</pre>

And for each board result we can increment it 100 ways, one increment for each of the 100 blocks.

Yeah, so that's not going to be solved anytime soon on a laptop. I originally ran this on my laptop, then moved it to AWS, and eventually refactored the counting scripts from python to c++ and ran on a desktop formerly used for algorithmic trading backtesting. All this in effort to count the number of permutations for the 10x10 board.

### 2x2 Permutation Tree

The tree for a 2x2 board is shown below and confirms the number of permutations found by the code. Making any additional moves from the leaf nodes would either break the board's physics or create a repeat of a previous permutation (eg. rotation or board height values are all +n height more than a previous board). Hopefully this illustrates that even for a small board the number of permutations quickly gets out of hand.

As the boards become bigger we not only have more squares, but also the range of height gets larger. It actually scales with the size of the board: 2x2 ranges from -2 to +2 in height while 10x10 can go from -10 to +10 assuming a border. A borderless 10x10 board can range from 0 to 36.

![](https://github.com/darkfireXXI/Board_game/blob/images/tree.jpg)

## Permutation Theory

As it is compute intensive to brute force count all the permutations, I made an exploration into the theory to check if I was on track. Each square of the board can be considered to start at height 0 and have the ability to move to heights -2, -1, 0, 1, and 2. This gives each square 5 possible heights. As a back-of-the-envelope calculation we can estimate the number of permutations for a 10x10 board as 5^(10 * 10) = 7.888 * 10^69. This can be our upper bound as to how many permutations exist. This overcounts the actual number because it ignores the adjacency constraint. For example, it includes boards where one block is at -2 and its neighbor is at 2, which would be invalid. Not to mention, for the board with a border, that creates a boundary condition that further limits permutations. Of course, we would also like to not include duplicates that can be made via adding a constant to the height of every square or rotating the board 90°.

## Permutations

Below are the results of running the c++ counting script. To be more comprehensive I included non-square boards as well to get more data points. We're a ways from being able to calculate the number of permutations for a borderless 10x10 board, however the values are clearly increasing exponentially... 

#### Permutations Array No Border

| Rows x Cols | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **1** | 1 | 5 | 25 | 125 | 625 | 3,125 | 15,625 | 78,125 | 390,625 | 1,953,125 |
| **2** | 5 | 85 | 1,459 | 25,061 | 430,491 | 7,394,885 | 127,027,835 | 2,182,057,437 | ? | ? |
| **3** | 25 | 1,459 | 87,825 | 5,305,715 | 320,673,823 | ? | ? | ? | ? | ? |
| **4** | 125 | 25,061 | 5,305,715 | 1,133,641,543 | ? | ? | ? | ? | ? | ? |
| **5** | 625 | 430,491 | 320,673,823 | ? | ? | ? | ? | ? | ? | ? |
| **6** | 3,125 | 7,394,885 | ? | ? | ? | ? | ? | ? | ? | ? |
| **7** | 15,625 | 127,027,835 | ? | ? | ? | ? | ? | ? | ? | ? |
| **8** | 78,125 | 2,182,057,437 | ? | ? | ? | ? | ? | ? | ? | ? |
| **9** | 390,625 | ? | ? | ? | ? | ? | ? | ? | ? | ? |
| **10** | 1,953,125 | ? | ? | ? | ? | ? | ? | ? | ? | ? |

### How Exponential?

What if we then took the ratio of each row from the one before it? Essentially measuring the growth factor as we increase the number of rows in the board; a derivative of sorts.

#### Permutations Array No Border: Row Growth Factor

| Rows \ Cols | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **1** | - | - | - | - | - | - | - | - | - | - |
| **2** | 5 | 17 | 58.36 | 200.49 | 688.79 | 2,366.36 | 8,129.78 | 27,930.34 |  |  |
| **3** | 5 | 17.16 | 60.20 | 211.71 | 744.90 |  |  |  |  |  |
| **4** | 5 | 17.18 | 60.41 | 213.66 |  |  |  |  |  |  |
| **5** | 5 | 17.18 | 60.44 |  |  |  |  |  |  |  |
| **6** | 5 | 17.18 |  |  |  |  |  |  |  |  |
| **7** | 5 | 17.18 |  |  |  |  |  |  |  |  |
| **8** | 5 | 17.18 |  |  |  |  |  |  |  |  |
| **9** | 5 |  |  |  |  |  |  |  |  |  |
| **10** | 5 |  |  |  |  |  |  |  |  |  |

### Convergence

It appears the growth factor for adding a single additional row to any given board slowly starts to converge on a constant value. Naturally as the number of columns increases, these values increase accordingly, but also exponentially. This convergence is intuitive: each new row only interacts with its immediate neighbor. Therefore as we add a new row for any board with fixed n columns, its multiplicative contribution stabilizes. What if we did this again, but now taking the ratios across the columns?

#### Permutations Array No Border: Second-Order Growth Factor

| Rows \ Cols | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **1** | - | - | - | - | - | - | - | - | - | - |
| **2** | - | 3.40 | 3.43 | 3.44 | 3.44 | 3.44 | 3.44 | 3.44 |  |  |
| **3** | - | 3.43 | 3.51 | 3.52 | 3.52 |  |  |  |  |  |
| **4** | - | 3.44 | 3.52 | 3.54 |  |  |  |  |  |  |
| **5** | - | 3.44 | 3.52 |  |  |  |  |  |  |  |
| **6** | - | 3.44 |  |  |  |  |  |  |  |  |
| **7** | - | 3.44 |  |  |  |  |  |  |  |  |
| **8** | - | 3.44 |  |  |  |  |  |  |  |  |
| **9** | - |  |  |  |  |  |  |  |  |  |
| **10** | - |  |  |  |  |  |  |  |  |  |

### Average Positions Available per Block

As we established earlier in the Permutation Theory section, any two blocks can have at most 5 distinct positions relative to one another. After calculating the second-order growth factor, we start to converge on a value that takes into account that on an actual board, due to the physical constraints and positions of neighboring blocks all 5 positions will not always be available. While calculating the rectangular boards gave us the data points needed to observe convergence, we ultimately only really care about the 10x10 square board without a border. Therefore, if we look along the diagonal of the last matrix (where rows equal columns), we see the second-order growth factor, slowly increasing as it converges. The most recent value is 3.54. Based on the current trend, we can anticipate that this value will likely continue to increase slightly, but for the sake of this exercise we can commit to 3.54 as our final number of positions per block. Since 3.54 is likely a slight underestimate, our final calculation is conservative.

## Estimated Starting Positions

3.54^100
way more than fischer random 960
less than shannon number
but still pretty good and about equal to catan



## Permutations

Here are the results so far:

| Board Size | Permutations with Border | Permutations no Border |
| --- | --- | --- |
| 1x1 | 1 | 1 |
| 2x2 | 23 | 23 |
| 3x3 | 20,389 | 22,050 |
| 4x4 | 207,016,289 | 283,415,151 |
| 10x10 | ? | ? |

If these numbers seem too big for boards this small good! I'm skeptical myself and encourage people to check the codes used for these calculations in this repository!

Another script extrapolates the current results to estimate the 10x10 board will have on the order of 10^24 permutations. Unfortunately that's tracking to be less than the Shannon number, [but is on par with the estimated number of setup permutations of Catan](https://www.reddit.com/r/Catan/comments/9uh2be/how_many_different_map_combinations_on_a_standard/). Regardless, the real objective is a board game where you will likely never play the same starting conditions twice, thus reducing the value of memorizing standard opening practices.

![](https://github.com/darkfireXXI/Board_game/blob/images/extrapolation.jpg)

### Video Game Version
Seeing as I didn't have access to a wood shop, but my computer runs python I began coding the game. Creating a matrix for the board and then using it to make a 3D plot was very straightforward, but randomizing the board was a fun challenge. Furthermore, I didn't want it to be absolutely random, but random within a user defined window. The best way to understand this is to look at the terminal interface to get a sense of the game start up:
![](https://github.com/darkfireXXI/Board_game/blob/images/terminal_interface.png)  
Users can select if they want to play on mountains or valleys and from within this selection adjust the extremity of the terrain. **Yet**, even with this level of specification I still **guarantee** that if you pick Medium Mountains 1000 times in a row you will **never** get the same board twice! I encourage you to run game.py and see for yourself!  

Now let's take a look at what this would look like:  
Mountains:  
![](https://github.com/darkfireXXI/Board_game/blob/images/mountain.png)  
Valley:  
![](https://github.com/darkfireXXI/Board_game/blob/images/valley.png)  
There's even a Special Terrain called Mixed Mountains & Valleys that generates landscapes like this:  
![](https://github.com/darkfireXXI/Board_game/blob/images/mixed.png)  
And no, that doesn't violate board physics.  
There are some other things going on behind the scenes here that were interesting to code. When plotting the board height differences of 2 will leave unsightly gaps in the render. The easy fix is to simply plot everything a second time under itself to close the gaps. This is not only lazy, but also slows the graphics as then it has to keep track of 200 cubes opposed to a 100 plus some space fillers. A better method can be viewed in the plot_board function in game.py. Another fun feature was getting it color code the cubes according to height.
### Next Steps
I began coding the gameplay, but stopped after realizing that I had never actually played this game in any context (eg. on paper using marker caps for figures), so hard coding rules that are highly subject to change as I get user feedback was not on my agenda. Once I've gotten more user feedback from mock games I will continue to code out the game play. In the meantime I've been considering switching from matplotlib plotting to creating my own GUI.
