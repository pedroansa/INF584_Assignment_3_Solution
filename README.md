MyRenderer
Base code for INF584 "Image Synthesis" Assignments
==================================================

Author: Tamy Boubekeur (tamy.boubekeur@telecom-paris.fr)

### Building

This is a standard CMake project. It is recommanded to use shell, such as Linux Bash the Developer Powershell of Visual Studio under Windows. 
Building it consists in running:

```
cd <path-to-MyRenderer-directory>
mkdir build
cmake -B build
cmake --build build --config Release
```

By default, the program is compile in Debug mode. For a high performance Release binary, just do:

```
cmake --build build --config Release
```

Each time you change your code, you can recompile it by typing the previous command.

### Running

To run the program
```
cd <path-to-MyRenderer-directory>
./build/Release/MyRenderer(.exe) [[file.off] [material-directory-path]]
```
Note that a collection of example meshes are provided in the Resources/Models directory and a collection of materials are provided in the Resources/Materials directory

When starting to edit the source code, rerun 

```
cmake --build build --config Release
```
each time you want to recompile MyRenderer


