
# C-Prime Language Specification

## Lexical Structure

### Identifiers

&emsp;Identifiers in C-Prime are used to name variables, types, functions, and other user-defined items. Identifiers must begin with a letter (A-Z or a-z) or an underscore (_) and can be followed by letters, digits (0-9), or underscores. Identifiers are case-sensitive.

### Keywords

&emsp;C-Prime has a set of reserved keywords that cannot be used as identifiers. These keywords can be found in the [C-Prime Keywords](keywords.txt) document.

### Comments

&emsp;Single-line or end-line comments in C-Prime are denoted by `//` and multi-line comments are denoted by `/* */`.

```cprime
// This is a single-line comment
/* This is a multi-line comment
   that spans multiple lines */
```

### Literals

&emsp;Literals in C-Prime represent fixed values in the source code. They can be of various types, including integer literals, floating-point literals, character literals, and string literals.

```cprime
3 // Integer literal
3.14 // Floating-point literal
'a' // Character literal
"Hello, World!" // String literal
```

### Symbols

&emsp;C-Prime supports a variety of symbols for performing operations on variables and values.

```txt
// Arithmetic operators
+ // Addition
- // Subtraction
* // Multiplication
/ // Division
% // Modulus
** // Exponentiation

// Unary operators
+ // Unary plus
- // Unary minus

// Bicremental Operators
++ // Increment
-- // Decrement

// Relational operators
== // Equal to
!= // Not equal to
> // Greater than
< // Less than
>= // Greater than or equal to
<= // Less than or equal to

// Logical operators
&& // Logical AND
|| // Logical OR
! // Logical NOT

// Bitwise operators
& // Bitwise AND
| // Bitwise OR
^ // Bitwise XOR
~ // Bitwise NOT
<< // Left shift
>> // Right shift

// Assignment operators
= // Assignment

// Compound assignment operators
+= // Addition assignment
-= // Subtraction assignment
*= // Multiplication assignment
/= // Division assignment
%= // Modulus assignment
**= // Exponentiation assignment
&= // Bitwise AND assignment
|= // Bitwise OR assignment
^= // Bitwise XOR assignment
<<= // Left shift assignment
>>= // Right shift assignment

// Separators
, // Comma
. // Dot
; // Semicolon

// Grouping, Scope, and Indexing symbols
( // Left parenthesis
) // Right parenthesis
{ // Left brace
} // Right brace
[ // Left bracket
] // Right bracket

// Comment Markers
// // Single-line comment
/* */ // Multi-line comment
```

## Variables

### Naming Conventions

&emsp;It is recommended to use snake_case for variable names in C-Prime. This means using lowercase letters and underscores to separate words in a variable name.

```cprime
variable_name // Recommended naming convention for run-time initialized variables
```

&emsp;For constants that are used as compile-time constants, it is recommended to use UPPER_CASE with underscores to separate words.

```cprime
VARIABLE_NAME // Recommended naming convention for compile-time constants
```

### Declaration and Initialization

&emsp;Variables in C-Prime must be declared and initialized with a type and value before they can be used. The declaration and initialization specifies the variable's type, name, and initial value.

```cprime
ExampleType variable_name = initial_value;
```

&emsp;Variables in C-Prime cannot be declared without an initial value.

```cprime
ExampleType x; // ERROR (Cannot declare variables without an initial value)
```

### Assignment

&emsp;Variables in C-Prime can be assigned values using the assignment operator (`=`). The value assigned must be of the same type as the variable or a type that can be implicitly converted to the variable's type.

```cprime
mutable ExampleType y = initial_value; // y is assigned the value initial_value
y = new_value; // y is now assigned the value new_value
```

### Constants and Mutability

&emsp;Variables in C-Prime are constant or immutable by default. To declare a mutable variable, the `mutable` keyword must be used.

&emsp;Constant variables can optionally be declared using the `const` keyword, but this is not required as all variables are immutable by default.

```cprime
ExampleType x = initial_value; // Immutable variable
// const ExampleType x = initial_value; // Same as above, but explicitly declared as constant
x = new_value; // ERROR (Cannot modify immutable variable)

mutable ExampleType y = initial_value; // Mutable variable
y = new_value; // OK (Can modify mutable variable)
```

## Types

### Naming Conventions

&emsp;It is recommended to use PascalCase for type names in C-Prime. This means using uppercase letters for the first letter of each word in a type name.

```cprime
TypeName // Recommended naming convention for user-defined types
```

&emsp;The only exception to this is for built-in primitive types, which are recommended to be written only in lowercase letters (this rule is more for language devs since the user, normally is unable to create primitive types).

```cprime
typename // Recommended naming convention for primitive types
```

### Type System

&emsp;C-Prime has a strong and static typing system. The type system includes primitive types, domain types, and composite types. The type system enforces type safety and prevents type errors at compile time.

### Primitive Types

&emsp;C-Prime supports a set of primitive types that are built into the language. These include integer types, floating-point types, character types, and boolean types.

```txt
uint8, uint16, uint32, uint64 // Unsigned integer types
int8, int16, int32, int64 // Signed integer types
float32, float64 // Floating-point types
char8, char16, char32 // Character types
bool // Boolean type
```

&emsp;C-Prime users are highly encouraged to define new types from primitive types in order to use them in application code. So much that C-Prime forces you to use the `primitive` keyword when declaring variables of primitive types. This is to ensure that the user is aware that they are using a primitive type and to encourage the use of domain types instead.

```cprime
uint32 x = 0; // ERROR (Cannot declare primitive types without the `primitive` keyword)
primitive uint32 x = 0; // OK (Declares a variable of primitive type uint32)
```

### Domain Types

&emsp;Domain Types are a C-Prime feature where the user can define new types from primitive types in order to use them in application code without the `primitive` keyword. Unique from other languages, domain types are not just aliases for primitive types, but are distinct types that cannot be implicitly converted to other domain or primitive types. This allows for a more robust type system that can prevent many common programming errors.

&emsp;The only implicit conversion allowed to a domain type is during initialization, where a literal value (not a variable) that is compatible with the underlying primitive type can be used to initialize a domain type variable.

```cprime
type Altitude : primitive uint32;
type Distance : primitive uint32;

Altitude altitude = 0; // OK (0 is a literal value compatible with the underlying primitive type uint32)

primitive uint32 x = 1000;
Altitude altitude = x; // ERROR (Cannot implicitly convert a variable of primitive type uint32 to a domain type Altitude)
Altitude altitude = Altitude(x); // OK (Explicitly constructs a domain type Altitude from a variable of primitive type uint32)

Distance distance = altitude; // ERROR (Cannot implicitly convert between domain types)
Distance distance = Distance(altitude); // ERROR (No valid constructor for Distance that can accept a value of type Altitude)
Distance distance = Distance(underlying_cast<primitive uint32>(altitude)); // OK (Explicitly constructs a domain type Distance from the underlying primitive type of Altitude)
```

### Type Conversion

&emsp;C-Prime supports explicit type conversion between compatible types using built-in functions. This allows for converting values from one type to another when necessary.

```cprime
T(value) // Constructs a value of type T from the given value, raises an error if there is no valid constructor for T that can accept the given value
widen_cast<T>(value) // Widening conversion
narrow_cast<T>(value) // Narrowing conversion (potentially lossy); unsafe
underlying_cast<T>(value) // Used on domain types to convert to their underlying primitive type; will raise an error if T is not the underlying type of the domain type of value
reinterpret_cast<T>(value) // Reinterpretation conversion; bit-level reinterpretation; unsafe
```

```cprime
primitive uint16 x = 1000;
primitive uint32 y = widen_cast<primitive uint32>(x); // OK (Widening conversion)
primitive uint16 z = narrow_cast<primitive uint16>(y); // ERROR (Narrowing conversion, unsafe)
unsafe primitive uint16 z = narrow_cast<primitive uint16>(y); // OK (Narrowing conversion, but potentially lossy)

type Altitude : primitive uint32;
Altitude altitude = 1000;
primitive uint32 n = underlying_cast<primitive uint32>(altitude); // OK (Converts Altitude to its underlying primitive type)
primitive uint16 m = underlying_cast<primitive uint16>(altitude); // ERROR (Cannot convert Altitude to uint16, as it is not the underlying type of Altitude)

primitive uint32 a = 1065353216;
primitive float32 b = reinterpret_cast<primitive float32>(a); // ERROR (Unsafe reinterpretation conversion, must be marked as unsafe)
unsafe primitive float32 b = reinterpret_cast<primitive float32>(a); // OK (Unsafe reinterpretation conversion)
```

### Composite Types

#### Tuples

&emsp;C-Prime tuples are a fixed-size collection of elements of potentially different types. Tuples are defined with parentheses containing the types of the elements.

```cprime
type Point : (primitive uint32, primitive float32); // Defines a tuple with a uint32 and a float32 element
```

&emsp;Tuple literals are defined with parentheses containing comma-separated values of the corresponding types.

```cprime
Point point = (10, 3.14); // Declares and initializes a Point tuple with values 10 and 3.14
```

&emsp;Tuple elements can be accessed using the tuple variable, the `[]` operator, and the index of the element.

```cprime
primitive uint32 x = point[0]; // Accesses the first element of the Point tuple
primitive float32 y = point[1]; // Accesses the second element of the Point tuple
```

&emsp;Tuples can be unpacked into individual, named variables.

```cprime
(
    primitive uint32 x,
    primitive float32 y
) = point; // Unpacks the Point tuple into individual variables x and y

primitive uint32 z = x; // Unpacked variables can now be used independently
```

#### Arrays

&emsp;C-Prime arrays are a fixed-size collection of elements of the same type. Arrays are declared with a type followed by square brackets containing the size of the array.

&emsp;Arrays are zero-indexed, meaning the first element is accessed with index 0, the second element with index 1, and so on.

&emsp;Arrays must be initialized with starting values for all elements, or they can be initialized with a default value using the `array_fill` function.

```cprime
primitive uint32[5] numbers; // ERROR (Cannot declare an array without initializing all elements)
primitive uint32[5] numbers = {0, 0, 0, 0, 0}; // Declares and initializes an array of 5 uint32 elements with the value 0
primitive uint32[5] numbers = array_fill<primitive uint32>(5, 0); // Declares and initializes an array of 5 uint32 elements with the value 0
```

```cprime
primitive uint32[2][2] matrix = {{1, 2}, {3, 4}}; // Declares and initializes a 2D array of 2x2 uint32 elements
primitive uint32[2][2] matrix = array_fill<primitive uint32>(2, array_fill<primitive uint32>(2, 0)); // Declares and initializes a 2D array of 2x2 uint32 elements with the value 0
primitive uint32[2][2] matrix = array_fill<primitive uint32>((2, 2), 0); // Declares and initializes a 2D array of 2x2 uint32 elements with the value 0
```

```cprime
primitive uint32[3] numbers = {1}; // ERROR (Cannot partially initialize an array without specifying all elements)
primitive uint32[3] numbers = {1, 2, 3, 4}; // ERROR (Cannot initialize an array with more elements than its size)
```

#### Structs

&emsp;Structs or structures, declared with the `struct` reserved word, are a way to define a set of related variables with a shared name/type.
```cprime
struct State {
    Position position;
    Velocity velocity;
    Acceleration acceleration;
};
```

&emsp;Struct members can be accessed using the struct variable, the `.` operator, and the name of the underlying variable.

```cprime
State state = {
    position = 0,
    velocity = 0,
    acceleration = 0
};
Position position = state.position;
```

#### Enums

&emsp;Enums or enumerations, declared with the `enum` reserved word, are a way to define a named set of named values.

```cprime
enum Status {
    OK,
    WARNING,
    ERROR
};
```

&emsp;The C-Prime compiler will auto define the underlying type of `enum` values unless explicitly defined by the user.

```cprime
enum Status : primitive uint8 {
    OK,
    WARNING,
    ERROR
};
```

&emsp;The underlying values of `enum`'s are undefined unless defined by the user.

```cprime
enum Status : primitive uint8 {
    OK = 0,
    WARNING = 1,
    ERROR = 2
};
```

### Memory

#### References

&emsp;References in C-Prime are declared with the `reference` keyword with the referenced type inside of angled brackets `<T>` and are a way to represent an alias to an existing variable.

&emsp;A reference is constructed using the `reference(T value)` constructor, where `T` is the type of the value being referenced and will return a reference of type `reference<T>`.

```cprime
primitive uint32 a = 10;
reference<primitive uint32> b = reference(a); // b is a reference to a
```

&emsp;A reference must always be initialized when it is declared, and it cannot be null. This ensures that a reference always refers to a valid variable. 

```cprime
reference<primitive uint32> a; // ERROR (References must be initialized)
```

&emsp;By default, references in C-Prime are immutable; meaning the cannot be assigned to refer to a new variable after initialization. References can be made mutable by using the `mutable` keyword, just like other variables.

```cprime
primitive uint32 a = 10;
primitive uint32 b = 20;

reference<primitive uint32> r1 = reference(a); // `r1` is an immutable reference to `a`
r1 = reference(b); // ERROR (`r1` cannot be reassigned because it is an immutable reference)

mutable reference<primitive uint32> r2 = reference(a); // `r2` is a mutable reference to `a`
r2 = reference(b); // `r2` can be reassigned to refer to a new variable because `r2` is mutable
```

&emsp;The type a reference declared to refer to determines whether the value of the referenced variable can be modified through that reference. With `reference<mutable T>`, the referenced variable can be modified, whereas for a `reference<T>`, the referenced variable cannot be modified through the reference. Using `reference<mutable T>` is valid only when the referenced variable itself is mutable.

```cprime
mutable primitive uint32 a = 10;
primitive uint32 b = 10;

reference<mutable primitive uint32> r = reference(a);
r = 20; // Modifies the value of the referenced variable `a`

reference<mutable primitive uint32> r3 = reference(b); // ERROR (Cannot create a mutable reference to an immutable variable)

reference<primitive uint32> r2 = reference(a);
r2 = 30; // ERROR (Cannot modify the value of the referenced variable through a reference to an immutable type)

```

&emsp;A reference can be declared to refer to either a immutable variable, even if the variable itself is mutable, this means that the reference cannot be used to modify the value of the variable it refers to.

#### Pointers

&emsp;Pointers in C-Prime are declared with the `pointer` keyword with the pointed-to type inside of angled brackets `< >`. Unlike references, pointers are an unsafe, low-level construct and can only be declared and initialized in an unsafe context.

&emsp;A value is converted to a pointer using the `pointer(value)` function, mirroring how `reference(value)` is used to create a reference, and will return a pointer of type `pointer<T>`, where `T` is the type of the referenced value.

```cprime
mutable primitive uint32 a = 10;
unsafe pointer<primitive uint32> p = pointer(a); // OK ('unsafe' marks the declaration itself)

unsafe {
    mutable pointer<primitive uint32> p2 = pointer(a); // OK (declared inside an `unsafe` block)
}

pointer<primitive uint32> p3 = pointer(a); // ERROR (pointer types must be declared 'unsafe' or used inside an unsafe block)
```

## Functions

### Naming Conventions

&emsp;It is recommended to use camelCase for function names in C-Prime. This means using lowercase letters for the first letter of the function name and uppercase letters for the first letter of each subsequent word.

```cprime
functionName() // Recommended naming convention for functions
```

### Function Declaration

&emsp;C-Prime functions are declared with the `function` reserved word, followed by the function name, and the call operator.

```cprime
function foo();
```

### Function Definition

&emsp;C-Prime function definitions are placed after the function declaration and are enclosed in curly braces (`{`, `}`). The function body contains the statements that define the behavior of the function.

```cprime
function foo() {
    // Function body goes here
}
```

### Function Parameters

&emsp;Arguments and return types can be of any type, including primitive types, domain types, and composite types. If a value is a primitive type, it must be declared with the `primitive` keyword.

```cprime
function foo(primitive uint32 a);
function bar(Altitude a, primitive uint32 b);
```

&emsp;A function that takes no arguments can be declared with no parameters or with the `void` keyword.

```cprime
function foo();
function foo(void);
```
> Both of the above function declarations are equivalent and declare a function that takes no arguments.

### Function Return Values

&emsp;Function return types are defined after the `->` operator in the function declaration. 

&emsp;Functions can return primitive types, domain types, and composite types. If a value is a primitive type, it must be declared with the `primitive` keyword.

```cprime
function foo() -> primitive uint32;
function bar() -> Altitude;
function baz() -> (primitive uint32, Altitude);
```

&emsp;Functions that return no value can be declared with no return type or with the `void` keyword.

```cprime
function foo();
function foo() -> void;
function foo(void);
function foo(void) -> void;
```
> All of the above function declarations are equivalent and declare a function that takes no arguments and returns no value.

### Function Return Statements

&emsp;Functions can return values using the `return` reserved word followed by the value to be returned. The value must be of the same type as the function's return type. The returned type cannot be implicitly converted to the function's true return type.

```cprime
function foo() -> primitive uint32 {
    return 42; // OK (Returns a value of type primitive uint32)
}

function bar() -> Altitude {
    return Altitude(1000); // OK (Returns a value of type Altitude)
    // return 1000; // ERROR (Cannot implicitly convert a value of type primitive uint32 to a domain type Altitude)
}

function baz() -> (primitive uint32, Altitude) {
    return (42, Altitude(1000)); // OK (Returns a tuple of type (primitive uint32, Altitude))
    // return (42, 1000); // ERROR (Cannot implicitly convert a value of type primitive uint32 to a domain type Altitude)
}
```

### Function Calls

&emsp;Functions can be called using the function name followed by parentheses containing the arguments to be passed to the function. The arguments must be of the same type as the function's parameters and cannot be implicitly converted to the function's true parameter types.

```cprime
function foo(primitive uint32 a) -> primitive uint32 {
    return a + 1;
}
```

```cprime
primitive uint32 x = 42;
primitive uint32 y = foo(x); // OK (Calls the function foo with a value of type primitive uint32)
primitive uint16 z = foo(x); // ERROR (Cannot implicitly convert a value of type primitive uint32 to a value of type primitive uint16)

primitive uint16 u = 1000;
primitive uint32 v = foo(u); // ERROR (Cannot implicitly convert a value of type primitive uint16 to a value of type primitive uint32)
```

### Function Pass-by-Value and Pass-by-Reference

#### Pass-by-Value

&emsp;By default, C-Prime functions pass arguments by value. This means that a copy of the argument is made and passed to the function. Changes made to the parameter inside the function do not affect the original argument.

```cprime
function increment(primitive uint32 a) -> primitive uint32 {
    a += 1; // Increments the value of a
    return a; // Returns the incremented value
}
```

```cprime
primitive uint32 x = 42;
primitive uint32 y = increment(x); // OK (Calls the function increment with a value of type primitive uint32)

primitive bool is_equal = (x == y); // is_equal is false because x is still 42 and y is 43
```

#### Pass-by-Reference

&emsp;To pass an argument by reference, the `reference` keyword must be used in the function parameter declaration. This allows the function to modify the original argument.

```cprime
function increment(reference<mutable primitive uint32> a) -> primitive uint32 {
    a += 1; // Increments the value of a
    return a; // Returns the incremented value
}
```

```cprime
mutable primitive uint32 x = 42;
primitive uint32 y = increment(x); // OK (Calls the function increment with a value of type primitive uint32)

primitive bool is_equal = (x == 43); // is_equal is true because x has been modified to 43 by the increment function
```

### Function Return-by-Value and Return-by-Reference

#### Return-by-Value

&emsp;By default, C-Prime functions return values by value. This means that a copy of the return value is made and returned to the caller. Changes made to the returned value do not affect the original value.

```cprime
function get_value(primitive uint32 a) -> primitive uint32 {
    return a; // Returns a copy of the argument
}
```

```cprime
primitive uint32 x = 42;
primitive uint32 y = get_value(x); // OK (Returns a copy of x)

y += 1; // Modifies y, but x remains unchanged
primitive bool is_equal = (x == 42); // is_equal is true because x has not been modified
```

&emsp;A function can be passed a reference to a variable, but return a copy of the referenced value.

```cprime
function get_value_from_reference(reference<primitive uint32> a) -> primitive uint32 {
    return a; // Returns a copy of the referenced value
}
```

```cprime
primitive uint32 x = 42;
primitive uint32 y = get_value_from_reference(x); // OK (Returns a copy of the referenced value)

y += 1; // Modifies y, but x remains unchanged
primitive bool is_equal = (x == 42); // is_equal is true because x has not been modified
```

#### Return-by-Reference

&emsp;To return a value by reference, the `reference` keyword must be used in the return type declaration.

```cprime
function get_reference(reference<primitive uint32> a) -> reference<primitive uint32> {
    return a; // Returns a reference to the argument
}
```

```cprime
primitive uint32 x = 42;
reference<primitive uint32> y = get_reference(x); // OK (Returns a reference to x)

y += 1; // Modifies x through the reference y
primitive bool is_equal = (x == 43); // is_equal is true because x has been modified through y
```

&emsp;Only a function's arguments that are passed by reference can be returned by reference. Returning a reference to a local variable is not allowed, as it would result in a dangling reference.

```cprime
function get_local_reference() -> reference<primitive uint32> {
    primitive uint32 local = 42;
    return local; // Error: Returning a reference to a local variable is not allowed
}
```

### Function Overloading

&emsp;C-Prime supports function overloading, which allows multiple functions to have the same name but different parameter types or numbers of parameters. The compiler determines which function to call based on the arguments provided.

```cprime
function add(primitive uint32 a, primitive uint32 b) -> primitive uint32 {
    return a + b; // Adds two uint32 values
}

function add(primitive uint16 a, primitive uint16 b) -> primitive uint16 {
    return a + b; // Adds two uint16 values
}
```

### Function Templates

&emsp;C-Prime supports function templates, which allow the creation of generic functions that can operate on different types. Templates are defined using the `template` keyword followed by type parameters in angle brackets (`< >`).

```cprime
template <type T>
function add(T a, T b) -> T {
    return a + b; // Adds two values of type T
}

template <type T, type U>
function add(T a, U b) -> T {
    return a + T(b); // Adds two values of different types, converting U to T
}
```

### Function Contracts

&emsp;C-Prime supports function contracts, which allow the specification of preconditions and postconditions for functions. Contracts are defined using the `requires` and `ensures` keywords.

&emsp;For use in the `ensures` clause, the return value of the function can be named after the `->` operator and the return type declaration. This allows the user to refer to the return value in the postcondition. This name is only valid in the `ensures` clause and cannot be used in the function body.

```cprime
function increment(primitive uint32 a) -> primitive uint32 b
    requires a < uint32_max; // Precondition: a must be less than the maximum value of uint32
    ensures b == a + 1; // Postcondition: b must be equal to a + 1
{
    return a + 1;
}
```

&emsp;C-Prime contracts can be marked either `debug` or `release`. Debug contracts are only checked in debug builds, while release contracts are checked in both debug and release builds. By default, contracts are marked as `debug` if no contract marker is provided.

```cprime
function increment(primitive uint32 a) -> primitive uint32 b
    release requires debug a < uint32_max; // Precondition: checks condition in both debug and release builds
    debug ensures release b == a + 1; // Postcondition: only checks condition in debug builds
    ensures release b == a + 1; // Same as above; debug only if no contract marker is provided
{
    return a + 1;
}
```

## Control Flow

### Conditional Statements

&emsp;Pretty much all control flow statements in C-Prime are similar to those in C and C++. The `if` statement allows for conditional execution of code blocks based on a boolean expression.

```cprime
if (condition) {
    // Code block executed if condition is true
} else if (another_condition) {
    // Code block executed if another_condition is true
} else {
    // Code block executed if condition is false
}
```

### Switch Statements

&emsp;Switch statements are declared with the `switch` reserved word, followed by a value to be evaluated. The `case` reserved word is used to define different cases for the value, and the `default` reserved word is used to define a default case if none of the specified cases match.

```cprime
switch (value) {
    case 1:
        // Code block executed if value is 1
    case 2:
        // Code block executed if value is 2
    case 3:
        // Code block executed if value is 3
    default:
        // Code block executed if value does not match any case
}
```

### Loops

&emsp;Loops in C-Prime can be broken down into main "categories": a `for` loop, a `foreach` loop, a `while` loop, and a `do while` loop, and an endless `loop`. The `for` loop is used for iterating over a range of values, the `foreach` loop is used for iterating over elements in a collection, the `while` loop is used for executing a block of code while a condition is true, and the `do while` loop is similar to the `while` loop but guarantees that the block of code will be executed at least once.

#### Loop Limit

&emsp;One unique feature of C-Prime loops is that every loop has a hard coded `limit` that can be defined by the user. This limit is a compile-time constant that is used to prevent infinite loops and ensure that loops terminate after a certain number of iterations. If a loop exceeds its limit, the loop will terminate.

&emsp;The `limit` statement is declared with the `limit` reserved word, followed by a compile-time constant value. The `limit` statement is an optional, but highly recommended, feature of C-Prime loops. If no `limit` statement is provided, the loop will use a default limit `uint32_max` for the loop limit.

&emsp;The `limit` statement is declared as `limit (compile_time_constant)`, where `compile_time_constant` is a compile-time constant value that is used to limit the number of iterations of the loop. The parentheses surrounding the `compile_time_constant` are not required, but are recommended for clarity, especially when the `compile_time_constant` is an expression.

&emsp;The `limit` statement is created to easily address [The Power of 10 : Rule 2](https://en.wikipedia.org/wiki/The_Power_of_10:_Rules_for_Developing_Safety-Critical_Code#:~:text=Give%20all%20loops%20a%20fixed%20upper%20bound.%20It%20must%20be%20trivially%20possible%20for%20a%20checking%20tool%20to%20prove%20statically%20that%20the%20loop%20cannot%20exceed%20a%20preset%20upper%20bound%20on%20the%20number%20of%20iterations.%20If%20a%20tool%20cannot%20prove%20the%20loop%20bound%20statically%2C%20the%20rule%20is%20considered%20violated.)

```cprime
<loop syntax> limit (compile_time_constant) {
    // Code block executed for each iteration of the loop
}
```

#### For Loops

```cprime
for (initialization; condition; update) limit (compile_time_constant) {
    // Code block executed for each iteration of the loop
}
```

```cprime
for (primitive uint32 i = 0; i < 10; i++) {
    println(i); // Prints numbers 0 to 9
}

for (primitive uint32 i = 0; i < 10; i++) limit (10) {
    println(i); // Prints numbers 0 to 9, but guarantees that the loop will not exceed 10 iterations
}
```

#### Foreach Loops

```cprime
foreach (DataType element in collection) {
    // Code block executed for each element in the collection
}
```

```cprime
foreach (DataType element in collection) limit (compile_time_constant) {
    // Code block executed for each element in the collection
}
```

#### While Loops

```cprime
while (condition) {
    // Code block executed while the condition is true
}
```

```cprime
while (condition) limit (compile_time_constant) {
    // Code block executed while the condition is true
}
```

#### Do-While Loops

```cprime
do {
    // Code block executed at least once and then while the condition is true
} while (condition);
```

```cprime
do {
    // Code block executed at least once and then while the condition is true
} while (condition) limit (compile_time_constant);
```

#### Endless Loops

&emsp;Most other languages provide endless loops using constructs like `while (true)` or `for (;;)` which run indefinitely until explicitly broken out of. C-Prime provides the explicit `loop` construct for endless loops.

&emsp;A `loop` can only be terminated using `break` or by reaching a `limit` if specified.

&emsp;They are also useful when wanting to create a loop that runs for a specific number of iterations where you do not need to access the `i` variable, by specifying a `limit` on the `loop` construct.

```cprime
loop {
    // Code block executed indefinitely until broken out of
}
```

```cprime
loop limit (compile_time_constant) {
    // Code block executed indefinitely until broken out of or the limit is reached
}
```

```cprime
loop limit (10) {
    // Code block executed 10 times (equivalent to `for (primitive uint32 i = 0; i < 10; i++)`)
}
```

#### Break and Continue

&emsp;C-Prime provides the `break` and `continue` statements to control the flow of loops.

&emsp;- `break` immediately terminates the innermost loop. \
&emsp;- `continue` skips the remaining code in the current iteration and proceeds to the next iteration of the loop.

## Built-in Functions

### Print Functions

&emsp;C-Prime provides built-in functions for printing output to the console. The functions are `print` and `println`. `print` outputs the text without a newline at the end, while `println` outputs the text followed by a newline.

```cprime
print("Hello, World!");   // Prints "Hello, World!" without a newline
println("Hello, World!"); // Prints "Hello, World!" followed by a newline
```
