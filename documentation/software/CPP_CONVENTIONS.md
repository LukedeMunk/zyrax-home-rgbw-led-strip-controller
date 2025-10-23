# C/C++ Coding Conventions

This document secures the code quality of C/C++ modules by describing the coding conventions.

> When the code doesn't fulfill the guidelines, the code cannot be integrated in the project.

## Naming conventions

| Object Name               | Notation   | Length | Prefix | Suffix | Char Mask   | Example            |
|---------------------------|------------|-------:|--------|--------|-------------|--------------------|
| Namespace name            | kebab-case |     50 | No     | No     | [A-z]       | `example-space`    |
| Class name                | PascalCase |     50 | No     | No     | [A-z][0-9]  | `UserAccount`      |
| Constructor name          | PascalCase |     50 | No     | No     | [A-z][0-9]  | `UserAccount`      |
| Method name               | camelCase  |     50 | No     | No     | [A-z][0-9]  | `doThis`           |
| (RT) Task name            | camelCase  |     50 | `task` | No     | [A-z][0-9]  | `taskDoThis`       |
| Method arguments          | camelCase  |     50 | No     | No     | [A-z][0-9]  | `isDone`           |
| Variables                 | camelCase  |     50 | No     | No     | [A-z][0-9]  | `userIndex`        |
| Constants name            | MACRO_CASE |     50 | No     | No     | _[A-z][0-9] | `MINIMUM_AGE`      |
| Field name public         | camelCase  |     50 | No     | No     | [A-z][0-9]  | `normalVariable`   |
| Field name private        | camelCase  |     50 | `_`    | No     | _[A-z][0-9] | `_privateVariable` |
| Enum type name            | PascalCase |     50 | No     | No     | [A-z]       | `States`           |
| Structure type name       | PascalCase |     50 | No     | No     | [A-z]       | `UserInformation`  |

## File layout

> Use this layout for every C/C++ and header file.

1. File header:

```cpp
/******************************************************************************/
/*
 * File:    [name_of_file].cpp
 * Author:  [author]
 * Version: [X].[X].[X]
 * 
 * Brief:   Description. More information:
 *          https://github.com/LukedeMunk/[repository]
 */
/******************************************************************************/
```

2. #define guard (see 1. below)
3. Constants
4. Global variables
5. Function declarations (if any)
6. Main function or Class declaration
7. Other functions

## Examples and other instructions

### 1. Use the `#define guard` at the start of all header files
The format of the symbol name should be `PROJECT_PATH_FILE_H_`.

```cpp
#ifndef PROJECT_PATH_FILE_H_
#define PROJECT_PATH_FILE_H_

...

#endif  // PROJECT_PATH_FILE_H_
```

***Why: All header files should have #define guards to prevent multiple inclusion.***

### 2. Use PascalCasing for enumerations, structures, class names and namespace names:

```cpp
struct InitialConfiguration {
    string userPassword;
    string adminPassword;
};

class ClientActivity {
    ClientActivity() {
        //Constructor has the same name as class
    }
}
```

### 3. Use camelCasing for methods and variables:

```cpp
class UserLog {
    void add(LogEvent logEvent) {
        uint8_t itemCount = logEvent.numberOfItems;
        // ...
    }
}
```

### 4. Use spaces around operators ( = + / * ), and after commas

```cpp
// Correct
uint8_t x = y + z;
String values = ["Volvo", "Saab", "Fiat"];

// Avoid
uint8_t x=y+z;
String values=["Volvo","Saab","Fiat"];
```

***Why: This makes it easier to read.***

### 5. Use 4 spaces for indentation of code blocks

```cpp
float toCelsius(fahrenheit) {
    return (5 / 9) * (fahrenheit - 32);
}
```

***Why: Do not use tabs (tabulators) for indentation. Text editors interpret tabs differently.***

### 6. General rules for complex (compound) statements:

1. Put the opening bracket at the end of the first line.
2. Use one space before the opening bracket.
3. Put the closing bracket on a new line, without leading spaces.
4. Do not end complex statement with a semicolon.

Functions:
```cpp
float toCelsius(fahrenheit) {
    return (5 / 9) * (fahrenheit - 32);
}
```

Loops:
```cpp
for (uint8_t i = 0; i < 5; i++) {
    x += i;
}
```

Conditionals:
```cpp
if (time < 20) {
    greeting = "Good day";
} else {
    greeting = "Good evening";
}
```

***Why: This makes it easier to read.***

### 7. Do not use Hungarian notation or any other type identification in identifiers

```cpp
// Correct
uint8_t counter;
string name;
 
// Avoid
uint8_t iCounter;
string strName;
```

***Why: IDEs make it easy to see what type a variable is. In general, you want to avoid type indicators in any identifier.***

### 8. Use comment headers for every method and align single line comments

> This implicates that the line length of code < 80 characters. If a statement does not fit on one line, the best place to break it is after an operator or a comma.

```cpp
// Correct
/******************************************************************************/
/*!
  @brief    Looks if the specified character is in the keyword.
  @param    keyword             String to search
  @param    character           Character to find
  @returns  int16_t             Index of the character, -1 when not found
*/
/******************************************************************************/
int16_t lookForCharacter(string keyword, char character) {
    if (keyword[0] == character) {}                                             //If character..
    return index;                                                               //return...
}

// Avoid
int16_t lookForCharacter(string keyword, char character) {
    if (keyword[0] == character) {} //If character..
    return index; //return...
}
```

***Why: This makes it easier to read and look for methods in big files.***

### 9. Use `#define` where possible to define constantes:

```cpp
// Correct
#define MAXIMUM_VOLUME      10

// Avoid
const uint8_t MAXIMUM_VOLUME = 10;
```

***Why: Precompiler elements get optimized and in this way the footprint of the application can be smaller. Especially important for microchips.***

### 10. Use meaningful names for variables. The following example uses seattleCustomers for customers who are located in Seattle:

```cpp
uint8_t seattleCustomers[10] = {0};
if (customer.city == "Seattle") {
    seattleCustomers[0] = customer.id;
}
```

### 11. Avoid using abbreviations. Exceptions: abbreviations commonly used as names, such as Id, Xml, Ftp, Uri.

```cpp    
// Correct
UserGroup userGroup;
Assignment employeeAssignment;

// Avoid
UserGroup usrGrp;
Assignment empAssign;

// Exceptions
CustomerId customerId;
XmlDocument xmlDocument;
FtpHelper ftpHelper;
UriPart uriPart;
```

***Why: Prevents inconsistent abbreviations.***

### 12. Use PascalCasing or camelCasing (Depending on the identifier type) for abbreviations 3 characters or more (2 chars are both uppercase when PascalCasing is appropriate or inside the identifier).:

```cpp  
HtmlHelper htmlHelper;
FtpTransfer ftpTransfer, fastFtpTransfer;
UIControl uiControl, nextUIControl;
```

### 13. Do not use Underscores in identifiers. Exception: you can prefix private fields with an underscore:

```cpp 
// Correct
DateTime clientAppointment;
TimeSpan timeLeft;

// Avoid
DateTime client_Appointment;
TimeSpan time_Left;

// Exception (Private class field)
DateTime _registrationDate;
```

***Why: Makes code more natural to read (without 'slur'). Also avoids underline stress (inability to see underline).***

### 14. Use size defined integer types like `uint8_t`, `int16_t`, `int64_t`.

```cpp
// Correct
uint32_t lastIndex;
int16_t temperature;
uint64_t sdCapacity;

// Avoid
int lastIndex;
int temperature;
int sdCapacity;
```

***Why: Memory needs to be used as optimal as possible. There is no need to store small numbers in big memory blocks. Especially important for microchips.*** 

### 15. Use noun or noun phrases to name a class. 

```cpp 
class Employee {
}
class BusinessLocation {
}
class DocumentCollection {
}
```

***Why: Easy to remember.***

### 16. Organize namespaces and classes with a clearly defined structure: 

```cpp 
// Examples
namespace Company.Technology.Feature.Subnamespace {
}
namespace Company.Product.Module.SubModule {
}
namespace Product.Module.Component {
}
namespace Product.Layer.Module.Group {
}
```

***Why: Maintains good organization of the code base.***

### 17. Start curly brackets the end of the first line:

```cpp
// Correct
class Program {
    static void Main(string[] args) {
        //...
    }
}

// Avoid
class Program
{
    static void Main(string[] args)
    {
        //...
    }
}
```

***Why: It is more space efficient than vertically aligned brackets.***

### 18. Declare all member variables at the bottom of a class in the header file, with static variables at the very top. Start with public, then private.

```cpp 
// Correct
class Account {
    public:
        Account() {
            // Constructor
        }

        static string bankName;
        static int64_t reserves;      
        uint64_t index;
        DateTime dateOpened;
    
    private:
        uint8_t _age;
}
```

***Why: Generally accepted practice that prevents the need to hunt for variable declarations.***

### 19. Use singular names for enums. Exception: bit field enums.

```cpp 
// Correct
enum Color {
    red,
    green,
    blue,
    yellow,
    magenta,
    cyan
}

// Exception [Flags]
enum Dockings {
    none = 0,
    top = 1,
    right = 2, 
    bottom = 4,
    left = 8
}
```

***Why: Makes the code more natural to read. Plural flags because enum can hold multiple values (using bitwise 'OR').***

### 20. Do not use an `Enum` suffix in enum type names:

```cpp
// Correct
enum Coin {
    penny,
    nickel,
    dime,
    quarter,
    dollar
}

// Avoid
enum CoinEnum {
    penny,
    nickel,
    dime,
    quarter,
    dollar
}
```

***Why: Consistent with prior rule of no type indicators in identifiers.***

### 21. Name task handlers (for real-time functionality) with the "Handler" suffix, as shown in the following example:

```cpp 
TaskHandle_t _downloadHandler;                      //Free-RTOS example
```

***Why: Easy see different task elements.***

### 22. Name task dispatchers (for real-time functionality) with the "dispatch" prefix, as shown in the following example:

```cpp 
static void _dispatchLogsDownload(void* parameter); //Free-RTOS example
```

***Why: Easy see different task elements.***

### 23. Name tasks (for real-time functionality) with the "task" prefix, as shown in the following example:

```cpp 
void _taskDownloadLogFile(uint8_t fileType);        //Free-RTOS example
```

***Why: Easy see different task elements.***

### 24. Do not create names of parameters in methods (or constructors) which differ only by the register:

```cpp 
// Avoid
void myFunction(string name, string Name) {
    //...
}
```

***Why: Easy to read, and also excludes possibility of occurrence of conflict situations.***

### 25. Use prefix `any`, `is`, `has` or similar keywords for boolean identifier:

```cpp 
// Correct
bool isZero(uint8_t value) {
    return (value == 0);
}
```

***Why: Easy to read and understand.***

### 26. File guidelines

1. Classes should consist of a header (`.h`) file and a C++ (`.cpp`) file. In the header file, the class gets declared and the dependecies are included. In the C++ file, the class gets defined.
2. For project globals, a `Globals.h` file should be included in the project. This file includes every global constant, variable or definition.

## Reference

- [MSDN General Naming Conventions](http://msdn.microsoft.com/en-us/library/ms229045(v=vs.110).aspx)
- [MSDN Naming Guidelines](http://msdn.microsoft.com/en-us/library/xzf533w0%28v=vs.71%29.aspx)
- [MSDN Framework Design Guidelines](http://msdn.microsoft.com/en-us/library/ms229042.aspx)