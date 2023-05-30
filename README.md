# RayTracing

RayTracing is a simple raytracer program. For now, it only does a depth of one for the reflections as it is a preperation project for a much faster version. <br>
The main goal is to use OpenCL to parallelize the raytracing program without requiring OpenGL or Compute Shaders simply because we read and write to an unsigned int buffer here. More shapes and diverse models will eventually be added, but also may be added to a seperate project which will be more advanced.

<br>

This is my first raytracer so there are a lot of other improvements that can be added but that may be implemented in a seperate repo with parallelization.
For now, it will be a demo project to test different ideas out and work towards the larger project of a raytraced game using the physics engine from a seperate project also being worked on. RayTracing outputs an image in a ppm format which does require a seperate software to load. As mentioned before, there are lots of improvements that can be added to the ray triangle or ray plane intersections, especially if other shapes or meshes are imported.
