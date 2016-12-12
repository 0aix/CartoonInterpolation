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

TBD