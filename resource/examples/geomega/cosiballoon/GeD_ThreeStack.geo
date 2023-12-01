#Defines three GeD detectors stacked vertically to simlify the geometry


Include Intro.geo
Include GeD_DetectorBuild.geo


Volume GeD_ThreeStack
GeD_ThreeStack.Visibility 1
GeD_ThreeStack.Material Vacuum
GeD_ThreeStack.Virtual True
GeD_ThreeStack.Shape BRIK 5 5 4

GeD_Mother.Copy GeD_Mother_Copy00
GeD_Mother_Copy00.Virtual True
GeD_Mother_Copy00.Position 0 0 2.54
GeD_Mother_Copy00.Rotation 0 0 0
GeD_Mother_Copy00.Mother GeD_ThreeStack

GeD_Mother.Copy GeD_Mother_Copy01
GeD_Mother_Copy01.Virtual True
GeD_Mother_Copy01.Position 0 0 0
GeD_Mother_Copy01.Rotation 0 0 0
GeD_Mother_Copy01.Mother GeD_ThreeStack

GeD_Mother.Copy GeD_Mother_Copy02
GeD_Mother_Copy02.Virtual True
GeD_Mother_Copy02.Position 0 0 -2.54
GeD_Mother_Copy02.Rotation 0 0 0
GeD_Mother_Copy02_GEWA[GEWA_Copy1].Color 3
GeD_Mother_Copy02.Mother GeD_ThreeStack
