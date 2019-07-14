# Board Game
For quite some time now I've been wanting to create a board game. The goal was to create something similar to chess, where the game dynamics were primarily skill based opposed to luck based. The key aspect I wanted to incorporate into the game was a dynamic environment. This way even if the game pieces were the same every round, the board (well it's really a 3D surface) could be reset. The board had to be able to shift around in ways that would yield a high number of permutations° yet still give a somewhat standardized surface. This is important because certain pieces would have different movement abilities (eg. climbing mountains vs descending valleys). It was important that no matter how randomized the playing surface becomes it would never require hyper specialized movement abilities or completely hinder other pieces from accessing part of the map.  

° I want the board alone to have more permutations than possible games of chess (10^120).

## Woodshop vs Laptop
The original idea probably came to me about 10 years ago. Since then it's mostly been a random thought in my head that pops up about twice a year. I mull it over and then let it go. All this time though it had strictly been a physical wooden contraption that would bring this idea to life. This is important as it meant I came up with the game physics based on what I thought I could cut, drill, grind, etc. to achieve a physical object that could move the way I wanted. Unfortunately, laptops are much easier to come by than woodshops and with my newfound coding skills I thought why not give it a shot.  

## Board Physics
The board is 10 by 10 enclosed by a thin wall, not in the video game but in the physcial version. This is important to keep in mind because it sets a boundary condition. Each square by definition can move a maximum of 2 units relative to it's neighbors (up, down, left, right - **not** diagonals). The unit of height could be marked by a notch on a sliding block representing the individual square. If I've already lost you just scroll down to the Wood Version section for a visual aid.  

Set atop a flat surface all squares would be flush, thus having a height of 0. The wooden wall holding the pieces together is a boardary set at height = 0. Therefore, it follows that the outermost squares (the 10x10 ring) can be at a maximum height = 2 or minimum height = -2. Thus, any edge piece can be have a heigh in the set of [-2, -1, 0, 1, 2] providing the neighbor permits it. This condition is that each square can only have at most 2 units of height difference between itself and any one of it's 4 adjacent neighbors.  

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
Now with these constraints and our boundary condition we would like to randomize the board to get something that looks like this:
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
 
### Wood Version
Turns out I have CAD drawings (January 2016) for the wood version and a rendering:  
![](https://github.com/darkfireXXI/Board_game/blob/images/Render1.png)

## Board Counting
Now that we've established the board physics we want to figure out how many different permutations there are between the two extremities shown above?  

This was the first part of the project I coded. Before jumping into the code, let's start small. A 1x1 board; one square with values -2 <= height <= 2. We can increment throught the combinations and rotate it how we like, but in the end there's only one unique combination. Next we can take a 2x2 board. We start at the lower extremity with each square's height value = -2. With two nested for loops with can increment each square on the board once. How many times do we have to do this?  
<pre>
(4 squares * 2 height) - (4 square * -2 height) = 16 total moves from the lowest board to the highest
</pre>
This is not totally accurate though because it assumes we increment one at a time directly moving the square up until we hit the maximum. In reality we record the starting board as a unqiue final board. Then we increment one square one unit and check to see if this new board is already recorded among the unique final boards. These verification checks include 90° rotations and height offsets <= 4 as the largest height difference yielding a duplicate board would be 2--2 = 4. If the new board doesn't match any of the unique final boards we add it to the list.  

This would be be the first of 4 squares to check in the 2x2 example. The 16 moves calculated above should really be thought of as 16 rounds of moves, in each rounding moving one square one unit up, checking to see if it's unique, going back to the start board, and repeating. So we would then return to our starting board and trying moving the additional 3 squares. However, you can easily imagine the remaining 3 squares are cornes and give rotations of the first one. Thus we conclude the first round of 16. In the second round we use the new boards found in round one (there's only 1 in this case) and repeat the process (one at a time incrementing the four squares) to generate new boards. This time we generate 3 new boards and now have 5 unique final boards. Now for the third round each of these 3 new boards goes through the process again. Now one can start to see how even with small board many variations arise. Now let's take a look at the 10x10 analogy:
<pre>
sum(Mt. Everest board) - sum(Mariana Trench Board) = 880 total rounds of moves from the lowest board to the highest
</pre>
Yeah, so that's not going to be solved anytime soon on a laptop. You can solve the 2x2 example on a laptop, but then for the 3x3 I had to put it on AWS. The 4x4 I started solving, but have yet to complete.  

Here are the results so far:  
1x1 - 1 combination  
2x2 - 23 combinations  
3x3 - 20389 combinations  
4x4 - >71000 combinations (round 11/48)  

If these numbers seem too big for boards this small good! I'm skeptical myself and encourage people to check the codes used for these calculations in this repository. The brute force methodology is backed up in logic and complexity if one considers the problem imagining the unique boards as leaves/child nodes of a tree with the lowest board as the root. The difference bewteen the sum of the heights of a parent board and the sum of the heights of a child board would always be exactly one. The tree for a 2x2 board is shown below and confirms the number of combinations found by the 2x2BG.py code. Making any additional moves from the leaf nodes would either break the board's physics or create a repeat of a previous combination (eg. rotation or board height values are all +1 to +4 more than a previous board). Hopefully this illustrates that even for a small board the number of combinations quickly gets out of hand and as the boards become bigger we not only have more squares, but also the range of height gets larger. It actually scales with the size of the board: 2x2 ranges from -2 to +2 in height while 10x10 can go from -10 to +10.  
![](https://github.com/darkfireXXI/Board_game/blob/images/tree.jpg)
### Video Game Version
Seeing as I didn't have access to a wood shop, but my computer runs Python I began coding the game. Creating a matrix for the board and then using it to make a 3D plot was very straightforward, but randomizing the board was a fun challenge. Furthermore, I didn't want it to be absolutely random, but random within a user defined window. The best way to understand this is to look at the terminal interface to get a sense of the game start up:
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
