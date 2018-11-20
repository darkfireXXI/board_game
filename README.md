# Board Game
For quite some time now I've been wanting to create a board game. The goal was to create something similar to chess, where the game dynamics were primarily skill based opposed to luck based. The key aspect I wanted to incorporate into the game was a dynamic environment. This way even if the game pieces were the same every round, the board (well it's really a 3D surface) could be reset. The board had to be able to shift around in ways that would yield a high number of permutations° yet still give a somewhat standardized surface. This is important because certain pieces would have different movement abilities (eg. climbing mountains vs descending valleys). It was important that no matter how randomized the playing surface becomes it would never require hyper specialized movement abilties or completely hinder other pieces from accessing part of the map.  

° I want the board alone to have more permutations than possible games of chess (10^120).

## Woodshop vs Laptop
The original idea probably came to me about 10 years ago. Since then it's mostly been a random thought in my head that pops up about twice a year. I mull it over and then let it go. All this time though it had strictly been a physical wooden contraption that would bring this idea to life. This is important as it meant I came up with the game physics based on what I thought I could cut, drill, grind, etc. to achieve a physical object that could move the way I wanted. Unfortunately, laptops are much easier to come by than woodshops and with my newfound coding skills I thought why not give it a shot.  

## Board Physics
The board is 10 by 10 enclosed by a thin wall, not in the video game but in the physcial version. This is important to keep in mind because it sets a boundary condition. Each square by definition can move a maximum of 2 units relative to it's neighbors (up, down, left, right - NOT diagonals). The unit of height could be marked by a notch on a sliding block representing the individual square. If I've already lost you just scroll down to the Wood Version section for a visual aid.  

Set atop a flat surface all squares would be flush, thus having a height of 0. The wooden wall holding the pieces together is a boardary set at height = 0. Therefore, it follows that the outermost squares (the 10x10 ring) can be at a maximum height = 2 or minimum height = -2. Thus, any edge piece can be have a heigh in the set of [-2, -1, 0, 1, 2] providing the neighbor permits it. This condition is that each square can only have at most 2 units of height difference between itself and any one of it's 4 adjacent neighbors.  

Now it might help to start visualizing a birds eye view of the board with each sqace displaying a umber representing it's height. So a 0 height/flat board in an array form would look like:  
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
 If we imagine the board like this with x and y as indices the neighbor condition can be writting as:
 <pre>
board[x, y] - board[x-1, y] <= 2
board[x, y] - board[x+1, y] <= 2
board[x, y] - board[x, y-1] <= 2
board[x, y] - board[x, y+1] <= 2
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
I implore you to check that none of our in game physics are not broken. Take a minute to think about it because what comes next might be a surprise. Let's say you have the 0/flat board on a table. All squares have height = 0. Now you pick it up and hold it out over the air... what happens?  

You get the playing field known as the Mariana Trench (left). At the other extreme you have the field known as Mt. Everest (right):
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
![]()

## Board Counting
Now that we've established the board physics we want to figure out how to different permutations there are between the two extremities shown above?  

This was the first part of the project I coded. 
