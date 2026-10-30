# Tessellated-solid validation — 2026-10-30

Implementation and builds used `/home/bruce/MEGAlib`, ROOT 6.40.04 and
Geant4 10.02.p03. No input meshes or generated COSI geometry were modified.

## Automated regression results

`make cos` and `make test-tessellated` passed.

- Real Geomega parsing and Cosima construction of inline, OBJ, ASCII STL and
  binary STL meshes, named shapes, scaling and a rotated copy.
- ROOT dictionary availability and ROOT/Geant4 facet-count and volume agreement.
- Box containment, safety, entry/exit and missed rays compared with an analytic
  box; concave-prism volume, interior/notch classification and notch entry.
- Sampled fixture overlap checks and Geant4 navigation through a rotated copy.
- Rejection of open meshes, flipped facets, inward winding, invalid indices,
  repeated-index facets, nonfinite coordinates and tiny closed tetrahedra.
- Two seeded Cosima runs of 200 photons at 100 keV, through an aluminium mesh
  box and an equivalent `BRIK`. Both stored all 200 events and produced 379
  physical interactions. Interaction coordinates were inside the target.

The standalone `resource/examples/cosima/source/TessellatedBox.source`
example also completed all 200 events.

## COSI reduced assembly

Input:
`/home/bruce/cad-conversion-dev/output/cosi_test_reduced/cosi_test.geo.setup`

The optional assembly check passed for all **200 tessellated volumes and
883,320 facets**, including facet counts and ROOT/Geant4 volume agreement.
That check took 134 seconds in this run.

A separate actual Cosima run used:

```text
Version 1
Geometry /home/bruce/cad-conversion-dev/output/cosi_test_reduced/cosi_test.geo.setup
PhysicsListEM Standard
StoreSimulationInfo all
StoreOnlyTriggeredEvents false
DiscretizeHits false
DefaultRange 0.01
CheckForOverlaps 1000 0.0001
Run Test
Test.FileName assembly
Test.NEvents 1000
Test.Source Beam
Beam.ParticleType 1
Beam.Beam FarFieldPointSource 0 0
Beam.Spectrum Mono 100
Beam.Flux 1
```

With seed 12345, all **1,000 events** were stored, with **177 Compton and 41
photoelectric interactions**. No Geant4 overlap warnings, stuck tracks or
navigation errors were reported. Overlap sampling used 1,000 surface points
per placed volume and a tolerance of 0.0001 cm (1 micrometer).

Wall time, including loading and overlap checks, was 87.03 seconds. Peak
resident memory was 2,236,520 KiB (about 2.13 GiB). Geant4 reported 0.105 seconds
of CPU time for the event loop. These are individual smoke-test measurements,
not a controlled performance benchmark.

Logs and outputs from this session are in `/tmp/megalib-tessellated-assembly/`:
`assembly.log`, `assembly_transport.source`, `assembly_transport.log` and
`assembly.inc1.id1.sim`. Regression logs are also available in
`/tmp/tessellated-final-tests.log` and `/tmp/megalib-tessellated-run2/`.
Temporary files may be removed by the system; the test suite and the source
configuration above reproduce the checks.

The assembly retained its placeholder aluminium materials and has no defined
detector/trigger configuration. This validates passive geometry transport.
Sampled overlaps and a finite photon run do not certify absence of all
overlaps or self-intersections, or physical fidelity of the material model.
