
#read in the template for the GeD definition
f = open("GeD_DetectorBuild.geo.template","r")
gedtemplate = f.read()
f.close()

#thick = [1.49,1.45,1.50,1.51,1.50,1.47,1.48,1.47,1.49,1.47,1.42,1.45] #COSI 2014
thick = [1.49,1.45,1.50,1.45,1.51,1.50,1.48,1.47,1.49,1.47,1.42,1.45] #COSI 2016
fnames = []

detlines = ['']
triggerlines = [ '' ]

for i,th in enumerate(thick):

	#define GeD_Mother_i
	ged_mother = 'GeD_Mother_' + str(i)
	lines = ['Volume ' + ged_mother,\
			   ged_mother + '.Visibility 1',\
				ged_mother + '.Virtual True',\
				ged_mother + '.Material mvacuum',\
				ged_mother + '.Shape BRIK 10. 3. 7.',\
				'']

	#define the Ge wafer
	ged_wafer = 'GeWafer_' + str(i)
	lines += ['Volume ' + ged_wafer,\
				 ged_wafer + '.Visibility 1',\
				 ged_wafer + '.Material mgermanium_ge_recoil',\
				 ged_wafer + '.Shape BRIK 4.025 4.025 ' + str(th/2.0),\
				 ged_wafer + '.Position 0. 0. ' + str( 0.5 * (th - 1.5) ),\
				 ged_wafer + '.Color 2',\
				 ged_wafer + '.Mother ' + ged_mother,\
				 '']

	#define the Ge corner
	ged_corner = 'GeCorner_' + str(i)
	lines += ['Volume ' + ged_corner,\
				 ged_corner + '.Visibility 1',\
				 ged_corner + '.Material mvacuum',\
				 ged_corner + '.Shape TRD1 0 0.805 ' + str(th/2.0) + ' 0.403',\
				 '']

	#make the Ge corner copies
	corner_positions = ['3.73 3.73 0','-3.73 3.73 0','-3.73 -3.73 0','3.73 -3.73 0']
	corner_rotations = ['90 -45 0 45 90 -135','90 45 0 135 90 -45','90 135 0 -135 90 45','90 -135 0 -45 90 135']
	for j,data in enumerate(zip(corner_positions,corner_rotations)):
		corner_copy = ged_corner + '_' + str(j)
		lines += [ged_corner + '.Copy ' + corner_copy,\
					 corner_copy + '.Position ' + data[0],\
					 corner_copy + '.Rotation ' + data[1],\
					 corner_copy + '.Mother ' + ged_wafer,\
					 '']

	fname = 'GeD_DetectorBuild_' + str(i) + '.geo'; fnames.append(fname)
	fout = open(fname,'w')
	fout.writelines(map(lambda x:x + '\n',lines))
	fout.write( gedtemplate.replace('QQQ',str(i)) )
	fout.close()


	#add in the detector + trigger info
	detector = 'Detector' + str(i)
	trigger = detector + '_Trigger'
	grtrigger = detector + '_GRTrigger'
	detlines = ["Include " + fname + '\n']
	detlines += ['Strip3D ' + detector,\
				 detector + '.DetectorVolume ' + ged_wafer,\
				 detector + '.SensitiveVolume ' + ged_wafer,\
				 detector + '.StructuralPitch 0.0 0.0 0.0',\
				 detector + '.StructuralOffset 0.0 0.0 0.0',\
				 detector + '.Offset 0.32 0.32',\
				 detector + '.StripNumber 37 37',\
				 detector + '.EnergyResolution Gauss 0 0 1.02',\
				 detector + '.EnergyResolution Gauss 100 100 1.02',\
				 detector + '.EnergyResolution Gauss 300 300 1.04',\
				 detector + '.EnergyResolution Gauss 1000 1000 1.07',\
				 detector + '.EnergyResolution Gauss 3000 3000 1.15',\
				 detector + '.EnergyResolution Gauss 10000 10000 1.42',\
				 detector + '.DepthResolution 100 0.02',\
				 detector + '.NoiseThreshold 15.0',\
				 detector + '.TriggerThreshold 40.0',\
				 detector + '.GuardringEnergyResolution 100 1.5',\
				 detector + '.GuardringEnergyResolution 1000 1.5',\
				 detector + '.GuardringTriggerThreshold 40',\
				 detector + '.FailureRate 0.0',\
				 '']
	triggerlines = [ 'Trigger ' + trigger,\
				 trigger + '.Veto False',\
				 trigger + '.TriggerByChannel True',\
				 trigger + '.Detector ' + detector + ' 1',\
				 '',\
				 'Trigger ' + grtrigger,\
				 grtrigger + '.Veto True',\
				 grtrigger + '.TriggerByDetector True',\
				 grtrigger + '.GuardringDetector ' + detector + ' 1',\
				 '']

	# Write the individual .det files
	fname = 'GeD_DetectorBuild_' + str(i) + '.det'
	fout = open(fname,'w')

	fout.writelines(map(lambda x:x + '\n',detlines))
	fout.writelines(map(lambda x:x + '\n',triggerlines))
	fout.close()



# Now write the tigger criteria file
#fout = open('GeD_TriggerCriteria.det','w')
#fout.writelines(map(lambda x:x + '\n',triggerlines))
#fout.close()


##################################
#now make the file GeD_12Stack.geo
##################################

lines = ["Include " + f for f in fnames]
lines += ['Include Intro.geo',\
			 'Constant det_xoffset 4.5977',\
			 'Constant det_yoffset 6.22935',\
			 '']

#define the 12 stack
lines += ['Volume GeD_12Stack',\
			'GeD_12Stack.Visibility 0',\
			'GeD_12Stack.Virtual True',\
			'GeD_12Stack.Material Vacuum',\
			'GeD_12Stack.Shape BRIK 15 15 10',\
			'']

ZSpacing = [2.54,0.0,-2.54]
IsReversed = [True,False,False,False]
#stack_positions = ['-det_xoffset -det_yoffset 0','-det_xoffset det_yoffset 0','det_xoffset -det_yoffset 0','det_xoffset det_yoffset 0']
#stack_rotations = ['180 0 90','0 0 -90','0 0 90','180 0 -90']
stack_positions = ['-det_xoffset -det_yoffset 0','det_xoffset -det_yoffset 0','det_xoffset det_yoffset 0','-det_xoffset det_yoffset 0']
stack_rotations = ['180 0 90','0 0 90','180 0 -90','0 0 -90']
for S in range(4): #loop over stacks
	stack = 'GeD_ThreeStack_' + str(S)
	lines += ['#### Stack '+str(S)+' ####']
	lines += ['Volume ' + stack,\
				stack + '.Visibility 1',\
				stack + '.Material Vacuum',\
				stack + '.Virtual True',\
				stack + '.Shape BRIK 5 5 4',\
				stack + '.Position ' + stack_positions[S],\
				stack + '.Rotation ' + stack_rotations[S],\
				stack + '.Mother GeD_12Stack',\
				'']
	zspace = ZSpacing
	if IsReversed[S]:
		zspace.reverse()
	#put the detectors in the stack
	for D in range(3):
		ged_mother = 'GeD_Mother_' + str(S*3 + D)
		lines += [ged_mother + '.Mother ' + stack,\
					ged_mother + '.Position 0 0 ' + str(zspace[D])]

	lines += ['\n']

fout = open('GeD_12Stack.geo','w')
fout.writelines(map(lambda x:x + '\n',lines))
fout.close()



