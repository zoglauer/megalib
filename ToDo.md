# To Do

## Make the MEGAlib error stream safe during static destruction

`merr` currently expands to the global `MStreams __merr` object. Global object
destruction order across source files is not defined, so code running during
static destruction could access `__merr` after its lifetime has ended.

Replace the destructible global error stream with a process-lifetime accessor:

```cpp
MStreams& GetErrorStream()
{
  static MStreams* Stream = new MStreams();
  return *Stream;
}
```

Define `merr` in terms of `GetErrorStream()` and update direct internal uses of
`__merr`, including `MAssert` and `MUnitTest`. The intentional allocation should
not be deleted: the stream must remain available until process termination, and
the operating system reclaims its memory.

Review this as an API change before implementation. External code directly
using the internal-looking `__merr` symbol would need to be updated and rebuilt.
Run `rg '__merr'` across the complete repository during the refactor so no
direct uses remain unnoticed.

Known affected files:

- `src/global/misc/inc/MStreams.h`
- `src/global/misc/src/MStreams.cxx`
- `src/global/misc/src/MAssert.cxx`
- `src/global/misc/src/MGlobal.cxx`
- `src/global/misc/inc/MUnitTest.h`
- `src/global/misc/src/MUnitTest.cxx`
- `src/cosima/src/MCCosima.cc`
- `src/global/misc/unittests/UTStreams.cxx`
- `src/global/misc/unittests/UTRotation.cxx`
- `src/cosima/unittests/UTCosimaInputSpectra.cc`

## Add reliable header dependency tracking to the build system

Changing the layout of `MUnitTest` rebuilt `libCommonMisc`, but some unit-test
binaries remained linked against the old class layout. The regular
`make unittests` invocation did not relink all affected binaries because the
Makefiles do not reliably track included headers.

Enable compiler-generated dependency files, for example with `-MMD -MP`, and
include the generated `.d` files from the relevant Makefiles. Verify that
touching a commonly included header such as `MUnitTest.h` causes every affected
unit-test binary to be relinked by a normal `make unittests` invocation.
