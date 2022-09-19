Include GeD_DetectorBuild_0.geo
Include GeD_DetectorBuild_1.geo
Include GeD_DetectorBuild_2.geo
Include GeD_DetectorBuild_3.geo
Include GeD_DetectorBuild_4.geo
Include GeD_DetectorBuild_5.geo
Include GeD_DetectorBuild_6.geo
Include GeD_DetectorBuild_7.geo
Include GeD_DetectorBuild_8.geo
Include GeD_DetectorBuild_9.geo
Include GeD_DetectorBuild_10.geo
Include GeD_DetectorBuild_11.geo
Include Intro.geo
Constant det_xoffset 4.5977
Constant det_yoffset 6.22935

Volume GeD_12Stack
GeD_12Stack.Visibility 0
GeD_12Stack.Virtual True
GeD_12Stack.Material Vacuum
GeD_12Stack.Shape BRIK 15 15 10

#### Stack 0 ####
Volume GeD_ThreeStack_0
GeD_ThreeStack_0.Visibility 1
GeD_ThreeStack_0.Material Vacuum
GeD_ThreeStack_0.Virtual True
GeD_ThreeStack_0.Shape BRIK 5 5 4
GeD_ThreeStack_0.Position -det_xoffset -det_yoffset 0
GeD_ThreeStack_0.Rotation 180 0 90
GeD_ThreeStack_0.Mother GeD_12Stack

GeD_Mother_0.Mother GeD_ThreeStack_0
GeD_Mother_0.Position 0 0 -2.54
GeD_Mother_1.Mother GeD_ThreeStack_0
GeD_Mother_1.Position 0 0 0.0
GeD_Mother_2.Mother GeD_ThreeStack_0
GeD_Mother_2.Position 0 0 2.54


#### Stack 1 ####
Volume GeD_ThreeStack_1
GeD_ThreeStack_1.Visibility 1
GeD_ThreeStack_1.Material Vacuum
GeD_ThreeStack_1.Virtual True
GeD_ThreeStack_1.Shape BRIK 5 5 4
GeD_ThreeStack_1.Position det_xoffset -det_yoffset 0
GeD_ThreeStack_1.Rotation 0 0 90
GeD_ThreeStack_1.Mother GeD_12Stack

GeD_Mother_3.Mother GeD_ThreeStack_1
GeD_Mother_3.Position 0 0 -2.54
GeD_Mother_4.Mother GeD_ThreeStack_1
GeD_Mother_4.Position 0 0 0.0
GeD_Mother_5.Mother GeD_ThreeStack_1
GeD_Mother_5.Position 0 0 2.54


#### Stack 2 ####
Volume GeD_ThreeStack_2
GeD_ThreeStack_2.Visibility 1
GeD_ThreeStack_2.Material Vacuum
GeD_ThreeStack_2.Virtual True
GeD_ThreeStack_2.Shape BRIK 5 5 4
GeD_ThreeStack_2.Position det_xoffset det_yoffset 0
GeD_ThreeStack_2.Rotation 180 0 -90
GeD_ThreeStack_2.Mother GeD_12Stack

GeD_Mother_6.Mother GeD_ThreeStack_2
GeD_Mother_6.Position 0 0 -2.54
GeD_Mother_7.Mother GeD_ThreeStack_2
GeD_Mother_7.Position 0 0 0.0
GeD_Mother_8.Mother GeD_ThreeStack_2
GeD_Mother_8.Position 0 0 2.54


#### Stack 3 ####
Volume GeD_ThreeStack_3
GeD_ThreeStack_3.Visibility 1
GeD_ThreeStack_3.Material Vacuum
GeD_ThreeStack_3.Virtual True
GeD_ThreeStack_3.Shape BRIK 5 5 4
GeD_ThreeStack_3.Position -det_xoffset det_yoffset 0
GeD_ThreeStack_3.Rotation 0 0 -90
GeD_ThreeStack_3.Mother GeD_12Stack

GeD_Mother_9.Mother GeD_ThreeStack_3
GeD_Mother_9.Position 0 0 -2.54
GeD_Mother_10.Mother GeD_ThreeStack_3
GeD_Mother_10.Position 0 0 0.0
GeD_Mother_11.Mother GeD_ThreeStack_3
GeD_Mother_11.Position 0 0 2.54


