# This script will perform the standard point-source simulations necessary for standard performance estimates for Compton telescopes using SensitivityOptimizer
#
# Modify the file SimulationParameters.dat to set simulation directories, number of events, geometry, revan configuration file, etc.
#
# It simulates:
# (1) A list of narrow line point sources as defined in the parameter files 
# (2) A default set of broad line point sources (1-keV (1 sigma) 511 keV, 14-keV (1 sigma) 847-keV line, 4-keV (1 sigma) 1157-keV lines)
# (3) A set of polarized and unpolarized continuum point sources with Crab-like -2 power law, the start energy is defined in the paramerters file, highest energy is 10 GeV
#
# The script will
# (1) Generate the cosima source files
# (2) Simulate them with mcosima locally in this machine in parallel
# (3) Reconstruct the with mrevan in parallel on this machine, assuming a revan configuration file is given 
#
# Launch the script via
bash SimulateSources.sh


