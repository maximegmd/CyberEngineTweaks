# Code Guidelines

## Language

We are using C++20, any C++20 feature supported by vs2019 is allowed.

Please try to use templates responsibly, we don't want compilation times to explode and to deal with bloated binaries.

Try to follow SRP as much as possible, a huge class containing tons of functionnalities is not better than many small components, it's easier to re-use them and to extend.

## Naming

### Variables

The first letter is lower case, other words must start with an upper case : ``someVariableName``.

Function arguments must be prefixed with an ``a`` for 'argument' : ``aFunctionArgument``.

Const variables must be prefixed with a ``c`` for 'const' : ``const int cSomeInt;``

Pointers must be prefixed with a ``p`` for 'pointer' : ``int* pSomePointer``.

The rules above must be used together for example ``void SomeFunc(const int* acpSomeArgument)``.

Static variables must be prefixed with ``s_`` : ``static int s_someInt;``.

Global variables must be prefixed with ``g_`` : ``extern int g_someGlobalInt;``.

### Classes

Class names must start with an upper case : ``class SomeClass``.

Class attributes must be prefixed with ``m_`` and must use the same rules as variables : ``int* m_pSomeMemberPointer;``.

### Functions

All functions must start with an upper case : ``void SomeFunc();``.

## Generalities

Names must be self explanatory, ``size_t a;`` is not acceptable, ``size_t incomingPacketCount;`` is good.

``auto`` is allowed when dealing with long names, it is not accepted for primitive types as we don't want the compiler to give us a signed int when we are using it as unsigned.

Don't use java style blocks, a ``{`` needs to be on a new line.

Don't use exceptions, don't use STL code that can throw, use the nothrow version if available or the unsafe version.
