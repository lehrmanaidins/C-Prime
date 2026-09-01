

<h1 align="center">C-Prime</h1>

<h3 align="center">Built for Critical Systems</h3>

<p align="center">
    <img src="C-Prime-Logo.svg" alt="C-Prime Logo" width="150">
</p>

--- 

### Work in Progress

&emsp;The C-Prime language is very early in the development cycle, so features may be incomplete or subject to change. Please provide feedback and report any issues you encounter.

---

### About

&emsp;C-Prime is a programming language designed for critical systems, emphasizing safety and reliability.

---

### Philosophy

#### Safety and Criticality

&emsp;C-Prime prioritizes safety and reliability, particularly in critical systems where errors can have severe consequences. The language enforces strict safety checks and provides the tools to easily follow best practices to minimize the risk of critical failures. The language also does not include features or allow code that could compromise safety or lead to undefined behavior.

#### Intention and Explicitness

&emsp;C-Prime is designed for intention; every feature and design choice is made to reflect the developer's intent clearly and unambiguously in the code. No viewer should be able to misinterpret the developer's intent with any part of the code. The main language feature supporting this philosophy is the strict type system, which ensures that newly declared types, derived from primitive and other existing types, are strictly their own distinct types and cannot be implicitly converted or confused with other types, even the underlying data type.

---

### Transpiler

&emsp;C-Prime includes a transpiler that converts C-Prime code into C++ code for execution on standard C++ compilers. This allows developers to leverage existing C++ toolchains while benefiting from the safety and reliability features of C-Prime.
