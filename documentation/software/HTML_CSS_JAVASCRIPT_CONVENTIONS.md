# HTML/CSS/JavaScript Coding Conventions

This document secures the code quality of web modules (HTML/CSS/JavaScript) by describing the coding conventions.

> When the code doesn't fulfill the guidelines, the code cannot be integrated in the project.

## HTML naming conventions (for element IDs)

| Element Type       | Notation   | Length | Prefix | Suffix   | Char Mask   | Example        |
|--------------------|------------|-------:|--------|----------|-------------|----------------|
| Title              | camelCase  |     50 | No     | `Title`  | [A-z][0-9]  | `pageTitle`    |
| Output field       | camelCase  |     50 | No     | `Field`  | [A-z][0-9]  | `successField` |
| Text inputs        | camelCase  |     50 | No     | `Txt`    | [A-z][0-9]  | `nameTxt`      |
| Range inputs       | camelCase  |     50 | No     | `Range`  | [A-z][0-9]  | `ageRange`     |
| Checkboxes         | camelCase  |     50 | No     | `Cb`     | [A-z][0-9]  | `enableCb`     |
| Selectors          | camelCase  |     50 | No     | `Select` | [A-z][0-9]  | `audioSelect`  |
| Buttons            | camelCase  |     50 | No     | `Btn`    | [A-z][0-9]  | `submitBtn`    |

## CSS naming conventions (for classes)

| Element Type       | Notation   | Length | Prefix | Suffix | Char Mask   | Example      |
|--------------------|------------|-------:|--------|--------|-------------|--------------|
| class              | kebab-case |     50 | No     | No     | -[a-z][0-9] | `test-class` |

## JavaScript naming conventions

| Object Name        | Notation   | Length | Prefix | Suffix | Char Mask   |  Example       |
|--------------------|------------|-------:|--------|--------|-------------|----------------|
| Function names     | camelCase  |     50 | No     | No     | [A-z][0-9]  | `helloWorld`   |
| Class name         | PascalCase |     50 | No     | No     | [A-z][0-9]  | `UserAccount`  |
| Variables          | camelCase  |     50 | No     | No     | [A-z][0-9]  | `testVariable` |
| Constants name     | MACRO_CASE |     50 | No     | No     | _[A-z][0-9] | `MINIMUM_AGE`  |
| HTML element names | camelCase  |     50 | No     | `Elem` | [A-z][0-9]  | `ageRangeElem` |
| Text constants     | MACRO_CASE |     50 | `TEXT` | No     | _[A-z][0-9] | `TEXT_HELLO`   |

## File layout

> Do use this layout for every JavaScript file.

1. File header:

```javascript
/******************************************************************************/
/*
 * File:    [name_of_file].js
 * Author:  [author]
 * Version: [X].[X].[X]
 * 
 * Brief:   Description. More information:
 *          https://github.com/LukedeMunk/[repository]
 */
/******************************************************************************/
```

2. HTML element constants
3. Constants
4. Global variables
5. Key event listeners
6. Main function (i.e.: `$(document).ready(function() {...});`)
7. Other functions

## Examples and other instructions

### 1. Use camelCasing for methods and variables:

```javascript
var loginUser(username, password) {
    var passwordIsCorrect;
    //...
}
```

### 2. Use PascalCasing class names:

```javascript
class DateTime {
    constructor(date, time) {
        this.date = date;
        this.time = time;
    }
}
```
> Don't start names with a $ sign. It will put you in conflict with many JavaScript library names.

### 3. Use spaces around operators ( = + / * ), and after commas

```javascript
// Correct
var x = y + z;
var values = ["Volvo", "Saab", "Fiat"];

// Avoid
var x=y+z;
var values=["Volvo","Saab","Fiat"];
```

***Why: This makes it easier to read.***

### 4. Use 4 spaces for indentation of code blocks

```javascript
function toCelsius(fahrenheit) {
    return (5 / 9) * (fahrenheit - 32);
}
```

***Why: Do not use tabs (tabulators) for indentation. Text editors interpret tabs differently.***

### 5. Always end simple statements or lines with a semicolon.

```javascript	
var values = ["Volvo", "Saab", "Fiat"];

var person = {
    firstName: "John",
    lastName: "Doe",
    age: 50,
    eyeColor: "blue"
};
```

***Why: Improves readability and is in line with other programming languages like C/C++.***

### 6. General rules for complex (compound) statements:

1. Put the opening bracket at the end of the first line.
2. Use one space before the opening bracket.
3. Put the closing bracket on a new line, without leading spaces.
4. Do not end complex statement with a semicolon.

Functions:
```javascript
function toCelsius(fahrenheit) {
    return (5 / 9) * (fahrenheit - 32);
}
```

Loops:
```javascript
for (var i = 0; i < 5; i++) {
    x += i;
}
```

Conditionals:
```javascript
if (time < 20) {
    greeting = "Good day";
} else {
    greeting = "Good evening";
}
```

***Why: This makes it easier to read.***

### 7. General object rules

General rules for object definitions:

1. Place the opening bracket on the same line as the object name.
2. Use colon plus one space between each property and its value.
3. Use quotes around string values, not around numeric values.
4. Do not add a comma after the last property-value pair.
5. Place the closing bracket, on a new line, without leading spaces.
6. Always end an object definition with a semicolon.

Example:

```javascript
var person = {
    firstName: "John",
    lastName: "Doe",
    age: 50,
    eyeColor: "blue"
};
```

Short objects can be written compressed, on one line, like this:

```javascript
var person = {firstName:"John", lastName:"Doe", age:50, eyeColor:"blue"};
```

### 8. Use comment headers for every method and align single line comments

> This implicates that the line length of code < 80 characters. If a statement does not fit on one line, the best place to break it is after an operator or a comma.

```javascript
// Correct
/******************************************************************************/
/*!
  @brief    Looks if the specified character is in the keyword.
  @param    keyword             String to search
  @param    character           Character to find
  @returns                      Index of the character, -1 when not found
*/
/******************************************************************************/
function lookForCharacter(keyword, character) {
    if (keyword[0] == character) {}                                             //If character..
    return index;                                                               //return...
}

// Avoid
function lookForCharacter(keyword, character) {
    if (keyword[0] == character) {} //If character..
    return index; //return...
}
```

***Why: This makes it easier to read and look for methods in big files.***

### 9. Loading JavaScript in HTML

Use simple syntax for loading external scripts (the type attribute is not necessary):

```html	
<script src="myscript.js">
```

### 10. Accessing HTML elements

The HTML elements that are used in JavaScript code, need to be retrieved and saved as constant at the start of the JavaScript file. This is done to improve the readability.

HTML:
```html
<h3 id="pageTitle"></h3>
<input id="nameTxt" type="text">
```

JavaScript:
```javascript
const pageTitleElem = document.getElementById("pageTitle");
const nameTxtElem = document.getElementById("nameTxt");
```

### 11. HTML elements attributes priority

HTML elements with multiple attributes, have to be organized in this order:

```html
<h3 id="" class="" [other attributes] style=""></h3>
```

### 12. File extensions

1. HTML files should have a `.html` extension (not `.htm`).
2. CSS files should have a `.css` extension.
3. JavaScript files should have a `.js` extension.

## Reference

- [Google JavaScript Style Guide](https://google.github.io/styleguide/jsguide.html#introduction)
- [JavaScript Style Guide and Coding Conventions](http://www.w3schools.com/js/js_conventions.asp) 
