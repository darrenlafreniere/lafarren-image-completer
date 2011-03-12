///////////////////////////////////////////////////////////////////////////////
// NOTICE

Copyright 2010, Darren Lafreniere
<http://www.lafarren.com/image-completer/>

This file is part of lafarren.com's Image Completer.

Image Completer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Image Completer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Image Completer, named License.txt. If not, see
<http://www.gnu.org/licenses/>.


///////////////////////////////////////////////////////////////////////////////

// Getting started

If you are using Linux, you must #define UNIX. There is a line in the CMakeLists.txt
file that can be uncommented to achieve this.

///////////////////////////////////////////////////////////////////////////////

// Examples
There are .bat files (for Windows) and .sh files (for Linux) in the test-data directory. They demonstrate how to run the program.


    Windows:
    image-completer-cmd.exe -ii elephant-input.png -im elephant-mask.png -io elephant-output.png

    Linux:
    ./ImageCompleter -ii elephant-input.png -im elephant-mask.png -io elephant-output.png


NOTE: to quickly complete an image, add this to the command line: -sp auto
The final output image quality may be reduced, but this will automatically, internally scale the image down to a quickly solvable size.

For a full list of options, run the executable without any additional arguments.

///////////////////////////////////////////////////////////////////////////////

// NOTES

Requires:
- wxWidgets 2.9

Developed using:

- Visual C++ Express 2008 and 2010.
- wxWidgets (wxMSW-2.8.10, static C runtime (/MT))
- FFTW (3.2.2, static C runtime (/MT), single precision enabled)
- Taucs, Metis, and Atlas (included in lib)

Tested on:
- Fedora 13
- GCC 4.4.5