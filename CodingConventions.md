# Coding conventions

for MEGAlib

## Automatic formatting

clang-format can be used to automatically format the code:

To format a file do the following:

1. Test it

clang-format MSubModuleDEEIntake.cxx > /tmp/Reformated.cxx
diff MSubModuleDEEIntake.cxx /tmp/Reformated.cpp

Check if everything looks OK.

2. Do it
clang-format -i MSubModuleDEEIntake.cxx

3. Recompile and test your code again

4. Do your pull request / check it in

## New classes

For new classes, copy and modify an existing one, or use the MModuleTemplate class as tempplate.

## Language

Use US English spelling in code, comments, and documentation, e.g., "Orthonormalize", "color", "behavior", "center", "normalize".

## Naming

### Classes

* Due to historic reasons, all classes start with M.
* Modules start with MModule
* Option GUI's start with MGUIOptions
* Expo (= data display) GUI's start with MGUIExpo

### Member functions

The naming follows the "upper camel case" convention, e.g.,
```
MVector, Clear, IsNull, SetMagThetaPhi
```

A name says what the function does or returns:
* A function which returns a stored or looked-up value is `Get...` (`GetRemainingTimeBudget`, `GetGeometry`), not a bare noun (`RemainingTimeBudget`). A function which computes a quantity from its arguments may be named after the quantity (`Mag`, `Dot`, `Median`, `ARM`).
* A function which returns a flag is `Is...`, `Has...`, or `Can...` (`IsExecutable`, `HasDisplay`); one which only checks something and reports it may be `Check...` or `Verify...`.
* A function which changes the object is `Set...`, `Add...`, `Remove...`, `Clear`, ...; one which does some work is a verb (`Execute`, `Simulate`).
* The name matches the result: a function which returns the path of an executable is not called `GetProgram`.

### Member variables

Member variables start with "m_", and the remainder of the name follow the "upper camel case" convention, e.g.,
```
m_X, m_DataPoint, m_IsNonZero
```

The name says what the value is, so that no comment has to explain it: `m_SimulationTime` and `m_SimulatedParticles`, not `m_Time` and `m_Generated`; `m_FileSuccessfullyRead`, not `m_Valid`. A flag is named for the fact it records.

### Enumerations

* Use `enum class`, defined at namespace scope above the class which uses it, named like a class with the prefix M, e.g., `MTestStatus`.
* The enumerators start with `c_` and follow the "upper camel case" convention; the name of the enumeration is not repeated in them. They are always used qualified, e.g., `MTestStatus::c_Pending`, not `c_StatusPending`.
```
enum class MTestingScope { c_UnitTests, c_All, c_EndToEnd };
```

### Variables in functions

Variables in functions *should* follow the "upper camel case" convention, e.g.,
```
X, DataPoint, IsNonZero
```

## Code organization

- Prefer class member functions over free functions. If a helper is tightly related to a class, make it a private, protected, or public member of that class as appropriate.
- Avoid namespace-scope helper functions when possible, including anonymous-namespace helpers. Use anonymous namespaces only when there is a clear reason the helper cannot reasonably be part of a class.
- Prefer C++ standard headers and C++ standard-library functionality over C or POSIX headers and functions when practical.
- Avoid adding C headers such as `<stdio.h>`, `<stdlib.h>`, or POSIX headers such as `<unistd.h>` and `<fcntl.h>` when a suitable C++ header and C++ mechanism exists.
- If a C or POSIX function is required because the C++ standard library does not provide equivalent semantics, document the reason briefly near the use.
- If new functionality is useful beyond the class it is written for (a second class needs it, or it would otherwise be copied), put it where all its users can reach it: in the lowest common base class that all of them share, or in a small dedicated class if there is none. Do not copy it into each class, and keep it a member, not a free function.
- In the source file, the member functions are defined in the order of their declaration in the header, with the constructors and the destructor first.
- Callbacks and signal handlers are static member functions of the class, not file-scope functions.
- A module has the directories `inc` (headers), `src` (implementations, and the programs), and `unittests`. A class or program belongs to the directory of the module which it serves: a base class for the end-to-end tests is in `src/endtoend`, not in a library below it.
- Small data classes which have no implementation of their own (only data members and short inline functions) may be defined in the header of the class which they serve, instead of in files of their own. They start with the prefix of their module (e.g. `ET` for the end-to-end tests) instead of `M`, to make clear that they are only a part of it.
- Group the declarations of the member functions in the header by purpose, in the order in which a reader needs them (for example environment, running programs, the chain of programs, reading the results, analysis), with one short comment line above each group.
- Group the includes under the comments `// Standard libs:`, `// POSIX libs:` (only if there are POSIX headers such as `<unistd.h>`, `<fcntl.h>`, `<sys/wait.h>`; directly after the standard libs), `// ROOT libs:`, and `// MEGAlib libs:`, spelled exactly like this (plural, with `libs`), and in the header and source files alike. `using namespace std;` follows the last of the system groups.
- Generalize when the second use appears, not speculatively. Do not widen a base class with something only one derived class needs.
- Search MEGAlib before you write a new function, class, or constant (`grep -rn "Keyword" src`, and look at the classes which fit the job), and use what exists; if it lacks something, extend it instead of copying it. Look at least at `MFile` (files, directories, paths, `$(MEGALIB)` expansion with `ExpandFileName`, `CreateDirectory`, `IsDirectory`, `ProgramExists`), `MSystem` (processes, the shell, `GetShellQuoted`), `MString`, `MGlobal` (constants), `MVector` and the other physics classes, and the readers (`MFileEventsSim`, `MFileEventsTra`). Use their classes and constants (`c_Pi`, `c_Rad`, `c_Deg`, `c_E0`, ...) instead of private structs, `M_PI`, hand-written parsers, or your own `std::filesystem`/`getenv` code for the same job.
- A helper which is useful beyond the class that needs it first belongs to the class that matches its job (a shell-quoting function is in `MSystem`, not in the end-to-end test base class), see also the rule above about the lowest common base class.
- Do not write a function which only renames or wraps a single call or expression and adds no meaning (a `UnitVector()` around one vector constructor); call it directly.
- Give types and helpers the smallest access level which works: what only derived classes use is `protected`, what only the class uses is `private`. A class does not export types only for a single outside user (e.g. a test); that user gets access through a derived class.

## C++ language use

- The code is C++17. Do not use features of later standards.
- Use C++ casts (`static_cast`, `dynamic_cast`, `const_cast`, `reinterpret_cast`) everywhere, never C-style casts.
- Do not use `auto` for simple types (`int`, `double`, `bool`, `MString`, ...) or ordinary classes. Use it for complex types where the type name carries no information: iterators, lambdas, template-heavy return types.
- Prefer smart pointers (`unique_ptr`, `shared_ptr`) in new code. Use `new` only where it is necessary, e.g. when ROOT or another API takes the ownership of the object.
- A function which hands an object over to the caller returns a smart pointer (`unique_ptr` for one owner, `shared_ptr` if the object is shared, e.g. the events of a file which several lists hold), never a raw pointer which the caller has to delete.
- Prefer the C++ threads, mutexes, and futures (`std::thread`, `std::mutex`, `std::async`) over the ROOT ones (`TThread`).
- Use `MString`, not `std::string`; `std::string` only where an API requires it.
- Counts, sizes, and loop indices are unsigned, not `size_t`: `unsigned int` by default, `unsigned long` where the count can exceed 4 billion (e.g. the number of simulated particles or events of a long run).
- End lines of streams with `endl`, not `"\n"`.

## Conditions and error handling

- Do not return or assign the result of a comparison or of a combination of conditions. Write an `if` which returns `true` or `false` (or sets the variable) explicitly, and compare every flag explicitly (`Flag == true`), also inside such conditions:
  ```cpp
  // Instead of: return system(Command.Data()) == 0;
  if (system(Command.Data()) != 0) return false;
  return true;

  // Instead of: Valid = Time > 0 && Generated > 0 && HasInitialInteraction;
  Valid = false;
  if (Time > 0 && Generated > 0 && HasInitialInteraction == true) Valid = true;
  ```
- Do not use the conditional operator `?:` to choose between values or to build return values. Write an `if` instead, so that each branch is visible and can be commented. A very short, plain selection of one of two simple values in an argument or a stream output (e.g. `Count == 1 ? "event" : "events"`) is the only exception.
  ```cpp
  // Instead of: return (Root == nullptr) ? MString("") : MString(Root) + "/resource/examples/geomega/special/Max.geo.setup";
  if (Root == nullptr) return "";
  return MString(Root) + "/resource/examples/geomega/special/Max.geo.setup";
  ```
- Do not rely on implicit truthiness. Write explicit comparisons such as `Flag == true`, `Flag == false`, `Pointer == nullptr`, and `Error.value() != 0`.
- For recoverable filesystem operations, prefer the `std::error_code` overloads. Print a contextual `merr` message before returning failure unless the failure is expected and intentionally ignored.

## Filesystem safety

- Every argument which is not a fixed word (a path, a file name, user input) is passed to the shell through `MSystem::GetShellQuoted()`, never pasted into a command line: a space, a quote, or a `$` in a path must neither break the command nor run something else. Only a glob (`*`) stays outside the quotes. Prefer a function of `MFile` or `MSystem` over a shell command for the same job (`MFile::Remove`, `MFile::CreateDirectory`, `MSystem::RunChildProcess`).

- Keep user-facing labels separate from filesystem-safe names. Validate path components instead of silently sanitizing them.
- Restrict destructive filesystem operations to a validated private root directory. Reject empty paths, traversal outside the root, sibling paths, and symlink escapes.
- If a mutex protects a non-obvious filesystem race, add a short comment describing the race, such as preventing concurrent teardown while file I/O is in progress.

## Comments

- Use doxygen-style comments (//!) for classes, member functions, and variables, including a brief description of the method's functionality.
  ```cpp
  //! Standard constructor giving x, y, z component of the data
  MVector(double X = 0.0, double Y = 0.0, double Z = 0.0);

  //! Flag indicating if the vector is zero
  bool m_IsZero;
  ```
- Document all classes and all member functions and variables in the header
- Repeat the description of a member function above its definition in the source file, so that the source file can be read on its own:
  ```cpp
  //! Return the number of data points
  unsigned int MData::GetNumberOfDataPoints() const
  {
    return m_DataPoints.size();
  }
  ```

- Use single-line comments (//) to explain logic within methods.
- For non-obvious logic inside methods, document the code in short step-by-step comments directly above the relevant block. The comments should describe the intent of each block, not restate every line.
  ```cpp
  bool MString::IsPositiveInteger() const
  {
    // Accept a non-negative base-10 integer with optional surrounding
    // whitespace and an optional leading plus sign. Reject empty strings,
    // whitespace-only strings, minus signs, decimal points, and trailing text.

    // Strip leading spaces.
    size_t Begin = 0;
    while (Begin < m_String.size() && isspace(static_cast<unsigned char>(m_String[Begin])) != 0) {
      ++Begin;
    }

    // Strip trailing spaces.
    size_t End = m_String.size();
    while (End > Begin && isspace(static_cast<unsigned char>(m_String[End-1])) != 0) {
      --End;
    }

    // Reject empty or whitespace-only strings.
    if (Begin == End) {
      return false;
    }

    // Allow one optional leading plus sign.
    if (m_String[Begin] == '+') {
      ++Begin;
      if (Begin == End) {
        return false;
      }
    }

    // The remaining content must be decimal digits only.
    for (size_t i = Begin; i < End; ++i) {
      if (isdigit(static_cast<unsigned char>(m_String[i])) == 0) {
        return false;
      }
    }

    return true;
  }
  ```
- Do not write directly to `cout` or `cerr` in MEGAlib code. Use the MEGAlib stream classes such as `mout`, `mlog`, `merr`, or `mgui` instead, so output can be redirected or disabled consistently.


## Comment wording

Comments in MEGAlib are short, factual labels. They name what the next block does or state a fact
about a variable, and nothing more. Half of the inline comments are 3 words or fewer, and few are
longer than one line.

### Header (doxygen) comments

- One line, no trailing period.
- Start with a plain imperative verb: `Return`, `Set`, `Get`, `Add`, `Check if`,
  `Create`, `Parse`, `Initialize`, `Clone`, `Dump`, `Validate`. Not `Returns` or `Sets`.
- Member variables get a noun phrase: `The number of ...`, `Number of ...`, `Flag indicating ...`,
  `List of ...`, `Name of ...`.
- Yes/no results: `Return true if ...` or `True if ...`.
- Constructors and destructors: `Default constructor`, `Default destructor`, `Standard constructor`,
  `Copy constructor`.
- Classes: `Class representing ...`.
- Units and allowed values go on the same line, in parentheses or after a colon.
  ```cpp
  //! Return the number of data points
  //! Set the axis in HEALPIX based on a target pixel size (deg)
  //! Get the binning mode: 0: fixed number of bins, 1: fixed cts per bin, 2: Bayesian block
  //! True if the clone template has already been written to the geant file
  //! Flag indicating if the vector is zero
  ```

### Inline comments in functions

- One short line above the block it describes. Fewer than 1 in 10 inline comments span several lines.
- Start with an imperative verb, or with `Now`, `First` or `Then` for a sequence:
  `Open the simulation file:`, `Normalize the response files:`, `Now update the short graph:`,
  `First we get the key...`
- End with a colon when the comment introduces the following block, with `...` when the step
  continues. Otherwise use no punctuation; a final period is rare.
- Use "We" to describe a condition or decision:
  `We only continue if the input was valid!`, `We have to distinguish two different cases:`
- Number the steps of longer procedures: `(1) ...`, `(2) ...`, or `Step 1: ...`.
- Short comments after code, on the same line, for units or a single fact: `// keV`, `// deg`,
  `// No spaces allowed`.
  ```cpp
  // Open the simulation file:
  // Loop and search best combination:
  // Wait until we have a first reconstructed event
  // We have a random coincidence if we have more than one event in the list:
  // Remove empty slots from the list - time consuming!
  ```

### Warnings and markers

- `Attention:` for pitfalls, followed by the fact:
  `Attention: Dir needs to be a unit vector`, `Attention: This event will be owned and destroyed by this class`,
  `Attention: Simple, but very inefficient algorithm!`
- `!` marks something the reader must not miss. Use one `!`, not several.
- `ToDo:` / `TODO:` for open work: `TODO: Change to AreCoplanar(...)`
- Empty function bodies in source files, constructors and destructors included, always contain `// Intentionally left blank` (also if a constructor only has an initializer list). The only exception is a one-line inline `{}` in a header.

### Rationale

Give a reason only when the code does not make it obvious. Keep it to a short clause after
` - `, not a new sentence:
```cpp
// Wait for a client to connect - we add a random amount to make sure that two instances can connect at the same time
// Clone this fit - the returned element must be deleted!
```

### Avoid

These patterns are nearly absent from the older code and should not be introduced:

| Avoid | Usage in the older code | Use instead |
|---|---|---|
| Full sentences ending in `.`, several per comment | 8% of inline comments end in `.` | One line, no period |
| Explanatory paragraphs above a single statement | half are ≤ 3 words | One line naming the step |
| ` -- ` or `—` between clauses | almost never | ` - ` or a new line |
| `ensure`, `whether`, `consistent with`, `explicitly`, `therefore`, `so that`, `unless`, `instead` | each in 0.1% of comments or fewer | `make sure`, `if`, `check if`, or drop the word |
| `Note:`, `NOTE:`, `IMPORTANT:`, `WARNING:` | almost never | `Attention:` |
| `Returns ...`, `Sets ...` in headers | 54 `Returns` vs. 1231 `Return` | `Return ...`, `Set ...` |
| Explaining why an alternative was not chosen, or which other functions share this logic | almost never | Leave it out |
| Backticks or function names like `Foo()` in the text | under 1% | Name the thing in words |


## Formatting

### 1. **General Whitespace Guidelines**
- **Consistency**: Be consistent in the use of whitespace across the codebase. This promotes readability and makes the code easier to follow.
- **Spaces Around Operators**: Always place a space around binary operators (e.g., `+`, `-`, `=`, `==`, `&&`, etc.).
  ```cpp
  m_X += W.m_X;  // Correct
  m_X+=W.m_X;    // Incorrect
  ```
- **Function Parameters++: There should be a space after a comma separating parameters in function declarations and calls.
  ```cpp
  MVector(double X, double Y, double Z);  // Correct
  MVector(double X,double Y,double Z);    // Incorrect
  ```

### 2. **Indentation**

- **Indentation Level**: Use 2 spaces for indentation (no tabs). This applies to all indented blocks of code such as loops, conditionals, and class methods.
- **Alignment**: Ensure that all lines within the same block are aligned, especially for multiple function arguments or complex expressions. For instance:
  ```cpp
    m_X = V.m_X;
    m_Y = V.m_Y;
    m_Z = V.m_Z;
  ```

### 3. **Blank Lines Between Functions and Code Blocks**

- **Between Functions**: Insert the following seperating blockbetween functions to separate them visually and enhance readability.
  ```cpp
  void SetX(double x)
  {
    m_X = x;
  }
  
  
  ////////////////////////////////////////////////////////////////////////////////
  
  
  void SetY(double y)
  {
    m_Y = y;
  }
  ```

- **At the End and Around main()**: The separating block is also used after the last function of the file, and before and after `main()`. A source file ends with the line `// <FileName>: the end...` above the final separator line:
  ```cpp
  }


  ////////////////////////////////////////////////////////////////////////////////


  int main()
  {
    ...
  }


  // ETCosimaToMimrecSpectrum.cxx: the end...
  ////////////////////////////////////////////////////////////////////////////////
  ```

- **Within Functions**: Use blank lines to separate logical blocks of code within a function. For example, separate initialization, computation, and return statements:
  ```cpp
    void SetMagThetaPhi(double mag, double theta, double phi)
    {
      double r = Mag();
      double t = Theta();

      m_X = r * sin(t) * cos(p);
      m_Y = r * sin(t) * sin(p);
      m_Z = r * cos(t);
    }
  ```

### 4. **Spaces Around Braces inside functions**

- **Braces**: The body of `if`, `else`, `for`, and `while` is always in braces, also if it is a single statement (`.clang-format` has `AllowShortIfStatementsOnASingleLine: Never`).
- **Opening Brace**: The opening brace { should be placed at the end of the line for function definitions, conditionals, loops, and class definitions - the exception are meber functions, where it is placed on a single new line

- **Closing Brace**: The closing brace } should be on its own line, aligned with the line where the corresponding opening brace appeared.
  ```cpp
  if (m_X == 0) {  // Correct
    // Do something
  }
  if (m_X == 0) // Incorrect
  {
    // Do something
  }
  ```

  ```cpp
    void Class::SomeFunction()
    {
      // Code logic here
    }
  ```

### 5. **Spaces After Keywords**

- **Space After Control Flow Keywords**: Always add a space after control flow keywords like if, else, for, while, switch, try, etc.
  ```cpp
  if (x > y) {  // Correct
    // do something
  }
  
  for (int i = 0; i < n; ++i) {  // Correct
    // loop body
  }
   ```
- **No Space Before Parentheses in Control Flow**: There should be no space between the keyword and the opening parenthesis.
  ```cpp
    if (x == y) {  // Correct
      // do something
    }

    while (x < 10) {  // Correct
      // loop body
    }
  ```


### 6. **Whitespace in Expressions**

- **Space Around Operators**: Always add a space before and after most binary operators (=, +, -, &&, etc.). The exce[tion are * and / in math expresions, to visualize the order of operations.
  ```cpp
  double x = 5 + 3*2;    // Incorrect
  double x = 5+3*2;      // Incorrect
  double x = 5 + 3 * 2;  // Correct
  ```

- **Function Calls**: No spaces between the function name and the opening parenthesis. Add spaces between arguments if needed, but avoid excessive whitespace.
  ```cpp
  SomeFunction(x, y);    // Correct
  SomeFunction (x, y);   // Incorrect
  ```

- **Unary Operators**: Do not add spaces between unary operators (e.g., ++, --, !, -).
  ```cpp
    m_X++;   // Correct
    ++m_X;   // Correct
    --m_X;   // Correct
    m_X ++;  // Incorrect
  ```

- **MEGAlib streams**: No white spaces before and after <<, >> (97% of the existing code does it this way)
  ```cpp
    merr<<"Var: "<<V<<endl;        // Correct
    merr << "Var: " << V << endl;  // Incorrect
  ```

### 7. **Trailing Whitespace**

- **Avoid Trailing Whitespace**: Do not leave trailing spaces at the end of lines. Trailing spaces can cause version control diffs to become unnecessarily cluttered.



## Bug fixes and the ChangeLog

- Every bug fix which could potentially change results (simulation output, reconstruction, responses, file contents, numbers printed for the user) must be mentioned in `doc/ChangeLog.txt` in the section of the upcoming version.
- Describe the effect (what was wrong and what changes), not the code change, so that users can judge whether old results are affected.
- This includes fixes which change the interpretation of input (e.g. units or ranges of keywords).
- Pure crash, message, or refactoring fixes do not need an entry.
- Add the entry in the same pull request / check-in as the fix.


## Good C++ practises

### 1. **Use Meaningful Variable Names**
   - Avoid using single letters or unclear variable names like `a`, `b`, or `temp`. Instead, choose descriptive names that clearly represent the purpose of the variable.
   - Example:  
     ```cpp
     unsigned int NumberOfComptonEvents = 0; // Good  
     unsigned int N = 0; // Bad
     ```

### 2. **Use Comments Wisely**
   - Label each logical block with one short line, and explain non-obvious logic and maths.
   - Do not comment every line, and keep the wording as described in "Comment wording".
   - Example:
     ```cpp
     // Calculate the Compton scatter angle from recoil electron Ee and scattered gamma-ray energy Eg
     m_Phi = acos(1 - c_E0 * (1 / m_Eg - 1 / (m_Ee + m_Eg)));
     ```

### 3. **Always Initialize Your Variables**
   - Uninitialized variables can lead to undefined behavior. Always give your variables an initial value.
   - Use nullptr for uninitialized pointers
   - Example:
     ```cpp
     unsigned int NumberOfComptonEvents = 0; // Initialize variable
     ```

### 4. **Use `const` and `constexpr`**
   - Use `const` to define variables that shouldn’t change, and `constexpr` for values known at compile-time.
   - Example:
     ```cpp
     const int MAX_SIZE = 100;  // Value won't change
     constexpr int square(int x) { return x * x; } // Compile-time function
     ```

### 5. **Avoid Using Magic Numbers**
   - Avoid hard-coding numbers (also known as *magic numbers*), as they reduce code clarity.
   - Use named constants instead and add explanations
   - Example:
     ```cpp
     const double XGuardringSize = 0.3;
     const double ActiveDetectorSize = 7.4;
     double Size = 2 * XGuardringSize + ActiveDetectorSize;
     ```

### 6. **Use Functions to Avoid Repetition**
   - If you find yourself repeating code, consider moving that code into a function. Functions promote code reusability and make debugging easier.
   - Example:
     ```cpp
     double AverageOffsetCalculation(double XOffset, double YOffset)
     {
       return 0.5*(XOffset + YOffset);
     }
     ```

### 7. **Don't Ignore Compiler Warnings**
   - Always pay attention to warnings from the compiler. They are there to help you catch potential problems before they turn into bugs.
   - Example:  
     If you get a warning about unused variables, remove or fix them.

### 8. **Don't use C arrays but C++ vectors**
   - Arrays in C++ are fixed in size and can be cumbersome. Use `std::vector` for dynamic arrays, as it automatically resizes.
   - Example:
     ```cpp
     vector<int> Numbers = { 1, 2, 3, 4, 5 };
     Numbers.push_back(6); // Adds 6 to the end
     ```

### 9. **Error Handling with Exceptions**
   - C++ allows for **exception handling** with `try`, `catch`, and `throw`. Use them to handle unexpected situations (e.g., division by zero, out-of-range indices).
   - Example:
     ```cpp
     try {
       int Result = Divide(x, y); // Some division function
     } catch (const std::exception& e) {
       merr<<"Error: "<<e.what()<<endl;
     }
     ```
   - The class MExceptions has a wide range of useful exceptions
   - When validating inputs before throwing, prefer the guard form
     ```cpp
     if (i >= Values.size()) {
       throw MExceptionIndexOutOfBounds(0, Values.size(), i);
     }

     return Values[i];
     ```
     over
     ```cpp
     if (i < Values.size()) return Values[i];
     throw MExceptionIndexOutOfBounds(0, Values.size(), i);
     ```
   - This keeps the error path explicit and the normal return path clean.

### 10. **Avoid Polluting the Top-Level Namespace**
   - Do not add global variables, file-scope helper functions, global utility functions, or other names in the top-level namespace unless there is a strong reason.
   - Prefer class member functions and variables or other narrowly scoped solutions so helper logic stays attached to the owning type and does not leak into the global namespace.


### More to follow
