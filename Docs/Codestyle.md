CPP Codestyle
========

## General

#### Spaces vs. Tabs
Use oly spaces, 4 spaces for indent.
Do not use tabs in your code. 

#### Line Length
There is no limitation for line of text in your code. 
Try to meet your code 120 characters per line. This will help to easy look at your code in side-by-side mode. 



## Header Files

#### Header guard
Header files are guarder by `#pragma once`

#### Includes and Order of Includes
When listing includes, order them by increasing generality: your project's `.h`, other modules `.h`, other libraries `.h`, C++ library, C library.

Your project `.h` must be included with `#include "Your/Project/Path.h"` while external `.h` should be included with `#include <Library/Path.h>`

```cpp
//your project
#include "Your/Project/Path.h"

//common modules
#include <TArc/Core/ClientModule.h>

//DAVA library
#include <Base/BaseTypes.h>

//stl
#include <vector>
```

Within each section the includes should be ordered alphabetically. Note that older code might not conform to this rules and should be fixed when convenient.

```cpp
//common modules
#include <TArc/Core/ClientModule.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
```

All of a project's header files should be listed as descendants of the project's source directory without use of UNIX directory shortcuts `.` (the current directory) or `..` (the parent directory). For example, ../BaseTypes.h should be included as:

```cpp
// for Base/BaseTypes.cpp file
#include "Base/BaseTypes.h"
```

## Scoping


#### Namespaces
Intendation is not used inside namespaces

##### namespace DAVA
All DAVA Engine code (including modules, editors) is written inside `namespace DAVA`.

```cpp
namespace DAVA
{
class Foo
{
    void Bar();
};
}
```


##### Nested namespaces
Do not use nested namespaces, i.e. `namespace MyModule`. Just work inside `namespace DAVA`. Use class name prefixes instead of nested namespaces to avoid name collisions.
For example: `namespace DAVA{ class RenderObject; }` instead of `namespace DAVA { namespace Render { class Object; } }`.
The only exceptions are local(private) namespaces.

###### Local namespaces
Local(private) namespaces (are used to hide implementation details) must use `Details` suffix, i.e. `MyUtilsDetails`. Local(private) namespaces are used instead of unnamed namespaces to avoid collisions in unity builds.


#### Commented code
Remove commented code, we have revision control system for history. 

## Naming
Names are written in english, with no prefixes/suffixes and no odd shortenings. Name length and descriptiveness should be proportional to name scope: more descriptive names are used in bigger scopes.

```cpp
Image * CreateImageFromMemory(...); //long descriptive function name in public interface
```

```cpp
{
    void * imageData = image->Data(); //imageData is local variable name
    //plenty of code here
}
```

```cpp
for(int32 i = 0; i < width; ++i) //i is only used inside small loop
{
    Read(i);
}
```

#### Classes, functions
Camel notation begin with capital letter.

```cpp
class MyObject;
void Foo();
```

#### Utils classes
Additional functionality for specified class should be placed in separate files with class with suffix 'Utils'. 

```cpp
//Profiler.h
class Profiler{...}

//ProfilerUtils.h
class ProfilerUtils{...}
```

#### Variables
Camel notation begin with lower-case.

```cpp
MyObject myObjectInstance;
```
In case of names collision add underscore to local variable name.

```cpp
MyObject::MyObject(const String& name_)
:   name(name_)
{...}
```

#### Function objects
Function object is both function and variable. We just use the same naming rules as for variables: camel notation begin with lower-case.

```cpp
auto foo()[]{}
```

#### Static constants, defines, enums.
All capitals with underscores.

```cpp
static const int MAX_WIDTH = 100;
enum FrustumPlane 
{
    LEFT = 0,
    RIGHT
};
#define HAS_SPECULAR
```

## Types
Use types from `Base\BaseTypes.h` instead of built-in.
This include

* base types (int32, float32, etc.)
* standart library types (String, Vector, etc.)

## Classes
#### Class members initialization
In-class member initialization is preferred over all other methods.

```cpp
class MyObject
{
    int32 width = 0;
    int32 height = 0;
    Color color = Color::White();
};
```

#### Virtual functions
Either `virtual`, `override` or `final` keyword can be used in function declaration. **Do not** mix `virtual` with `override/final` keywords in one declaration.

```cpp
virtual void Foo(); //declared for the first time
void Bar() override; //overriding virtual function
```

#### Inline functions
Separate inline function definition from declaration and use `inline` keyword only in declaration. Declaration should be placed in the end of header file.

```cpp
class MyObject
{
    void Foo();
};
//...
inline void MyObject::Foo()
{
}
```
## CPP 11+ features
#### Autos
Usage of auto is limited to
##### lambda function type

```cpp
auto foo = [&](bool arg1, void* arg2)
{
    arg1 = *arg2;
}
```
and
##### shortening of long template types

```cpp
Map<FastName, SmartPointer<ObjectType>> map;
auto iter = map.begin();
```

## Unittests
Unittests files should be named 'MyClass.unittest' and placed along with .cpp files with tested code.

```
Profiler.cpp
Profiler.unittest
```

Python codestyle
========
Follow PEP8: https://www.python.org/dev/peps/pep-0008/
