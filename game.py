import sys
import time
import random as rand
import numpy as np
import matplotlib
matplotlib.use('QT5Agg') # WXAgg, GTKAgg, QT4Agg, QT5Agg, TkAgg
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

board = np.zeros((12, 12))
sboard = board
boardMin = np.zeros((12, 12))
boardMax = np.zeros((12, 12))

for i in range(1,6):
		boardMin[i:12-i, i:12-i] = boardMin[i:12-i, i:12-i] - 2
		boardMax[i:12-i, i:12-i] = boardMax[i:12-i, i:12-i] + 2

def gen_board(board, push, a1_1, a10_10, landscape_factor):
	newboard = board
	moves = 0

	while(moves < push):
		up_down = np.random.random()
		i = np.random.randint(a1_1, a10_10)
		j = np.random.randint(a1_1, a10_10)

		# if(board[i, j] == 0): #
		if(up_down >= landscape_factor):
			newboard[i, j] += 1
		else:
			newboard[i, j] -= 1
		# else:
		# 	if(board[i, j] > 0):
		# 		up_down = 1
		# 		newboard[i, j] += 1
		# 	else:
		# 		up_down = 0
		# 		newboard[i, j] -= 1

		if((abs(newboard[i, j] - newboard[i-1, j]) > 2) or (abs(newboard[i, j] - newboard[i, j-1]) > 2) or 
			(abs(newboard[i, j] - newboard[i+1, j]) > 2) or (abs(newboard[i, j] - newboard[i, j+1]) > 2) or 
			(newboard[i, j] > boardMax[i, j]) or (newboard[i, j] < boardMin[i, j])):
			
			if(up_down >= landscape_factor):
				newboard[i, j] -= 1
			else:
				newboard[i, j] += 1

			continue

		else:
			moves += 1
			# print(moves)

	# ALTERNATE METHOD THAT IS TOO SLOW/LUCK BASED

	# N10 = np.random.randint(-2, 2, 10)
	# S10 = np.random.randint(-2, 2, 10)

	# E8 = np.random.randint(-2, 2, 8)
	# W8 = np.random.randint(-2, 2, 8)

	# N8 = np.random.randint(-4, 4, 8)
	# S8 = np.random.randint(-4, 4, 8)

	# E6 = np.random.randint(-4, 4, 6)
	# W6 = np.random.randint(-4, 4, 6)

	# N6 = np.random.randint(-6, 6, 6)
	# S6 = np.random.randint(-6, 6, 6)

	# E4 = np.random.randint(-6, 6, 4)
	# W4 = np.random.randint(-6, 6, 4)

	# N4 = np.random.randint(-8, 8, 4)
	# S4 = np.random.randint(-8, 8, 4)

	# E2 = np.random.randint(-8, 8, 2)
	# W2 = np.random.randint(-8, 8, 2)

	# center = np.random.randint(-10, 10, [2, 2])

	# board[1, 1:11] = N10
	# board[10, 1:11] = S10

	# board[2:10, 10] = E8
	# board[2:10, 1] = W8

	# board[2, 2:10] = N8
	# board[9, 2:10] = S8

	# board[3:9, 9] = E6
	# board[3:9, 2] = W6

	# board[3, 3:9] = N6
	# board[8, 3:9] = S6

	# board[4:8, 8] = E4
	# board[4:8, 3] = W4

	# board[4, 4:8] = N4
	# board[7, 4:8] = S4

	# board[5:7, 7] = E2
	# board[5:7, 4] = W2

	# board[5:7, 5:7] = center
	
	# print(board)
	return newboard

def gen_mix_board(board, push, a1_1, a10_10, landscape_factor):
	newboard = board
	moves = 0

	mix_board = np.zeros((12, 12))
	x_y = np.random.random()
	if(x_y >= 0.5):
		for i in range(1,11):
			for j in range(1,11):
				if(j > i):
					mix_board[i, j] = True
				elif(i == j):
					x_y = np.random.random()
					if(x_y >= 0.5):
						mix_board[i, j] = True
				else:
					mix_board[i, j] = False

		if(x_y >= 0.5):
			mix_board = np.rot90(mix_board)
			mix_board = np.rot90(mix_board)

	else:
		# push = 235
		for i in range(1,11):
			for j in range(1,7):
				if(j < 5):
					mix_board[i, j] = True
				else:
					x_y = np.random.random()
					if(x_y >= 0.5):
						mix_board[i, j] = True
					
		if(x_y >= 0.5):
			mix_board = np.rot90(mix_board)
			mix_board = np.rot90(mix_board)

	# print(mix_board)
	strikes = 0
	while(moves < push):
		up_down = np.random.random()
		i = np.random.randint(a1_1, a10_10)
		j = np.random.randint(a1_1, a10_10)

		if(mix_board[i, j] == True):
			up_down = 1
			newboard[i, j] += 1
		else:
			up_down = 0
			newboard[i, j] -= 1

		if((abs(newboard[i, j] - newboard[i-1, j]) > 2) or (abs(newboard[i, j] - newboard[i, j-1]) > 2) or 
			(abs(newboard[i, j] - newboard[i+1, j]) > 2) or (abs(newboard[i, j] - newboard[i, j+1]) > 2) or 
			(newboard[i, j] > boardMax[i, j]) or (newboard[i, j] < boardMin[i, j])):

			if(up_down >= landscape_factor):
				newboard[i, j] -= 1
			else:
				newboard[i, j] += 1

			strikes += 1
			if(strikes >= 1e5):
				break

		else:
			moves += 1
			strikes = 0
			# print(moves)

	# print(newboard)
	return newboard

def check_board(board):
	status = True
	for i in range(1, 11):
		for j in range(1, 11):
			if((abs(board[i, j] - board[i-1, j]) > 2) or (abs(board[i, j] - board[i, j-1]) > 2) or 
				(abs(board[i, j] - board[i+1, j]) > 2) or (abs(board[i, j] - board[i, j+1]) > 2) or 
				(board[i, j] > boardMax[i, j]) or (board[i, j] < boardMin[i, j])):
				status = False
				return status

	return status
	# if(status == True):
	# 	return board
	# else:
	# 	board = gen_board()
	# 	board = check(board)

def board_scramble(board, landscape_factor):
	board = gen_board(board, 150, 1, 11, landscape_factor)
	board = gen_board(board, 150, 2, 10, landscape_factor)
	board = gen_board(board, 150, 3, 9, landscape_factor)
	board = gen_board(board, 150, 4, 8, landscape_factor)
	# print('cat')
	return board

def cube(x_range, y_range, z_range, cube_color):
	normal = [0, 0, 1]
	xx, yy = np.meshgrid(x_range, y_range)
	zz = (normal[0]*xx + normal[1]*yy + z_range[0]) / normal[2]
	ax.plot_wireframe(xx, yy, zz, color = 'black', alpha=0.5)
	ax.plot_surface(xx, yy, zz, color = cube_color)
	zz = (normal[0]*xx + normal[1]*yy + z_range[1]) / normal[2]
	ax.plot_wireframe(xx, yy, zz, color = 'black', alpha=0.5)
	ax.plot_surface(xx, yy, zz, color = cube_color)

	normal = [0, 1, 0]
	xx, zz = np.meshgrid(x_range, z_range)
	yy = (normal[0]*xx + normal[2]*zz + y_range[0]) / normal[1]
	ax.plot_wireframe(xx, y_range[0], zz, color = 'black', alpha=0.5)
	ax.plot_surface(xx, y_range[0], zz, color = cube_color)
	yy = (normal[0]*xx + normal[2]*zz + y_range[1]) / normal[1]
	ax.plot_wireframe(xx, y_range[1], zz, color = 'black', alpha=0.5)
	ax.plot_surface(xx, y_range[1], zz, color = cube_color)

	normal = [1, 0, 0]
	yy, zz = np.meshgrid(y_range, z_range)
	xx = (normal[1]*yy + normal[2]*zz + x_range[0]) / normal[0]
	ax.plot_wireframe(xx, yy, zz, color = 'black', alpha=0.5)
	ax.plot_surface(xx, yy, zz, color = cube_color)
	xx = (normal[1]*yy + normal[2]*zz + x_range[1]) / normal[0]
	ax.plot_wireframe(xx, yy, zz, color = 'black', alpha=0.5)
	ax.plot_surface(xx, yy, zz, color = cube_color)

def fast_cube(x_range, y_range, z_range, cube_color):
	cube =  (x_range[0] <= xx)&(xx < x_range[1]) & (y_range[0] <= yy)&(yy < y_range[1]) & (z_range[0] <= zz)&(zz < z_range[1])
	ax.voxels(cube, facecolor = cube_color, edgecolor = 'black', linewidth = 0.1)

def plot_board(board, color_stick):
	for i in range(1,11):
		for j in range(1, 11):
			cube_color = color_stick[int(board[i, j] + 10)]
			fast_cube([i-1, i], [j-1, j], [board[i, j]-1 + 11, board[i, j] + 11], cube_color)

			# LAZY WAY TO FILL GAPS
			# gap_color = color_stick[int(fill_gaps[i, j] + 10)]
			# fast_cube([i-1, i], [j-1, j], [fill_gaps[i, j]-1 + 10, fill_gaps[i, j] + 10], gap_color)

	for i in range(1,11):
		for j in range(1, 11):
			if(abs(board[i, j] - board[i-1, j]) > 1):
				if(board[i, j] > board[i-1, j]):
					cube_color = color_stick[int(board[i, j] + 9)]
					fast_cube([i-1, i], [j-1, j], [board[i, j]-2 + 11, board[i, j]-1 + 11], cube_color)
				else:
					cube_color = color_stick[int(board[i-1, j] + 9)]
					fast_cube([i-2, i-1], [j-1, j], [board[i-1, j]-2 + 11, board[i-1, j]-1 + 11], cube_color)
			
			if(abs(board[i, j] - board[i, j-1]) > 1):
				if(board[i, j] > board[i, j-1]):
					cube_color = color_stick[int(board[i, j] + 9)]
					fast_cube([i-1, i], [j-1, j], [board[i, j]-2 + 11, board[i, j]-1 + 11], cube_color)
				else:
					cube_color = color_stick[int(board[i, j-1] + 9)]
					fast_cube([i-1, i], [j-2, j-1], [board[i, j-1]-2 + 11, board[i, j-1]-1 + 11], cube_color)

			if(abs(board[i, j] - board[i+1, j]) > 1):
				if(board[i, j] > board[i+1, j]):
					cube_color = color_stick[int(board[i, j] + 9)]
					fast_cube([i-1, i], [j-1, j], [board[i, j]-2 + 11, board[i, j]-1 + 11], cube_color)
				else:
					cube_color = color_stick[int(board[i+1, j] + 9)]
					fast_cube([i, i+1], [j-1, j], [board[i+1, j]-2 + 11, board[i+1, j]-1 + 11], cube_color)

			if(abs(board[i, j] - board[i, j+1]) > 1):
				if(board[i, j] > board[i, j+1]):
					cube_color = color_stick[int(board[i, j] + 9)]
					fast_cube([i-1, i], [j-1, j], [board[i, j]-2 + 11, board[i, j]-1 + 11], cube_color)
				else:
					cube_color = color_stick[int(board[i, j+1] + 9)]
					fast_cube([i-1, i], [j, j+1], [board[i, j+1]-2 + 11, board[i, j+1]-1 + 11], cube_color)

def choose_landscape():
	print('\nPlease select the kind of landscape you would like to play on.\nYou can do this by typing in the number or description of the landscape you desire and then hitting Enter/Return.\n')
	print('1. Mountains\n2. Woodland\n3. Valleys\n4. Special Terrains\n')

	gen_landscape = False
	while(gen_landscape == False):
		gen_landscape = input()
		gen_landscape = gen_landscape.lower()
		if(gen_landscape in ['1', 'one', 'mountain', 'mountains']):
			gen_landscape = 1
		elif(gen_landscape in ['2', 'two', 'woodland', 'woodlands']):
			gen_landscape = 2
		elif(gen_landscape in ['3', 'three', 'valley', 'valleys']):
			gen_landscape = 3
		elif(gen_landscape in ['4', 'four', 'special terrain', 'special terrains', 'special', 'terrain', 'terrains']):
			gen_landscape = 4
		else:
			print('\nNot a valid selection. Please select the kind of landscape you would like to play on:\n')
			gen_landscape = False

	spc_landscape = False
	if(gen_landscape == 1):
		print('\nPlease adjust the extremity of the terrain (Mountains):\n')
		print('1. High\n2. Medium\n3. Low\n\n4. Go Back\n')
		while(spc_landscape == False):
			spc_landscape = input()
			spc_landscape = spc_landscape.lower()
			if(spc_landscape in ['1', 'one', 'high']):
				spc_landscape = 1
			elif(spc_landscape in ['2', 'two', 'medium', 'mid']):
				spc_landscape = 2
			elif(spc_landscape in ['3', 'three', 'low']):
				spc_landscape = 3
			elif(spc_landscape in ['4', 'four','go back', 'back', 'go', 'b', 'gb']):
				spc_landscape = False
				return spc_landscape
			else:
				print('\nNot a valid selection. Please select the extremity of the landscape you would like to play on (Mountains):\n')
				spc_landscape = False

	elif(gen_landscape == 2):
		print('\nPlease adjust the extremity of the terrain (Woodland):\n')
		print('1. High\n2. Medium\n3. Low\n\n4. Go Back\n')
		while(spc_landscape == False):
			spc_landscape = input()
			spc_landscape = spc_landscape.lower()
			if(spc_landscape in ['1', 'one', 'high']):
				spc_landscape = 4
			elif(spc_landscape in ['2', 'two', 'medium', 'mid']):
				spc_landscape = 5
			elif(spc_landscape in ['3', 'three', 'low']):
				spc_landscape = 6
			elif(spc_landscape in ['4', 'four', 'go back', 'back', 'go', 'b', 'gb']):
				spc_landscape = False
				return spc_landscape
			else:
				print('\nNot a valid selection. Please select the extremity of the landscape you would like to play on (Woodland):\n')
				spc_landscape = False

	elif(gen_landscape == 3):
		print('\nPlease adjust the extremity of the terrain (Valleys):\n')
		print('1. High\n2. Medium\n3. Low\n\n4. Go Back\n')
		while(spc_landscape == False):
			spc_landscape = input()
			spc_landscape = spc_landscape.lower()
			if(spc_landscape in ['1', 'one', 'high']):
				spc_landscape = 7
			elif(spc_landscape in ['2', 'two', 'medium', 'mid']):
				spc_landscape = 8
			elif(spc_landscape in ['3', 'three', 'low']):
				spc_landscape = 9
			elif(spc_landscape in ['4', 'four', 'go back', 'back', 'go', 'b', 'gb']):
				spc_landscape = False
				return spc_landscape
			else:
				print('\nNot a valid selection. Please select the extremity of the landscape you would like to play on (Valleys):\n')
				spc_landscape = False

	elif(gen_landscape == 4):
		print('\nPlease select which one of Speacial Terrain you would like to play on:\n')
		print('1. Mount Everest\n2. Gasherbrum I\n3. Mixed Mountains & Valleys\n4. Kuril–Kamchatka Trench\n5. Mariana Trench\n\n6. Go Back\n')
		while(spc_landscape == False):
			spc_landscape = input()
			spc_landscape = spc_landscape.lower()
			if(spc_landscape in ['1', 'one', 'mount everest', 'everest']):
				spc_landscape = 10
			elif(spc_landscape in ['2', 'two', 'gasherbrum i', 'gasherbrum', 'gash']):
				spc_landscape = 11
			elif(spc_landscape in ['3', 'three', 'mix', 'mixed', 'mixed mountains & valleys']):
				spc_landscape = 12
			elif(spc_landscape in ['4', 'four', 'kuril–kamchatka', 'kuril', 'kamchatka']):
				spc_landscape = 13
			elif(spc_landscape in ['5', 'five', 'mariana trench', 'mariana']):
				spc_landscape = 14
			elif(spc_landscape in ['6', 'six', 'go back', 'back', 'go', 'b', 'gb']):
				spc_landscape = False
				return spc_landscape
			else:
				print('\nNot a valid selection. Please select one of Speacial Terrain you would like to play on:\n')
				spc_landscape = False

	return spc_landscape

def soldier(r_cord, c_cord, team, board):
	if(team == True):
		team_color = 'black'
	else:
		team_color = 'red'
	z_range = [board[r_cord, c_cord] + 11, board[r_cord, c_cord] + 0.5 + 11]

	x = np.array([r_cord - 0.9, r_cord - 0.9, r_cord - 0.1, r_cord - 0.1])
	y = np.array([c_cord - 0.55, c_cord - 0.45, c_cord - 0.45, c_cord - 0.55])
	z = np.array([z_range[1], z_range[1], z_range[1], z_range[1]])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.4, r_cord - 0.4, r_cord - 0.3, r_cord - 0.3])
	y = np.array([c_cord - 0.75, c_cord - 0.25, c_cord - 0.25, c_cord - 0.75])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.9, r_cord - 0.1, r_cord - 0.1, r_cord - 0.9])
	y = np.array([c_cord - 0.55, c_cord - 0.55, c_cord - 0.55, c_cord - 0.55])
	z = np.array([z_range[1], z_range[1], z_range[0], z_range[0]])
	
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	y = y + 0.1
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.4, r_cord - 0.3, r_cord - 0.3, r_cord - 0.4])
	y = np.array([c_cord - 0.75, c_cord - 0.75, c_cord - 0.75, c_cord - 0.75])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	y = y + 0.5
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.4, r_cord - 0.4, r_cord - 0.4, r_cord - 0.4])
	y = np.array([c_cord - 0.75, c_cord - 0.25, c_cord - 0.25, c_cord - 0.75])
	
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	x = x + 0.1
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.9, r_cord - 0.9, r_cord - 0.9, r_cord - 0.9])
	y = np.array([c_cord - 0.55, c_cord - 0.45, c_cord - 0.45, c_cord - 0.55])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	x = x + 0.8
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

def guard(r_cord, c_cord, team, board):
	if(team == True):
		team_color = 'black'
	else:
		team_color = 'red'
	z_range = [board[r_cord, c_cord] + 11, board[r_cord, c_cord] + 0.5 + 11]

	x = np.array([r_cord - 0.875, r_cord - 0.875, r_cord - 0.375, r_cord - 0.125, r_cord - 0.375])
	y = np.array([c_cord - 0.75, c_cord - 0.25, c_cord - 0.25, c_cord - 0.5, c_cord - 0.75])
	z = np.array([z_range[1], z_range[1], z_range[1], z_range[1], z_range[1]])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.875, r_cord - 0.875, r_cord - 0.875, r_cord - 0.875])
	y = np.array([c_cord - 0.75, c_cord - 0.25, c_cord - 0.25, c_cord - 0.75])
	z = np.array([z_range[1], z_range[1], z_range[0], z_range[0]])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.875, r_cord - 0.375, r_cord - 0.375, r_cord - 0.875])
	y = np.array([c_cord - 0.75, c_cord - 0.75, c_cord - 0.75, c_cord - 0.75])
	
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	y = y + 0.5
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.375, r_cord - 0.125, r_cord - 0.125, r_cord - 0.375])
	y = np.array([c_cord - 0.75, c_cord - 0.5, c_cord - 0.5, c_cord - 0.75])
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.125, r_cord - 0.375, r_cord - 0.375, r_cord - 0.125])
	y = np.array([c_cord - 0.5, c_cord - 0.25, c_cord - 0.25, c_cord - 0.5])	

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

def medic(r_cord, c_cord, team, board):
	if(team == True):
		team_color = 'black'
	else:
		team_color = 'red'
	z_range = [board[r_cord, c_cord] + 11, board[r_cord, c_cord] + 0.5 + 11]

	x = np.array([r_cord - 0.9, r_cord - 0.9, r_cord - 0.1, r_cord - 0.1])
	y = np.array([c_cord - 0.6, c_cord - 0.4, c_cord - 0.4, c_cord - 0.6])
	z = np.array([z_range[1], z_range[1], z_range[1], z_range[1]])
	
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.6, r_cord - 0.6, r_cord - 0.4, r_cord - 0.4])
	y = np.array([c_cord - 0.9, c_cord - 0.1, c_cord - 0.1, c_cord - 0.9])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.9, r_cord - 0.1, r_cord - 0.1, r_cord - 0.9])
	y = np.array([c_cord - 0.6, c_cord - 0.6, c_cord - 0.6, c_cord - 0.6])
	z = np.array([z_range[1], z_range[1], z_range[0], z_range[0]])
	
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	y = y + 0.2
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))

	x = np.array([r_cord - 0.6, r_cord - 0.4, r_cord - 0.4, r_cord - 0.6])
	y = np.array([c_cord - 0.9, c_cord - 0.9, c_cord - 0.9, c_cord - 0.9])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	y = y + 0.8
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))


	x = np.array([r_cord - 0.6, r_cord - 0.6, r_cord - 0.6, r_cord - 0.6])
	y = np.array([c_cord - 0.9, c_cord - 0.1, c_cord - 0.1, c_cord - 0.9])

	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	x = x + 0.2
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))


	x = np.array([r_cord - 0.9, r_cord - 0.9, r_cord - 0.9, r_cord - 0.9])
	y = np.array([c_cord - 0.6, c_cord - 0.4, c_cord - 0.4, c_cord - 0.6])
	
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))
	
	x = x + 0.8
	verts = [list(zip(x, y, z))]
	ax.add_collection3d(Poly3DCollection(verts, facecolor = team_color))


# Start-Up ----------  Start-Up ----------  Start-Up ----------  Start-Up ----------  Start-Up ----------

print('\nWelcome to the game!\n\n')
print('Press Control C at any time if you would like to quit.\n')

spc_landscape = False
while(spc_landscape == False):
	spc_landscape = choose_landscape()

# Start-Up ----------  Start-Up ----------  Start-Up ----------  Start-Up ----------  Start-Up ----------

print('\nThank you, the game landscape is now being generated!\n')

if(spc_landscape == 1): # High Mountains
	landscape_factor = 0.15
	board = board_scramble(board, landscape_factor)
	print('You will be playing on High Mountains.\n')
elif(spc_landscape == 2): # Medium Mountains
	landscape_factor = 0.225
	board = board_scramble(board, landscape_factor)
	print('You will be playing on Medium Mountains.\n')
elif(spc_landscape == 3): # Low Mountains
	landscape_factor = 0.3
	board = board_scramble(board, landscape_factor)
	print('You will be playing on Low Mountains.\n')
elif(spc_landscape == 4): # High Woodlands
	landscape_factor = 0.4
	board = board_scramble(board, landscape_factor)
	print('You will be playing in High Woodlands.\n')
elif(spc_landscape == 5): # Medium Woodlands
	landscape_factor = 0.5
	board = board_scramble(board, landscape_factor)
	print('You will be playing in Medium Woodlands.\n')
elif(spc_landscape == 6): # Low Woodlands
	landscape_factor = 0.6
	board = board_scramble(board, landscape_factor)
	print('You will be playing in Low Woodlands.\n')
elif(spc_landscape == 7): # High Valleys
	landscape_factor = 0.7
	board = board_scramble(board, landscape_factor)
	print('You will be playing in High Valleys.\n')
elif(spc_landscape == 8): # Medium Valleys
	landscape_factor = 0.775
	board = board_scramble(board, landscape_factor)
	print('You will be playing in Medium Valleys.\n')
elif(spc_landscape == 9): # Low Valleys
	landscape_factor = 0.85
	board = board_scramble(board, landscape_factor)
	print('You will be playing in Low Valleys.\n')
elif(spc_landscape == 10): # Mount Everest
	board = boardMax
	print('You will be playing on Mount Everest.\n')
elif(spc_landscape == 11): # Gasherbrum I
	landscape_factor = 0.1
	board = board_scramble(board, landscape_factor)
	print('You will be playing on Gasherbrum I.\n')
elif(spc_landscape == 12): # Mixed Mountains & Valleys
	landscape_factor = 0.5
	board = gen_mix_board(board, 250, 1, 11, landscape_factor)
	print('You will be playing in Mixed Mountains & Valleys.\n')
elif(spc_landscape == 13): # Kuril–Kamchatka Trench
	landscape_factor = 0.9
	board = board_scramble(board, landscape_factor)
	print('You will be playing in the Kuril–Kamchatka Trench.\n')
elif(spc_landscape == 14): # Mariana Trench
	board = boardMin
	print('You will be playing in the Mariana Trench.\n')

print('------- 2D Version of the Game Board -------')

print(board[1:11, 1:11])
fill_gaps = board - 1
status = check_board(board)
if(status == True):
	print('\nGenerating playing board...\n')
else:
	print('\nAn error occurred while generating the board. Please restart the game\n')
	sys.exit()

fig = plt.figure()
ax = fig.gca(projection='3d')
ax.set_aspect('equal')
# ax.set_axis_off()
ax.set_xlabel('x')
ax.set_ylabel('y')
fig.set_size_inches(12, 6.5)
# plt.ion()

measuring_stick = np.arange(-10, 10 + 0.1)
color_stick = ['navy', 'navy', 'navy', 
	'dodgerblue','dodgerblue','dodgerblue', 
	'saddlebrown', 'saddlebrown', 'saddlebrown', 
	'green','green', 'green', 
	'grey', 'grey', 'grey', 
	'white', 'white', 'white', 
	'skyblue', 'skyblue', 'skyblue']

xx, yy, zz = np.indices((10, 10, len(measuring_stick)))

plot_board(board, color_stick)

# ax.scatter(0.5, 0.5, board[1, 1] + 11.75, color = 'red', alpha=1, marker='X', linewidths = 5)
medic(1, 1, True, board)
soldier(2, 3, False, board)

plt.show()

# plt.pause(5)
print('\nHow to Play\n')
print('Movement')
print('-------------------------')
print('Every game piece has the same basic abilities. They can move in the forward, back, right, and left directions one square provided the adjacent square is the same elevation. '
	'They can move onto a square one unit higher than the square they are currently occupying in the forward, back, right, and left directions. '
	'They can also move to a square two or less units below the square they are currently occupying in forward, back, right, and left directions. '
	' \n')
print('Player 1 may now select their game pieces.\n')

turns = 0
while(turns<3):	
	fig = plt.figure()
	ax = fig.gca(projection='3d')
	ax.set_aspect('equal')
	# ax.set_axis_off()
	ax.set_xlabel('x')
	ax.set_ylabel('y')
	fig.set_size_inches(12, 6.5)
	plot_board()
	# fig.canvas.draw()
	# plt.show(block = False)

	# ax.scatter(turns+1.5, turns+1.5, board[turns+2, turns+2] + 11.75, color = 'red', alpha=1, marker='X', linewidths = 5)
	medic(turns + 2, 1, False, board)
	# print(board[turns+2, 1])
	soldier(2, 4 + turns, True, board)
	
	guard(3+turns, 2+turns, True, board)
	# plt.draw()
	# plt.pause(3)

	plt.show()
	# plt.hold()
	# plt.pause(3)
	turns += 1
	print(turns)

# plt.ioff()
plot_board()
ax.scatter(turns+1.5, turns+1.5, board[turns+2, turns+2] + 11.75, color = 'red', alpha=1, marker='X', linewidths = 5)
plt.show()

