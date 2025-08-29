# Board Game

A novel board game that addresses issues prominent in many games, but particularly classical games, such as chess and go.

## Origin Story
Something about [chess](https://en.wikipedia.org/wiki/Chess) never felt quite right to me. It's perhaps the most popular board game to ever exist. It's tauted as intellectual sparring and revered for all the possibilities and complex gameplay. However, it greatly rewards memorization and studying. It's impossible to get far without learning the openning theory and new players are "punished" with opening traps. Particularly in the modern computer era it's all the more accessible to study various computer lines out 40 moves and curbstomp online players simply because one has memorized all the variations of a specific move sequence.

Chess (and [go](https://en.wikipedia.org/wiki/Go_(game)) and other games - although I'll be primarily targeting the first two) has these pitfalls for two major reasons:

1. The initial gameplay conditions/setup are always the same
2. Despite numerous gameplay possibilities, most options are bad (ie. non-condusive to winning)

The more in depth explanation is that with same starting conditions, all possible ways a game can begin can be calculated ahead of time and players can figure out the best strategy to win before even starting a game. After each player makes their first move in chess there are 400 possible boards. From there the numbers only grow exponentially. However, these are 400 knowable outcomes that have been relentlessly studied and analyzed, so much so that now only a few dozen of these options are considered viable gameplay. [As Bobby Fischer said "e4 is best by test"](https://en.wikipedia.org/wiki/Bobby_Fischer#Opening_theory). This brings me to my second point, the [Shannon Number](https://en.wikipedia.org/wiki/Shannon_number) (10^120) is widely considered a lower bound of the game-tree complexity. That's a very large number, but if you're playing optimally there are actually very sparse branches of the game tree worth pursuing. This means that the first several moves of any competitive chess game are one of a handful of traditional move sequences. This makes gameplay more dull, rewards knowledge of textbook openings, and reduces the size of the game-tree worth exploring.

Go, [shogi](https://en.wikipedia.org/wiki/Shogi), etc. pretty much any popular classic game are guilty of the same charges. [Catan](https://en.wikipedia.org/wiki/Catan) and [Stratego](https://en.wikipedia.org/wiki/Stratego) would be good examples of games where starting conditions are either randomized or player selected and as a result the game-tree is not only wider, but more branches are worth exploring as viable paths to victory.

These opinions are neither novel nor niche. [Bobby Fischer invented Fischer Random Chess](https://en.wikipedia.org/wiki/Chess960) to **reduce the emphasis on opening preparation and to encourage creativity in play** as the pieces on the back rank are shuffled . Magnus Carlsen is also now pushing Fischer Random with this brilliant headline [_"The Greatest Chess Player of All Time Is Bored With Chess"_](https://www.wsj.com/sports/chess-magnus-carlsen-fabiano-caruano-world-championship-e54c9fc5?gaa_at=eafs&gaa_n=ASWzDAhBJMl5WeSlB6BQ-xF4bRpuQm4KRYaY1hb27NMAVGvlmMJ3ooZ50Xca-NE61OU%3D&gaa_ts=68b13711&gaa_sig=-iAKOX_tvDHWRN8fX-ndMTiVfoOdej_VRq1LaTJ2QxQnRbmfFpQzbHp32I1Ynu9clemayx7M-xUOEu-3sfCk0w%3D%3D).

What does it say about a game when the best players of all time think it's stale?

## Motivation: Crush the Shannon Number

The Shannon Number is how many possible games of chess could exist. I want a board game where there are more setup possibilites than possible games of chess. If you then consider that for each game's initial conditions, one could play millions of different games, then multiplied by all the various setups would yield near infinite gameplay possibilies. The game-tree would dwarf that of go. Furthermore, having so many different setups would greatly diminish the value of opening theory. There would be too many possibilities to study/memorize and the game would lend itslef to conceptual knowledge and true strategy over edging out oppenents by knowing esoteric move sequences.

## Concept: Board Games are Flat, Real Life is 3D

It's in the name, board game, board, boards are flat. Real life is 3D. Why isn't there a game where players can ascend moutains and dive into valleys? [Warhammer](https://en.wikipedia.org/wiki/Warhammer_(game)) has great environments, but they're fixed, not to mention it's not the most accessible game. My goal is something as simple as chess in terms of learning and setup, but the setup can be easily changed to simulate a wide variety of terrains.

## Woodshop vs Laptop

The original board game idea probably came to me around 2015. Since then it's mostly been a random thought in my head that pops up about twice a year. I mull it over and then let it go. All this time though it had strictly been a physical wooden contraption that would bring this idea to life. This is important as it meant I came up with the game physics based on what I thought I could cut, drill, grind, etc. to achieve a physical object that could move the way I wanted. Unfortunately, laptops are much easier to come by than woodshops and with my newfound coding skills I thought why not give it a shot. Both coding up the game logic and counting the setup permutations to see if it's >10^120. 

## Board Physics
The board game is 10 by 10 square grid. Each square can move a maximum of 2 units relative to it's neighbors (up, down, left, right - **not** diagonals). The unit of height could be marked by a notch on the individual blocks representing that make up the gameplay squares. If I've already lost you just scroll down to the [Wood Version](###Wood-Version) section for a visual aid.   


A wooden version would need to be enclosed by a thin wall to contain all the blocks. Now that I've 3D printed this board with a different interlocking mechanism, or for a virtual version, the border is not necessary. This leads to different numbers of permutations depending on whether or not there is a border.

Set atop a flat surface all squares would be flush, thus having a height of 0. The border holding the pieces together is a boardary set at height = 0. Therefore, it follows that the outermost squares (the 10x10 ring) can be at a maximum height = 2 or minimum height = -2. Thus, any edge piece can be have a heigh in the set of [-2, -1, 0, 1, 2] providing the neighbor permits it. This condition is that each square can only have at most 2 units of height difference between itself and any one of it's 4 adjacent neighbors.  

Now it might help to start visualizing a birds eye view of the board with each sqace displaying a number representing it's height. So a 0 height/flat board in an array form would look like:  

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
abs(board[x, y] - board[x-1, y]) <= 2
abs(board[x, y] - board[x+1, y]) <= 2
abs(board[x, y] - board[x, y-1]) <= 2
abs(board[x, y] - board[x, y+1]) <= 2
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

I implore you to check that none of our in game physics are broken. Take a minute to think about it because what comes next might be a surprise. Let's say you have the 0/flat board on a table. All squares have height = 0. Now you pick it up and hold it out over the air... what happens?  

You get the playing field known as the Mariana Trench (left) because with nothing to support the block each square will fall it's maximum 2 units from it's neighbors (mechanism to physically allow this is a mechanical engineering problem for another time). At the other extreme you have the field known as Mt. Everest (right):

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

On a borderless board this is the most extreme playing field possible. Although the borderless version includes the Mariana Trench and Mount Everest as well.

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

Now that we've established the board physics we want to figure out how many different permutations there are between the two extremities shown above? I've also added code for counting boards the don't have a border.

Before jumping into the code, let's start small. A 1x1 board; one square with values -2 <= height <= 2. We can increment through the heights and rotate it how we like, but in the end there's only one unique combination. Next we can take a 2x2 board. We start at the lower extremity with each square's height value = -2. With two nested for loops with can increment each square on the board once. How many times do we have to do this?  

<pre>
(4 squares * 2 height) - (4 square * -2 height) = 16 total moves from the lowest board to the highest
</pre>

We record the starting board as a unqiue board result. Then we increment one square one unit and check to see if this new board is already recorded among the unique board results. The verification checks include 90° rotations and checking height offsets. A board with all values at 0 is the same as a board with all values at 2. If the new board doesn't match any of the unique board results we add it to the list.

This would be be the first of 4 squares to check in the 2x2 example. We would then return to our starting board and try moving the additional 3 squares and repeating the checks, adding new unique boards if found.

Now those 16 moves calculated above should really be thought of as 16 rounds of moves, in each rounding moving one square one unit up, checking to see if it's unique, going back to the start board, and repeating. In the subsewquent round, the starting board is no longer the board of heights all -2. We now have several starting boards, which would be the unique board results found in the previous round.

We repeat this until all the rounds and permutations are exhausted.

For perspective with the full size 10x10 board...

<pre>
sum(Mt. Everest board) - sum(Mariana Trench Board) = 880 total rounds of moves from the lowest board to the highest
</pre>

And for each final board result we can increment it 100 ways.
Yeah, so that's not going to be solved anytime soon on a laptop. You can solve the 2x2 example on a laptop, 3x3 refactored with almost a decade later I was also able to now compute on a laptop instead of AWS. 4x4 and higher I'm currently computing on a desktop with yet another refactor from python to c++.

### 2x2 Permutation Tree

The tree for a 2x2 board is shown below and confirms the number of combinations found by the in the code. Making any additional moves from the leaf nodes would either break the board's physics or create a repeat of a previous combination (eg. rotation or board height values are all +n height more than a previous board). Hopefully this illustrates that even for a small board the number of combinations quickly gets out of hand. As the boards become bigger we not only have more squares, but also the range of height gets larger. It actually scales with the size of the board: 2x2 ranges from -2 to +2 in height while 10x10 can go from -10 to +10 assuming a border. A borderless 10x10 board can range from 0 to 36.

![](https://github.com/darkfireXXI/Board_game/blob/images/tree.jpg)

## Permutations

Here are the results so far:

| Board Size | Permutations with Border | Permutations no Border |
| --- | --- | --- |
| 1x1 | 1 | 1 |
| 2x2 | 23 | 23 |
| 3x3 | 20,389 | 22,050 |
| 4x4 | 207,016,289 | >207,016,289 |
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
