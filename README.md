#This is a repo for 'Computer graphics' university subject.

Right now there are 2 projects in one Visual Studio solution:
'Comp_graphics' is lab 1.
'Comp_graphics_2' is lab 2.

'Libraries' directory contain all files needed for OpenGL. Also added Camera and ShaderProgram classes there too in 'auxiliary' directories.
External linking has been used to link these files to different projects, .vcxproj files contain needed information to link them.

If I didn't screw up (or forget something), you can just build solution in Visual Studio, choose one of the project as default and it should run.
If I did screw up (sorry), then go to one of the project properties (guide for Visual Studio).
Then in VC++ Directories:
1) Include Directories: add '<your_path>\Comp_graphics\Libraries\include' without single brackets;
2) Library Directories: add '<your_path>\Comp_graphics\Libraries\lib' without single brackets.
Then go to Linker -> Input -> Additional Dependecies: add 'glfw3.lib;opengl32.lib;' without single brackets.
