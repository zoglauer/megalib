# MEGAlib Testing Rules

## Test Tiers
MEGAlib requires four test tiers. Every change is covered by at least one applicable tier, and class or function changes are expected to have unit-test coverage.

1. **Unit tests** — single class or function. Fast (<1s each). Run on every commit.
2. **Integration tests** — a few classes together, no full pipeline. Fast to medium. Run on every commit.
3. **End-to-end regression tests** — full pipeline with small fixed inputs. Slow. Run on PR and nightly.
4. **Physics validation tests** — MC output vs. analytic or reference result. Slow. Run nightly or on release.

## Workflow
1. Write unit tests against the public API first.
2. Add tests for non-public functions only when needed for coverage or behavior verification.
3. Audit real in-tree usage of the class; add tests for patterns found there.
4. Add or update integration and end-to-end tests when the change crosses class boundaries or touches the simulation/reconstruction/imaging chain.
5. Reassess overall completeness only after steps 1–4 are done.
6. Fix bugs one at a time. Stop after each fix for user review.
7. Make the minimum code change needed to fix a bug.
8. Do not refactor, restyle, or clean up unrelated code unless explicitly asked.

## Per-Function Unit Test Requirements
1. Every public function gets a direct functional test. Indirect coverage through other APIs does not count.
2. Every public function is tested with typical inputs AND edge cases.
3. Edge cases must include, when applicable: empty input, single-element input, zero, negative values, maximum size, and boundary values (n, n-1, n+1).
4. Every documented error path is tested. Verify the exact documented behavior (throw, return code, sentinel).
5. When a concrete expected value can be derived, assert the exact value. Range, finiteness, and non-empty checks are not sufficient.
6. For math and geometry helpers, use direct value assertions.
7. Use inverse or round-trip checks when they validate a helper more directly than forward computation.
8. For floating-point comparisons, use an explicit tolerance. Never use `==` on floats. Document the tolerance in a comment if non-obvious.
9. For stochastic functions, use a fixed seed and assert against a golden expected value.
10. For stateful classes, add direct tests that reuse the same object across multiple operations. This includes repeated reads, parses, loads, resets, and clears.
11. For owning classes, add lifecycle tests for copy constructor, assignment operator, clear/reset, and destruction semantics. Verify copied state is independent and metadata is preserved.
12. For I/O classes, test failure paths on reused objects and verify failed operations do not leave stale state behind.
13. Public API consistency is part of unit testing: every declared public function should be linkable and testable, or explicitly marked unsupported/deprecated.
14. Do not only test trivial, symmetric, or axis-point inputs. For every function, add several representative nontrivial interior-domain test cases when applicable.

## Monte Carlo and Stochastic Code
1. Every MC test fixes the random seed explicitly. No reliance on default seeding.
2. Record and assert the Geant4 version and physics list used when outputs are version-sensitive. A version or physics-list mismatch is a test failure, not a warning.
3. Record and assert the ROOT version when outputs depend on ROOT serialization or other version-sensitive behavior.
4. For distribution outputs, assert statistical properties (mean, variance, KS-test against reference) with documented tolerances, not exact bin contents.
5. For bit-exact reproducibility tests, assert exact equality only when seed, physics list, and library versions are all pinned.
6. Golden reference files are versioned. When physics or algorithms change intentionally, regenerate goldens in a dedicated commit with justification.
7. Small MC test inputs live in the repo. Large reference datasets live outside and are fetched by test setup.

## End-to-End Regression Tests
1. Each test specifies: input geometry, input source, event count, seed, Geant4 physics list, ROOT version.
2. Compare key observables — event counts per detector, spectrum peaks, reconstructed angles, image moments — against a reference within a documented tolerance.
3. A failure must report which observable drifted and by how much.
4. Keep end-to-end inputs small enough that one test runs in under a minute.

## Physics Validation Tests
1. Compare MC output against an analytic result where one exists (e.g., Klein-Nishina, photoelectric cross-sections, attenuation lengths).
2. Compare against published experimental reference values where no closed form exists.
3. Document the reference and its uncertainty in the test.
4. A physics validation failure is a release blocker, not a warning.

## Geometry and File Format Tests
1. Reference `.geo.setup` files are loaded and asserted on: volume count, material assignments, detector counts, bounding box.
2. For every file format MEGAlib reads or writes (`.sim`, `.tra`, `.evta`, `.rsp`, etc.), include a write-read round-trip test.
3. Round-trip tests assert byte-exact or field-exact equality depending on whether the format is binary or text.

## Test Isolation
1. Tests must pass in any order and in isolation.
2. Tests must not depend on other tests' side effects.
3. Restore any global state, singleton, static, or environment variable the test mutates. This includes ROOT's global state (`gROOT`, `gDirectory`, `gRandom`) and MEGAlib globals.
4. Use temp directories for filesystem work. Never touch the real user filesystem.
5. Do not make network calls in unit or integration tests.
6. Freeze or inject clocks; do not read wall-clock time directly.
7. Flaky tests are bugs. Fix or quarantine them. Do not add retries.
8. When a class owns dynamic resources, include at least one test that exercises reassignment or reinitialization, not just single-use success paths.

## Assertions and Diagnostics
1. Failure messages must show expected vs. actual without requiring a rerun.
2. One logical behavior per assertion block when practical. Group related assertions only when they describe the same behavior.
3. Name tests `<Unit>_<Scenario>_<ExpectedResult>` or match the style already in the file.

## Mocking
1. Use real implementations for value types and pure functions.
2. Mock only at I/O and system boundaries (filesystem, network, clock, external processes).
3. Do not mock the class under test.
4. Do not mock ROOT or Geant4. Use real instances with minimal inputs.

## What Not to Test
1. Do not test ROOT, Geant4, or other third-party library behavior.
2. Do not test compiler or language behavior.

## Coding Conventions
1. Prefer `apply_patch` for edits.
2. Prefer `rg` for searching.
3. Do not use `cout` or `cerr`. Use `mout`, `merr`, `mlog`, or `mgui`.
4. When touching code that uses `cout`/`cerr`, switching it to `mout`/`merr` is allowed.
5. Follow the conventions in `CodingConventions.md` at the repo root. If a rule here conflicts with that file, that file wins.

## Test Hygiene
1. Silence expected noisy streams by disabling the stream immediately before the call and re-enabling it immediately after.
2. Do not leave disabled streams across test boundaries.
3. Range, finiteness, and non-empty checks are fallback assertions only when no exact expected result is reasonably derivable.

## Harness
1. `make unittests` builds all unit tests.
2. A missing test binary is a failure.
3. A test binary exiting non-zero is a failure.
4. A silently skipped test is a failure.
5. Unit-test source files, binaries, and suite names use the `UT<ClassNameWithoutLeadingM>` convention, e.g. `MIsotope` -> `UTIsotope.cxx`, `bin/UTIsotope`, and `MUnitTest("UTIsotope")`.
6. In directories that use the standard unit-test makefile pattern, adding a `unittests/*.cxx` source file is sufficient to register a new unit test.
7. End-to-end and physics validation tests live outside the unit test harness and have their own runner. A change that touches simulation, reconstruction, or imaging must run the relevant end-to-end tier before merge.
