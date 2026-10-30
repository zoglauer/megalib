#!/usr/bin/env python3
"""Generate isolated fixtures and run parser, navigation and transport tests."""
import argparse
import math
from pathlib import Path
import struct
import subprocess
import tempfile
import time


BOX_VERTICES = [(-1, -2, -3), (1, -2, -3), (1, 2, -3), (-1, 2, -3),
                (-1, -2, 3), (1, -2, 3), (1, 2, 3), (-1, 2, 3)]
BOX_FACES = [(0, 2, 1), (0, 3, 2), (4, 5, 6), (4, 6, 7),
             (0, 1, 5), (0, 5, 4), (3, 7, 6), (3, 6, 2),
             (0, 4, 7), (0, 7, 3), (1, 2, 6), (1, 6, 5)]
BASE = """Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo
SurroundingSphere 50 0 0 0 50
Volume World
World.Material Vacuum
World.Shape BRIK 60 60 60
World.Mother 0
"""


def obj(vertices, faces):
    return "".join("v " + " ".join(map(str, v)) + "\n" for v in vertices) + "".join(
        "f " + " ".join(str(i+1) for i in f) + "\n" for f in faces)


def inline(vertices, faces, name="Inline"):
    shape = name + "_Shape"
    return (f"TESSELLATED {len(vertices)} {len(faces)}\n{shape}.Vertices {len(vertices)}\n" +
            "".join(f"{shape}.Vertex " + " ".join(map(str, v)) + "\n" for v in vertices) +
            f"{shape}.Facets {len(faces)}\n" +
            "".join(f"{shape}.Facet " + " ".join(map(str, f)) + "\n" for f in faces))


def volume(name, shape, position="0 0 0"):
    return (f"Volume {name}\n{name}.Material Aluminium\n{name}.Mother World\n"
            f"{name}.Position {position}\n{name}.Shape {shape}\n")


def source(directory, name, geometry, events=200):
    path = directory / f"{name}.source"
    path.write_text(f"""Version 1
Geometry {geometry}
PhysicsListEM Standard
StoreSimulationInfo all
StoreOnlyTriggeredEvents false
DiscretizeHits false
DefaultRange 0.01
Run Test
Test.FileName {directory / name}
Test.NEvents {events}
Test.Source Beam
Beam.ParticleType 1
Beam.Beam HomogeneousBeam 0 0 10 0 0 -1 0.1
Beam.Spectrum Mono 100
Beam.Flux 1
""")
    return path


def run(command, directory, name, timeout=180):
    start = time.monotonic()
    log = directory / f"{name}.log"
    with log.open("w") as output:
        result = subprocess.run(list(map(str, command)), cwd=directory, stdout=output,
                                stderr=subprocess.STDOUT, timeout=timeout)
    text = log.read_text()
    if result.returncode:
        raise RuntimeError(f"{name} exited {result.returncode}; see {log}\n{text[-6000:]}")
    print(f"{name}: completed in {time.monotonic()-start:.2f}s", flush=True)
    return text


def test(args, directory):
    (directory / "box.obj").write_text(obj(BOX_VERTICES, BOX_FACES))
    (directory / "box-ascii.stl").write_text("solid box\n" + "".join(
        "facet normal 0 0 0\nouter loop\n" + "".join(
            "vertex " + " ".join(map(str, BOX_VERTICES[i])) + "\n" for i in face) +
        "endloop\nendfacet\n" for face in BOX_FACES) + "endsolid box\n")
    # Binary STL exercises the independent file loader, including vertex welding.
    with (directory / "box.stl").open("wb") as output:
        output.write(b"test box".ljust(80, b"\0"))
        output.write(struct.pack("<I", len(BOX_FACES)))
        for face in BOX_FACES:
            coordinates = [v for i in face for v in BOX_VERTICES[i]]
            output.write(struct.pack("<12fH", 0, 0, 0, *coordinates, 0))

    # An L-shaped prism: area 3 cm2, height 2 cm. The notch is outside.
    polygon = [(0, 0), (2, 0), (2, 1), (1, 1), (1, 2), (0, 2)]
    vertices = [(x, y, z) for z in (-1, 1) for x, y in polygon]
    caps = [(0, 1, 3), (1, 2, 3), (0, 3, 5), (3, 4, 5)]
    faces = [(a, c, b) for a, b, c in caps] + [(a+6, b+6, c+6) for a, b, c in caps]
    for a in range(6):
        b = (a+1) % 6
        faces.extend([(a, b, b+6), (a, b+6, a+6)])
    (directory / "concave.obj").write_text(obj(vertices, faces))
    fixtures = directory / "fixtures.geo.setup"
    fixtures.write_text(BASE + volume("Mesh", "TESSELLATED box.obj") +
                        "Mesh.Copy Rotated\nRotated.Position 10 0 0\nRotated.Rotation 0 0 90\nRotated.Mother World\n" +
                        volume("Inline", inline(BOX_VERTICES, BOX_FACES), "-10 0 0") +
                        volume("STL", "TESSELLATED box.stl", "0 -10 0") +
                        volume("ASCII", "TESSELLATED box-ascii.stl", "10 -10 0") +
                        "Shape TESSELLATED SharedBox\nSharedBox.MeshFile box.obj\n" +
                        volume("Named", "SharedBox", "10 10 0") +
                        volume("Scaled", "TESSELLATED box.obj", "-10 -10 0") + "Scaled.Scale 2 XY\n" +
                        volume("Analytic", "BRIK 1 2 3", "0 10 0") +
                        volume("Concave", "TESSELLATED concave.obj", "-10 10 0"))
    output = run([args.checker, source(directory, "fixtures", fixtures), "fixtures"], directory, "fixtures")
    assert "PASS: ROOT and Geant4" in output, output

    invalid = {
        "open": (BOX_VERTICES, BOX_FACES[:-1], "open, non-manifold"),
        "flipped": (BOX_VERTICES, [BOX_FACES[0][::-1]] + BOX_FACES[1:], "inconsistent facet winding"),
        "inward": (BOX_VERTICES, [f[::-1] for f in BOX_FACES], "outward orientation"),
        "bad_index": (BOX_VERTICES, [(0, 2, 100)] + BOX_FACES[1:], "invalid facet vertex index"),
        "degenerate": (BOX_VERTICES, [(0, 0, 1)] + BOX_FACES[1:], "repeated facet vertex index"),
        "nonfinite": ([(float("nan"), -2, -3)] + BOX_VERTICES[1:], BOX_FACES, "finite numbers"),
        # Closed positive-volume tetrahedron with tiny nonzero faces: ROOT used
        # to print errors yet accept this because closure alone passed.
        "tiny": ([(0, 0, 0), (1.e-6, 0, 0), (0, 1.e-6, 0), (0, 0, 1.e-6)],
                 [(0, 2, 1), (0, 1, 3), (1, 2, 3), (2, 0, 3)], "ROOT rejected"),
    }
    for name, (v, f, diagnostic) in invalid.items():
        geometry = directory / f"{name}.geo.setup"
        geometry.write_text(BASE + volume("BadMesh", inline(v, f, "BadMesh")))
        output = run([args.checker, source(directory, name, geometry), "reject"], directory, name)
        assert "PASS: invalid mesh rejected" in output and diagnostic in output, output
        assert "BadMesh" in output, "Missing shape name in error"

    # Run the actual application and inspect stored physical interactions.
    interactions = []
    for name, shape in [("transport_mesh", "TESSELLATED box.obj"), ("transport_brik", "BRIK 1 2 3")]:
        geometry = directory / f"{name}.geo.setup"
        geometry.write_text(BASE + volume("Target", shape))
        output = run([args.cosima, "-s", "12345", "-u", source(directory, name, geometry)], directory, name)
        for error in ("GeomNav", "Stuck Track", "Unknown volume type", "Unable to initalize", "Fatal Exception"):
            assert error not in output, f"{name}: {error}"
        files = list(directory.glob(f"{name}*.sim"))
        assert len(files) == 1, f"Expected one simulation output: {files}"
        text = files[0].read_text()
        events = sum(line == "SE" for line in text.splitlines())
        count = sum(line.startswith(("IA COMP", "IA PHOT", "IA RAYL")) for line in text.splitlines())
        assert events == 200, f"{name}: stored {events}/200 events"
        assert count > 50, f"{name}: insufficient physical interactions ({count})"
        for line in text.splitlines():
            if line.startswith(("IA COMP", "IA PHOT", "IA RAYL")):
                # Physical interactions must occur inside the target, in cm.
                position = list(map(float, line.split(";")[4:7]))
                assert all(abs(x) <= bound + 1.e-4 for x, bound in zip(position, (1, 2, 3))), line
        interactions.append(count)
    # Same seeded setup should give comparable interactions; allow statistical
    # differences if boundary ordering changes random-number consumption.
    assert abs(interactions[0]-interactions[1]) < 6*math.sqrt(sum(interactions)), interactions
    print(f"PASS: 200 photons each through mesh and BRIK; interaction counts {interactions}", flush=True)

    if args.geometry:
        output = run([args.checker, source(directory, "assembly", args.geometry.resolve()), "assembly"],
                     directory, "assembly", timeout=1800)
        assert "PASS: ROOT and Geant4" in output, output
        print(output[output.rfind("PASS:"):].strip(), flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--checker", required=True, type=Path)
    parser.add_argument("--cosima", required=True, type=Path)
    parser.add_argument("--geometry", type=Path, help="Also construct an existing assembly in both backends")
    parser.add_argument("--output-dir", type=Path, help="Keep fixtures, simulation outputs and logs here (must be new)")
    arguments = parser.parse_args()
    if arguments.output_dir:
        arguments.output_dir = arguments.output_dir.resolve()
        arguments.output_dir.mkdir(parents=True, exist_ok=False)
        test(arguments, arguments.output_dir)
    else:
        with tempfile.TemporaryDirectory(prefix="megalib-tessellated-") as temporary:
            test(arguments, Path(temporary))
    print("PASS: tessellated regression suite")
