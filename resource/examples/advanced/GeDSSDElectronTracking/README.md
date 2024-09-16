
The example show how to run the training and testing of neural network based approach to determine the recoil electron direction.

Steps to reproduce the results:

1. Run the simulations
```
mcosima GeDSSDElectronTracking.source
```

2. Run the training/testing data generation
```
responsecreator -m tf -g GeDSSDElectronTracking.geo.setup -f GeDSSDElectronTracking.p1.inc1.id1.sim.gz -r GeDSSDElectronTracking -c GeDSSDElectronTracking.revan.cfg
```

3. Run the application and check the performance
```
python3 run.py -f GeDSSDElectronTracking.x2.y1.electrontrackingwithcrossstripdetector.tmva.root
```
