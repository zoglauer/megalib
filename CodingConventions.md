# Coding conventions

by Andreas & ChatGPT

## Automatic formatting

Nuclearizer uses clang-format to automatically format the code:

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

### Member variables

Member variables start with "m_", and the remainder of the name follow the "upper camel case" convention, e.g.,
```
m_X, m_DataPoint, m_IsNonZero
```

### Variables in functions

Variables in functions *should* follow the "upper camel case" convention, e.g.,
```
X, DataPoint, IsNonZero
```

## Comments

- Use doxygen-style comments (//!) for classes, member functions, and variables, including a brief description of the method's functionality.
  ```cpp
  //! Standard constructor giving x, y, z component of the data
  MVector(double X = 0.0, double Y = 0.0, double Z = 0.0);

  //! Flag indicating if the vector is zero
  bool m_IsZero;
  ```
- Document all classes and all member functions and variables in the header

- Use single-line comments (//) to explain logic within methods.


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

- **cout etc.**: White spaces before and after <<, >> are OK but not enforced
  ```cpp
    cout<<"Var: "<<V<<endl;        // OK
    cout << "Var: " << V << endl;  // OK
  ```

### 7. **Trailing Whitespace**

- **Avoid Trailing Whitespace**: Do not leave trailing spaces at the end of lines. Trailing spaces can cause version control diffs to become unnecessarily cluttered.



## Good C++ practises

### 1. **Use Meaningful Variable Names**
   - Avoid using single letters or unclear variable names like `a`, `b`, or `temp`. Instead, choose descriptive names that clearly represent the purpose of the variable.
   - Example:  
     ```cpp
     unsigned int NumberOfComptonEvents = 0; // Good  
     unsigned int N = 0; // Bad
     ```

### 2. **Use Comments Wisely**
   - **Comments** should explain **why** something is done, not **what** is done (as the code itself should be self-explanatory).  
   - Use comments to clarify complex or non-obvious logic and maths, not for every line of code.
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
       cout<<"Error: "<<e.what()<<endl;
     }
     ```
   - The class MExceptions has a wide range of useful exceptions
   - 

### 10. **Avoid Global Variables**
   - Global variables make your code harder to debug and test. Use local variables or pass parameters to functions instead.

### More to follow
