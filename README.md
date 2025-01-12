# mariogame
A small project I worked on in my first year of college. It contains 5 levels in which Mario has to collect all stars on the map and avoid the brown mushrooms. However, Mario can kill them by jumping on top of them.

This was entirely implemented in C++ using CodeBlocks version 16.01 with the graphics.h and winbgim.h libraries for the GUI.

Steps for linking the GUI:
   1. Locate the installation path of CodeBlocks and find the include folder in the MinGW compiler folder (path example: C:\Program Files (x86)\CodeBlocks\MinGW\include\). Copy the graphics.h and winbgim.h files from the winbgim folder there.
   2. Locate the lib folder on your device (path example: C:\Program Files (x86)\CodeBlocks\MinGW\lib\). Copy the libbgi.a file from the winbgim folder there.
   3. In CodeBlocks click on Settings -> Compiler... -> Linker settings. In the link libraries tab click Add and locate the libbgi.a file you just copied.
   4. In the other linker options tab, write the following commands: -lbgi -lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32
   5. Click OK.

Steps for linking the audio library and ensuring chrono library is working:
   1. In CodeBlocks, on your desired project, click on Project -> Build Options... -> Linker settings. In the link libraries tab click Add and just write winmm.
   2. Click on Settings -> Compiler... -> Compiler flags. Make sure the following option is enabled: Have g++ follow the C++11 ISO C++ language standard

Steps for adding project resources:
   1. Create a new project and run it for the first time.
   2. Locate your project and copy the files from the resources folder there.
   3. Copy the source code from the .src folder.
