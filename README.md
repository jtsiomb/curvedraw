Curve Draw
==========

![curvedraw](http://nuclear.mutantstargoat.com/sw/misc/curves-512.png)

Overview
--------
Curvedraw is a simple 2D curve drawing program. I've often found it useful to
draw curves for use in my graphics programs. They can be useful for procedural
modelling (extrusion or surfaces of revolution), for animation paths, etc.

License
-------
Copyright (C) 2015-2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use it, modify it and/or
redistribute it under the terms of the GNU General Public License version 3 or
(at your option) any later version published by the Free Software Foundation.
See COPYING for details.

Dependencies
------------
You need to install the following libraries before compiling curvedraw:
 - libvmath: https://github.com/jtsiomb/libvmath
 - libimago: https://github.com/jtsiomb/libimago
 - libdrawtext: https://github.com/jtsiomb/libdrawtext
 - Qt 5 (optional): http://www.qt.io
 - GLUT (optional): http://freeglut.sourceforge.net

You need to have at least one of the dependencies marked as optional, to build
either the full GUI (uses Qt), or the simple hotkey UI (uses GLUT) version of
curvedraw.

Before building, if you intend to build the Qt GUI version of curvedraw, make
sure you have the `data` directory in the project root. The data directory is
included in release archives, but you will need to download it manually if you
got the code from the git repository.
Run the following command in the project root directory, to grab the data files
from the data repository (subversion):

`svn co svn://mutantstargoat.com/datadirs/curvedraw data`

Build
-----
To build curvedraw you will need cmake version 3.0 or greater.

Create a `build` directory and change into it, then execute cmake to configure
the project and generate the makefile, or project files for your favourite IDE:

```
mkdir build
cd build
cmake ..
```

The build system will auto-detect which version of curvedraw to build, based on
the presence of the optional dependencies (Qt or GLUT) on your system. If you
need to change the result of the autodetection, or in case cmake failes to
locate the Qt library, you may use the interactive variants of cmake, `ccmake`
or `cmake-gui`, to tweak the configuration manually.

After the build files are generated, type `make` to build and `make install` as
root to install curvedraw system-wide.

Usage
-----
Mouse:
 - Click to start adding points to a new curve, then finish it by right clicking.
 - Click on a curve to select it.
 - Right-click on a control point to remove it.
 - Click with a curve selcted to continue adding points to it.
 - Right-click in empty space to clear the selection.

Keys:
 - Holding 's' while adding points snaps to grid.
 - Holding SHIFT-'s' while adding points snaps to nearest existing point.
 - Press ESC to cancel the currently created curve.
 - Press 'delete' or 'backspace' to delete the currently selected curve.
 - Press 'q' to exit.
 - Press '1' - '3' to change selected curve type (polyline, hermite, bspline).
 - Press 'b' to show the selected curve's bounding box.
 - Press 'n' to normalize the selcted curve to the unit square.
 - Press 'e' to export the curves to a file called "test.curves" (TODO file dialog)
 - Press 'l' to load curves from the "test.curves" file (TODO file dialog)

Viewport:
 - Drag with the left or middle mouse button to pan.
 - Drag up/down with the right mouse button, or use the mouse wheel to zoom.

Curves file format
------------------
The files read and written by this program are simple text files. They start
with the word "GCURVES" in caps in the first line, followed by a series of curve
blocks of the form::

```
  curve {
      type <curve type, can be "hermite", "polyline", or "bspline">
      cpcount <number of control points>
      cp <x> <y> <z> <w>
      ...
  }
```

The control points are 4-vectors to allow for representing 3D rational
b-splines. Lower-dimensional curves should set 'w' to 1, and all other unused
coordinates to 0. Rational 2D b-splines are represented as 3D splines on the z=0
plane, so again 'w' acts as the weight.
