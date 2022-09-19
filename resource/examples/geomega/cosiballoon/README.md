# Mass model "COSI balloon"

This is the official mass model for the 2016 COSI balloon flight.


This document summarizes mass model v4, and possible improvements.


## Updates for massmodel v4:

- A few years ago, Alex created a python script to define each of the GeD's in an independent .geo file which can take into account the different thicknesses. At the time, he included detector information within the .geo file. 
- We have now separated the detector effects and trigger criteria for each GeD into a separate .det file, that is also automatically generated. This is to allow for a simplier transition between the different balloon mass models with different number of detectors active. In the process, many of the file names were modified in an attempt to make them easier to navigate. 
- As consequence, please use now COSIBalloon.12Detector.geo.setup, COSIBalloon.10Detector.geo.setup, or COSIBalloon.9Detector.geo.setup for your simulations and analysis.


## Updates for massmodel v3

- In 2016, we found that the tin thickness in the GradedZShield was too thick. This was modified in this version


## Previous updates

Annotation: the old version was 1.0, 1.1 which then was switched to version v1, v2, etc.The below documentation has not been updated

A few changes have been made in between MassModel_1.1 and *_1.0. To be honest, I wasn't very good at keeping track of revisions, so the current MassModel_1.0 has had quite a few changes over the past year, I figure now is the time to get better at documentton and check and sums. 
 A lot of MassModel_1.0 was copied directly from MassModel_0.9 (which probably was mostly copied from previous models), and I've tried to leave comments in the files to indicate that they were copied and not edited. All of the edits and new .geo files were created by Carolyn Kierans throughout Feburary and March of 2014, in preparation for the 2014 Antarctica flight.

I've created three .geo.setup files to be used as simulation geometries:
- COSI.DetectorHead.geo.setup includes the cryostat (including cooler and vacuum tube and the rest) CsI shields and thermal radiators. I think this will be the geometry file for most simulations. Without the gondola, the surrounding sphere is a reasonable size and this still contains the majority of the weight and important volumes of the instument.
- COSI.WithGondola.geo.setup is the above geometry in addition to the aluminum gondola frame structure (including card cages). Because this volume is a lot bigger than just COSI.DetectorHead.geo.setup, the simulations will take a much longer time.
- COSI.WithGondolaAndAntenna.geo.setup is the same as the above geometry, but also includes the Antenna, for when we start doing measurments in lab with the antenna attached. The surrounding sphere is again larger in the geometry, so simulation time will be longer. Currently, the Gondola.geo file needs to be edited to include the Antenna (see the file for details), but this is an easy fix.


Big changes since MassModel_0.9, other than all of the new .geo files I've created:
- Changed NCT->COSI
- I deleted all obsolete materials in COSIMaterials.geo
- I deleted all obsolete volume in GeD_VolumeDefinitions.geo (used to be NCT_VolumeDefinitions.geo)



Improvements/new volumes that can be created to better model the current instrument (in no particular order):
- Change the desity of the Cryocooler bellows. Currently it's just aluminum, but because it's not a solid piece, this is an over estimate of the weight.
- Mass of vacuum tube is a bit low, this can probably be improved by better flange models.
- Can do a bit better with the cryostat top. I don't currently include the thicker outer rib that follows the walls of the cryostat.
- Could add more cut out in the cryostat walls. The mass is still a bit larger in geomega than in solidworks.
- Could add the aluminum panels that the card cages mount to.
- I've made an approximation with the thermal radiators. For simplicity, I modeled them as a large flat plate with a quarter of a cylindrical prism at one end, but truely it is a triangular prism, but triangles are hard. The final mass is close.
- Include the thermal blankets
- Include the charcoal getters
- Include the straps that support the detectors inside the cryostat
- Include the copper plate underneath the cryocooler
- Include boxes in the electronics bay
- Include foam and thin aluminum for the gondola panels.
- Include top aluminum and foam shield in between the cryostat gondola layer and the e-bay.
- A lot of work can be put into remodeling the GeD mounts. They don't currently represent the latest model, but this would be a pretty big job.:






--------------------------------------------------------------------------------------------------------------------
Alex's comments for MassModel_0.9. Most of these are irrelevant now, but I wanted to keep the information somewhere:

MassModel_0.9 details:

-A rough model of the cryocooler is now included and positioned in the shield well
-The preamps on two sides of the cryostat have been moved up in order to accomodate the cryocooler
-The cryostat itself is now (more accurately) modeled by a 1/2" thick aluminum box on the bottom and
 sides, with a ribbed top.  I'm assuming (as per Chris Smith's suggestion) that the minimum thickness
 of the ribbed top is 0.1" and that the max is 0.3".
-I added 4 "thermal wings" which are the pieces of aluminum that attach the top of the cryostat
 to the top of the CsI shields (for thermal reasons)
-I shrunk the surrounding sphere a bit and removed the external structure so as to only simulate the
 important stuff.
-The distance between the top of the bottom-most shield module and the bottom of the bottom-most 
 germanium detector is 2.5".  This is Chris' current best estimate of this parameter.
- Coldfinger was modified so as to not interfere with the newly sized IR shield

2/8/13

including an air slab for the kiruna simulations... using atmospheric_profile_depth.py to calulate
the atmospheric depth (by integrating the output of nrlmsise) three different points along the 
flight trajectory so that I can get an average depth.

altitude = 40.0
geo_lat = 67.85
geo_lon = 20.235
Atmospheric Depth =  3.44280994272  gm/cm2

altitude = 40.0
geo_lat = 67.85
geo_lon = -47.02889
Atmospheric Depth =  3.4397269889  gm/cm2

altitude = 40.0
geo_lat = 67.85
geo_lon = -106.678
Atmospheric Depth =  3.46732506866  gm/cm2

looks like it doesn't change much. 3.450 gm/cm2 is the average.


