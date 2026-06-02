# MEGAlib Testing Rules

## Preamble

### Information for biological life forms:

This document is written by and for AIs.
When you want to add, extend, or review a unit test — or have an AI review
production code — have the AI read this document together with the MEGAlib
style guide `CodingConventions.md` at the repository root.

Begin every such prompt with:

```
Please read the MEGAlib style guide: CodingConventions.md
Please read the MEGAlib testing rules: TestingRules.md

If you find a bug in production code, do not modify those files
automatically — show a diff and ask before applying, and fix only one
bug at a time, pausing for approval after each.
```

Then state the task.  Example prompts:

Add a unit test for a class:

```
Add a unit test for the class in src/global/misc/src/MString.cxx to
src/global/misc/unittests/, following the same rules and style as the
existing unit tests. Cover the whole public API and work through the
checklist in section "Per-Function Unit Test Requirements".
```

Review or extend an existing unit test:

```
Review src/global/misc/unittests/UTString.cxx against MString for
completeness and correctness using the checklist in section
"Per-Function Unit Test Requirements". List the coverage gaps and weak
assertions, then add the missing coverage.
```

Review production code for bugs:

```
Do a thorough review of src/global/misc/src/MString.cxx and
src/global/misc/inc/MString.h for bugs. Report each finding with a
severity, propose fixes as diffs, and apply them one at a time only
after I approve.
```

Add an end-to-end regression test:

```
Add an end-to-end regression test for <pipeline> following section
"End-to-End Regression Tests". It must be fully automatic, safe to run
in parallel with other tests, and pin the geometry, source, event count,
seed, Geant4 physics list, and ROOT version.
```

### Information for AIs

This document captures the rules and patterns that govern every unit,
integration, end-to-end, and physics-validation test in MEGAlib.

Read it before writing a new test; keep it up to date when a new pattern
is established.

When you find a bug in production code, do not fix it silently while
writing tests.  Fix one bug at a time, show the diff and wait for
approval, make the minimum change that fixes it, and do not refactor or
restyle unrelated code unless explicitly asked.

---

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

## File Structure and main()

1. Unit tests live in `unittests/` next to the source they cover (e.g. `src/global/misc/unittests/UTString.cxx` for `src/global/misc/src/MString.cxx`). One test file per class.
2. Test class derives from `MUnitTest`. The constructor passes the test name as a string. `Run()` is virtual.
3. `Run()` chains every sub-test with `&& Passed` so one failure does not skip the rest, then calls `Summarize()` and returns `Passed`.
4. `main()` constructs the test, calls `Run()`, returns `0` on pass and `1` on fail.
5. Call `MGlobal::Initialize("UTFoo", "Unit tests for MFoo")` from `main()` only when the class under test depends on MEGAlib's global state (ROOT initialization, environment, etc.). Pure data and helper classes do not need it.
6. Include order: MEGAlib headers first (`MFoo.h`, `MUnitTest.h`), then standard headers (`<sstream>`, etc.), then `using namespace std;`.

Boilerplate template:

```cpp
// MEGAlib:
#include "MFoo.h"
#include "MUnitTest.h"

// Standard lib:
#include <sstream>
using namespace std;


class UTFoo : public MUnitTest
{
public:
  UTFoo() : MUnitTest("UTFoo") {}
  virtual ~UTFoo() {}

  virtual bool Run();

private:
  bool TestDefaultConstruction();
  bool TestGettersSetters();
  // ... one method per logical concern
};

bool UTFoo::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters()      && Passed;
  // ...

  Summarize();
  return Passed;
}

int main()
{
  UTFoo Test;
  return Test.Run() == true ? 0 : 1;
}
```

## MUnitTest API

1. All assertions use the `MUnitTest` helpers — never raw `assert`, `if (...) return false;`, or hand-rolled comparison output. Every helper shares the same first three arguments: `Function`, `Input`, `Description`.
2. The first argument is the function signature under test as a string (e.g. `"SetEnergy/GetEnergy"`, `"Parse()"`, `"operator+="`). For tests that exercise a scenario rather than a single function (e.g. end-to-end runs), use the scenario name.
3. The second argument is a short description of the input or scenario (e.g. `"representative value 511 keV"`, `"empty input"`, `"after Clear"`).
4. The third argument is a complete English sentence describing the expected outcome (e.g. `"GetEnergy returns the representative value 511 keV"`).
5. `Evaluate(Function, Input, Description, Output, Truth)` — exact equality. `Output` and `Truth` share a single template parameter, so they must be the same type or the call fails to compile (`deduced conflicting types`). Cast the literal to match the getter's return type — `size_t` from `.size()`, `unsigned long` from a getter that returns `unsigned long`, etc. Use `EvaluateSize` for `.size()` results to avoid the cast.
6. `EvaluateNear(Function, Input, Description, Output, Truth, Tolerance)` — floating-point. Always supply an explicit tolerance. Never compare floats with `==`; always go through `EvaluateNear`.
7. `EvaluateTrue` / `EvaluateFalse` — boolean. All three string arguments are required. Prefer these over `Evaluate(..., true)` / `Evaluate(..., false)`.
8. `EvaluateSize(Function, Input, Description, Output, Truth)` — for `.size()`-style results; both arguments are `size_t`.
9. `EvaluateException<E>(Function, Input, Description, Callable)` — assert that calling `Callable` throws an exception of type `E` (or a derived type caught by reference-to-`E`). Pass a lambda.
10. `EvaluateFilesIdentical(Function, Input, Description, GeneratedFile, ReferenceFile)` — stream both files line by line; stop and report at the first mismatch. Use for reference-file comparisons in end-to-end tests.
11. `Summarize()` — call at the end of `Run()`, returns `void`. Return `Passed` separately.

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

## Stream and Round-Trip Tests

1. For output-format checks, capture to an `ostringstream`, wrap the string in `MString`, then use `Contains`, `BeginsWith`, `EndsWith`, or `Index` to assert presence, position, or absence of tokens:

```cpp
ostringstream Out;
Foo.Stream(Out);
MString S(Out.str().c_str());

Passed = EvaluateTrue("Stream()", "contains energy",
                     "Output contains the energy value 511",
                     S.Contains("511")) && Passed;
```

2. For round-trips, write with one instance and read into another. Compare every field that was set. Do not reuse the source instance as the destination.
3. Tolerances for parse / round-trip checks:
   - `1e-9` — full `double` round-trip with no formatting loss.
   - `1e-4` — values limited by the default `ostream` precision (6 significant digits).
   - `0.5` — integer fields written as integers and read back with `%d`.
4. After a successful round-trip test, also assert that the parser rejects malformed input — wrong leading keyword, missing fields, garbage — and that the rejection does not leave stale state in the receiver.

## Ownership and Pointer Tests

1. **Owning containers** (the parent class deletes its children at `Clear()` or destruction) take `new`-allocated children. Never pass a stack-allocated object — destruction will double-free. After `Clear()` or removal, verify the count has dropped.
2. **Non-owning containers** (the parent class keeps a pointer but does not delete) take stack-allocated or caller-managed pointers. After `Remove*`, verify the remaining pointers still match the originals.
3. If a `Remove*` accessor erases from the container without deleting the pointer, the test owns the pointer afterwards — delete it explicitly to avoid a leak.
4. Out-of-bounds accessors that print a diagnostic via the MEGAlib streams (`merr`, `mout`) must be wrapped in `DisableDefaultStreams()` / `EnableDefaultStreams()` so the expected noise does not clutter the test output:

```cpp
DisableDefaultStreams();
Passed = EvaluateTrue("GetFoo()", "out of bounds",
                     "GetFoo(99) returns nullptr for an out-of-range index",
                     Foo.GetFoo(99) == nullptr) && Passed;
EnableDefaultStreams();
```

## External Data Files and Fixtures

1. Locate committed test data via the `$MEGALIB` environment variable. Never hard-code absolute paths.
2. If `$MEGALIB` is unset in a unit test, skip the data-dependent sub-test with an `mout` notice and return `Passed` (true) — local builds must not fail because optional data is unavailable.
3. End-to-end regression tests treat the data as required: if `$MEGALIB` is set but the input file is missing, the test fails.
4. Test-generated fixture files (small files the test writes itself) live under the private randomized temporary root managed by `MUnitTest`, never in the source tree. Use the guarded `MUnitTest` helpers instead of constructing raw `/tmp/...` paths, adding `getpid()` suffixes, invoking shell cleanup commands, or removing files directly:

```cpp
const MString FixtureFile = GetTemporaryFileName("fixture.dat");
Passed = EvaluateTrue("WriteTextFile()", "fixture",
                     "The fixture file can be created",
                     WriteTextFile(FixtureFile, "...")) && Passed;

// ... use the file ...

Passed = EvaluateTrue("RemoveTemporaryFile()", "fixture",
                     "The fixture file can be removed",
                     RemoveTemporaryFile(FixtureFile)) && Passed;
```

5. Use `PrepareTemporaryDirectory()` when a sub-test needs an empty directory. It removes and recreates that directory below the private temporary root.
6. Temporary-root cleanup is automatic. Explicit cleanup may still be used and tested where it is relevant to the behavior under test.
7. When direct stream I/O is required instead of `WriteTextFile()` or `ReadTextFile()`, always assert `is_open()` immediately after opening — a failure should surface where it happens, not later as a confusing read error.

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
4. Use the randomized private temporary root managed by `MUnitTest` for filesystem work. Never touch the real user filesystem.
5. Do not make network calls in unit or integration tests.
6. Prefer freezing or injecting clocks instead of reading wall-clock time directly. If the public API itself measures real time and no injection seam exists, wall-clock tests are allowed only as a last resort. Such tests must use coarse, stable assertions and document why a fake clock could not be used.
7. Flaky tests are bugs. Fix or quarantine them. Do not add retries.
8. When a class owns dynamic resources, include at least one test that exercises reassignment or reinitialization, not just single-use success paths.
9. When testing filesystem helpers, cover recreation, idempotent removal, traversal rejection, sibling rejection, symlink escape rejection, and concurrent lazy initialization where applicable.
10. When testing thread safety, force contention and validate every result, not only the final iteration.

## Assertions and Diagnostics
1. Failure messages must show expected vs. actual without requiring a rerun. The `MUnitTest` helpers do this automatically — use them, not raw `if`/`return`.
2. One logical behavior per assertion block when practical. Group related assertions only when they describe the same behavior.
3. Sub-test method names: `TestCamelCase`, one logical concern per method (e.g. `TestDefaultConstruction`, `TestGettersSetters`, `TestStreamDat`). See section "MUnitTest API" for the `Evaluate*` argument-order convention.

## Mocking
1. Use real implementations for value types and pure functions.
2. Mock only at I/O and system boundaries (filesystem, network, clock, external processes).
3. Do not mock the class under test.
4. Do not mock ROOT or Geant4. Use real instances with minimal inputs.

## What Not to Test
1. Do not test ROOT, Geant4, or other third-party library behavior.
2. Do not test compiler or language behavior.

## Coding Conventions
1. Prefer `rg` for searching.
2. Do not use `cout` or `cerr`. Use `mout`, `merr`, `mlog`, or `mgui`.
3. When touching code that uses `cout`/`cerr`, switching it to `mout`/`merr` is allowed.
4. Follow the conventions in `CodingConventions.md` at the repo root. If a rule here conflicts with that file, that file wins.

## Test Hygiene
1. Silence expected noisy streams by calling `DisableDefaultStreams()` immediately before the noisy call and `EnableDefaultStreams()` immediately after. Both are protected methods on `MUnitTest`.
2. Do not leave streams disabled across a sub-test boundary. Re-enable on every return path.
3. Range, finiteness, and non-empty checks are fallback assertions only when no exact expected result is reasonably derivable.

## Harness
1. `make unittests` builds all unit tests.
2. A missing test binary is a failure.
3. A test binary exiting non-zero is a failure.
4. A silently skipped test is a failure.
5. Unit-test source files, binaries, and suite names use the `UT<ClassNameWithoutLeadingM>` convention, e.g. `MIsotope` -> `UTIsotope.cxx`, `bin/UTIsotope`, and `MUnitTest("UTIsotope")`.
6. In directories that use the standard unit-test makefile pattern, adding a `unittests/*.cxx` source file is sufficient to register a new unit test.
7. End-to-end and physics validation tests live outside the unit test harness and have their own runner. A change that touches simulation, reconstruction, or imaging must run the relevant end-to-end tier before merge.
