# HacerExportar

## Overview

HacerExportar is a utility designed to address the complexities involved in using DLLs without header files and lib libraries. This tool is particularly useful for handling DLLs that export functions using C++ conventions, which can be challenging due to the presence of class pointers and the need to acquire addresses for all exported class functions and understand class structures.

## Key Features

- **Header File and Lib Generation**: HacerExportar generates corresponding header files and lib import libraries based on the information from the export table of a DLL.
- **C++ Support**: While the tool generates C++ header files, it currently does not handle virtual function tables. Additionally, the generated C++ classes do not include member variables, meaning the size of C++ classes remains undetermined.
- **C Language Exports**: For C language exports, only function names are provided without type specifications. Users need to resolve this aspect manually.

## Technical Details

### Generating a Lib

To generate a lib, a def file is required at a minimum. This can be achieved using `link.exe`, which should be accessible in your system's PATH environment variable (commonly available if Visual Studio is installed). The following command can be used:

```cmd
link /LIB /DEF:C:\path\to\yourfile.def /OUT:C:\path\to\yourlib.lib
```

An alternative approach involves creating a DLL project with the same name as the original DLL and implementing all functions. However, this method is more complex and less preferred.

### Generating Header Files

The process of generating header files involves proprietary algorithms and is not detailed here. Interested users can refer to the source code for more insights.

## Output

The generated header files and lib files are placed in the same directory as the DLL.

## Usage

Please refer to the source code for detailed usage instructions and examples.

## Note

This tool is a work in progress and may not handle all cases perfectly. Users are encouraged to contribute to its development or report any issues encountered.
