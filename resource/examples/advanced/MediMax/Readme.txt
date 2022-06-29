# This directory shows how to create and use the near-field list-mode reponse

# Perform simulations (as many cosima instances as you have core in your CPU):
mcosima -z MediMax.source

# Create the responses (again, as many instances as you have cores in your CPU)
mresponsecreator -g MediMax.geo.setup -m en -o xmin=-20:xmax=20:xbins=40:ymin=-5:ymax=5:ybins=40:zmin=-20:zmax=20:zbins=40:emin=500:emax=520:ebins=1 -c MediMax.revan.cfg -b MediMax.mimrec.cfg -r MediMax -z -f MediMaxResponse.p1.inc*.id1.sim.gz

# Join the response files
responsemanipulator -j MediMax.p1

# Look at the response files

# See Emittedxdetectedanywhere.png for how it should look like
responsemanipulator -v MediMax.listmoderesponsenearfield.emittedxdetectedanywhere.rsp.gz x 0 y 511

# See Emittedxdetectedy.png for how it should look like
responsemanipulator -v MediMax.listmoderesponsenearfield.emittedxdetectedy.rsp.gz x 0 y 5115 0 0 511

