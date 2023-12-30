# This is a repo for 'Computer graphics' university subject.

Numbers in the end of 'Comp_graphics_...' directories names are numbers of laboratory work.

'Libraries' directory contain all files needed for OpenGL. Also some classes in 'auxiliary' directories that provide some convenience with OpenGL programs.
External linking has been used to link these files to different projects, .vcxproj files contain the necessary information to link them to projects.

If I didn't screw up (or forget something), you can just build solution in Visual Studio, choose one of the project as default and it should run.
If I did screw up (sorry), then go to one of the project properties (guide for Visual Studio).
Then in VC++ Directories:
1) Include Directories: add '<your_path>\Comp_graphics\Libraries\include' without single brackets;
2) Library Directories: add '<your_path>\Comp_graphics\Libraries\lib' without single brackets;
Then in Linker -> Input -> Additional Dependencies -> <Edit...>, write this:
assimp-vc143-mtd.lib
glfw3.lib
opengl32.lib

You might also need need to link externally source files to your projects (or just copy them to a project's directory):
In Solution Explorer:
Source files -> Add -> Existing Item...
and choose all necessary source files.
