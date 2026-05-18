import numpy as np
np.set_printoptions(threshold=np.nan)

def BoardGameCombos():
	startingboard=np.zeros((6,6))
	for i in range(1,5):
		startingboard[i:6-i,i:6-i]=startingboard[i:6-i,i:6-i]-2

	print(startingboard)
	halfway=sum(sum(startingboard))*-1
	# print(halfway)

	board0=np.zeros((6,6))

	finalBoards=startingboard
	combinations=1
	oldcombinations=0
	newcombinations=1
	startcheck=0



	termination=0
	#board=np.ones((12,1))
	#print(board)
	#finalBoards=np.hstack((board,board,finalBoards))
	#print(finalBoards)
	#print(finalBoards.shape[1])
	#d=finalBoards[0:12][0:12]
	#print(d)

	endPts=[4,4]


	counter=1
	while(counter<=(halfway*2+1)):
		gencombo=newcombinations
		newPts=[oldcombinations,gencombo]
		endPts=np.vstack((endPts,newPts))
		endPts[0,0]=counter
		print('End Points')
		print(endPts)

		if(endPts[0,0]>=64):
			startcheck=endPts[(endPts[0,0]-63),0]
		# print(gencombo)
		for i in range(1,5):
			for j in range(1,5):
				plusboard=np.zeros((6,6))
				plusboard[i][j]=1
				# print(plusboard)
				# print(oldcombinations,gencombo)
				for k in range(oldcombinations,gencombo):
					genboard=finalBoards[0:6,(6*k):((k+1)*6)]
					newboard=plusboard+genboard				
					# print(finalBoards)
					# print(newboard)
					if(np.all(newboard==startingboard*(-1))):
							combinations=combinations+1
							# combinations=combinations*2
							print(combinations)
							print('YOU DID IT!!!')
							termination=1
							return
				
					checks=0
					# print(newcombinations)
					for ii in range(startcheck,newcombinations):
						# print(ii)
						oldboard=finalBoards[0:6,(6*ii):((ii+1)*6)]
						ob1=oldboard[1:5,1:5]+1
						ob2=oldboard[1:5,1:5]+2
						ob3=oldboard[1:5,1:5]+3
						ob4=oldboard[1:5,1:5]+4

						checkboard=newboard[1:5,1:5]

						#print(oldboard)
						ob90=np.rot90(oldboard)
						ob90_1=ob90[1:5,1:5]+1
						ob90_2=ob90[1:5,1:5]+2
						ob90_3=ob90[1:5,1:5]+3
						ob90_4=ob90[1:5,1:5]+4

						ob180=np.rot90(ob90)
						ob180_1=ob180[1:5,1:5]+1
						ob180_2=ob180[1:5,1:5]+2
						ob180_3=ob180[1:5,1:5]+3
						ob180_4=ob180[1:5,1:5]+4

						ob270=np.rot90(ob180)
						ob270_1=ob270[1:5,1:5]+1
						ob270_2=ob270[1:5,1:5]+2
						ob270_3=ob270[1:5,1:5]+3
						ob270_4=ob270[1:5,1:5]+4

						# print(checkboard)
						# print(ob180_3)

						if(abs(newboard[i,j]-newboard[i-1,j])>2):
							break
						elif(abs(newboard[i,j]-newboard[i,j-1])>2):
							break
						elif(abs(newboard[i,j]-newboard[i+1,j])>2):
							break
						elif(abs(newboard[i,j]-newboard[i,j+1])>2):
							break
						elif(np.all(newboard==oldboard)):
							break
						elif(np.all(checkboard==ob1)):
							break
						elif(np.all(checkboard==ob2)):
							break
						elif(np.all(checkboard==ob3)):
							break
						elif(np.all(checkboard==ob4)):
							break
						elif(np.all(newboard==ob90)):
							break
						elif(np.all(checkboard==ob90_1)):
							break
						elif(np.all(checkboard==ob90_2)):
							break
						elif(np.all(checkboard==ob90_3)):
							break
						elif(np.all(checkboard==ob90_4)):
							break
						elif(np.all(newboard==ob180)):
							break
						elif(np.all(checkboard==ob180_1)):
							break
						elif(np.all(checkboard==ob180_2)):
							break
						elif(np.all(checkboard==ob180_3)):
							break
						elif(np.all(checkboard==ob180_4)):
							break
						elif(np.all(newboard==ob270)):
							break
						elif(np.all(checkboard==ob270_1)):
							break
						elif(np.all(checkboard==ob270_2)):
							break
						elif(np.all(checkboard==ob270_3)):
							break
						elif(np.all(checkboard==ob270_4)):
							break
						else:
							checks+=1

						if(checks==(newcombinations-startcheck)):
							print(newboard)
							finalBoards=np.hstack((finalBoards,newboard))
							combinations+=1
							print(combinations)
							newcombinations=combinations
							print(counter)

		oldcombinations=gencombo
		# print(oldcombinations)
		# print('old')

		if(termination==1):
			break

		counter+=1

	# finalBoards.transpose()
	# print(finalBoards)

BoardGameCombos()

