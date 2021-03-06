CartoonInterpolation
====================

For Assignment 5, I've decided to implement cartoon interpolation separate from 
Scotty3D. It will be written in C++ using DirectX (Windows-specific code). 

Plan:

1. Set up a basic UI to load a single texture and display the texture in a 
   reasonably-sized window.
2. Extend the UI to be able to add/select/delete/render control points, zoom in and       out, translate, etc. 
3. Implement image deformation methods; at minimum, hopefully, the aligator example
   using bounded biharmonic weights. (http://igl.ethz.ch/projects/bbw/)
4. Round out UI controls, organization, etc.
5. *Implement other deformation methods (i.e. different weighting)
6. *Add animation over time using splines to interpolate vertex positions.
7. *Save/Load images, data, and frames

*Time permitting
**I won't push anything until right before the deadline, if that's a concern.

Usage
=====
0. Image must fit within a 1000x600 px^2 window
1. To load a file (e.g. alligator.png), load 'alligator' (without the extension). The
   .png and .objx files must be in the same folder. (The .objx files were renamed from      .obj to bypass .gitignore)
2. Hit 'tab' to toggle the mesh
3. Set point handles with shift + click. Points can be dragged around
3. Set joint handles with ctrl + click. Joints can be selected (and will only be placed 
   on ctrl + click).
4. Hit 'escape' to deselect points or quit placing joints.
5. Hit 'enter' to enter pose mode. (cannot go back to setting handles)
6. Drag control points to deform the image. Point handles can be rotated by right-clicking and dragging.

Final report
============
1. Could not implement bounded biharmonic weights. I spent a lot of time looking over
   the paper, the matlab code, and the C++ code, but the code is all very non-intuitive
   (matrix hacks populating the matlab code) and poorly documented. I also spent a lot
   of time looking at mesh generation to no avail (aka I figured out how to compute the
   boundary vertices and then realized the author passed in the data to a "triangle"
   program through a convoluted matlab wrapper).
** Cage not implemented because it's meaningless without the specific bounded biharmonic
   weights. It's still possible to form an "open" cage with joints, however.
2. Skeleton LBS and rotation WAS bugged even though my math was 100% correct because
   apparently DirectX applies transformations with right-multiplication and not left.
3. Really couldn't implement what I wanted to. I started late and spent many
   days trying to understand BBW and 2D LBS (it took me quite a while to realize that
   the image had to be meshed first and then a 2D-version of 3D LBS could be applied..)
4. It's really interesting to see the deformed version of a 2D mesh because it looks
   3D..
5. I tried ¯\_(ツ)_/¯