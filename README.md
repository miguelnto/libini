# libini

A small C99 library for reading **.ini** config files. 

For more information on `.ini` files, jump to [The INI format](#the-ini-format) section.

## Requirements

- A C99 compiler
- GNU Make

## Installation

You can simply install this library by running the following command *(if needed, run it as root):*

```sh
make install
```

This should install the library in the `/usr/lib` folder.

Alternatively, you could just drop the files **[ini.c](ini.c)** and **[ini.h](ini.h)** to a folder of your preference in your project, and build them along with the other source files in your project.

You can also uninstall the library by running: *(as root, if needed):*

```
make uninstall
```

To clean the unnecessary stuff after the compilation, run:

```
make clean
```

## Usage

Here you'll find a basic introduction with an example on how to use this library. For further information, please read the [reference section.](#reference)

Let's say I have the following `main.ini` file:

```ini
; Here we have information about a person (in this case, John Doe). 
[person]
name = John Doe
age = 43

[info]
; More information about John Doe
height = 1.80
mother = "Mary D. Jane"
```

First of all, we need to load the ini file. An ini file can be loaded into memory by using the `ini_load()` function.
`NULL` is returned if the file cannot be loaded. The variable needs to be free'd after use.
```c
#include <ini.h>

int main(void) {
    ini_t *config = ini_load("main.ini");
    // Treat the failure case accordingly
    if (!config) {
      return 1;
    }
    ini_free(config);
    return 0;
}
```

Now let's say we want to know the name of John Doe's mother.

The library provides the function `ini_getstring()` for retriving the value from a key-value pair as a string. If accepts an **ini_file**, a **section name** and the **key name** as parameters, in this order. Before using the value, we also need to check if it's `.ok`, that is, if it exists and its type is valid. After that, let's specify we want to get the value as a string. 

```c
ini_value mother = ini_getstring(config, "info", "mother");
if (mother.ok) {
  printf("Name of John Doe's mother: %s\n", mother.value.String);
}
```

What if we want to know John Doe's height?

It's simple, let's use the `ini_getdecimal()` function.

```c
ini_value height = ini_getdecimal(config, "info", "height");
  if (height.ok) {
    printf("John Doe's height: %f\n", height.value.Decimal);
}
```

Finally, let's check how old John Doe is:
```c
ini_value age = ini_getint(config, "person", "age");
  if (age.ok) {
    printf("John Doe's age: %d\n", age.value.Integer);
}
```

The final program should look something like this:

```c
#include <ini.h>
#include <stdio.h>

int main(void) {
    ini_file *config = ini_load("main.ini");
    if (!config) {
        return 1;
    }

    ini_value mother = ini_getstring(config, "info", "mother");
    if (mother.ok) {
        printf("Name of John Doe's mother: %s\n", mother.value.String);
    }

    ini_value height = ini_getdecimal(config, "info", "height");
    if (height.ok) {
        printf("John Doe's height: %f\n", height.value.Decimal);
    }

    ini_value age = ini_getint(config, "person", "age");
    if (age.ok) {
        printf("John Doe's age: %d\n", age.value.Integer);
    }
    
    ini_free(config);
    return 0;
}
```

You should now be able save the file (as `main.c`, for example) and compile this program with GCC by running:

```
gcc -lini main.c -o main 
```

That's it. Happy hacking!

## Reference

If you want to have a look at a pratical example on how to use this library, please check out the [usage](#usage) section.

Below there is a quick reference over the functions and types.

### Types

```c
struct ini_file; // Represents the .ini file.
struct ini_value; // Represents a possible value from the .ini file. 
// Note that if this value is valid it must have one of the following types:
// String (const char *)
// Boolean (bool)
// Integer (i64)
// Decimal (f64)
```

### Functions

```c
// Loads a file and process it as an ini_file. The variable must be free'd after use.
ini_file *ini_load(const char *filename);
// Frees a variable of ini_file type.
void ini_free(ini_file *ini);
// Searchs for a value according to the table name and key. Returns a ini_value of type i64.
ini_value ini_getint(ini_file *ini, const char *tablename, const char *key);
// Searchs for a value according to the table name and key. Returns a ini_value of type bool.
ini_value ini_getbool(ini_file *ini, const char *tablename, const char *key);
// Searchs for a value according to the table name and key. Returns a ini_value of type const char *.
ini_value ini_getstring(ini_file *ini, const char *tablename, const char *key);
// Searchs for a value according to the table name and key. Returns a ini_value of type f64.
ini_value ini_getdouble(ini_file *ini, const char *tablename, const char *key);
```

## The INI format

The `.ini` file format is mainly used as a configuration file that consists of sections containing key-value pairs. As there isn't really a standard for this format, in this section I'll be clarifing what I adopted as a standard for this library. 

I'll organize this in three subsections, focusing on the three components that a `.ini` is made of: **Section**, **Key-value pair**, and **Comments**.

### Section

A section in `.ini` should be organized like this:

```
[this is a section] ; Spaces in a section title is valid.

[ThisIsAnotherSection] ; No spaces is of course valid.

[
this is not a section ; a section is meant to be defined in just one line, so this isn't valid
]
```

### Key-value pair

A key-value pair goes inside sections and can be defined just as you would declare a variable in a language like Ruby or Python:

```
[mySection]
the_key = the_value
```

A value can have one of the following types:

- i64
- bool
- string
- f64

Examples will be shown below.

#### i64

```
[my section]
mynum = 11
myothernum = 100000000000
```

#### bool

```
[my section]
mybool = true
myotherbool = false
```

#### string

Quoted string values (with escapes) are valid. Unquoted values and keys are trimmed of whitespace when loaded.

```
[my section]
mystr = "A valid string"
myotherstr = I'm also a valid string.
yetanotherstr = \"I'm a real string with quotes\"
```

#### f64

```
[my section]
myfloat = 1.2
myotherfloat = 3.1444444
```

### Comments

The `;` character detones a comment. If it's inside a section definition, the rest of the line won't be treated as a comment.

```
; this is a valid comment

[proje;ct] ; this line is perfectly valid, and the name of this section is <proje;ct>

mynum ;= 11 the comment started right before the = sign. Here, no valid key-value pair is defined.
```

## Credits

This library is a refactoring and ressurection of [ini.](https://github.com/rxi/ini)
