# RayTracing

This repo was a simple raytracer but has become a much more advanced ray tracer. The reflections and lighting was still implemented without a tutorial so there could be some inaccuracies when compared to the real world which will be interesting to find in the future. New lighting methods will be added in the future so there can be point lights, directional lights, spotlights, etc. Reflections have been a very fun part of this project since I can do infininte reflections and make rooms look much bigger than they really are. <br>
The performance for the ray tracer is not terrible when running a 400x400 pixel image, but could be much better. The program is single threaded though CPU parallelization was added a while ago, it is turned off as of right now. After getting a form of parallel threadding implemented, I will work on moving over to the GPU since that will be much faster. The plan still is to go to OpenCL but since OpenGL is already integrated for viewing the image in real time, I may ending up going for that after all. I would like to add spheres and asset importing soon along with materials so everything is not just a single color and not metalic.

<br>

<br>
The image below shows an example where the two walls facing the user are relfective and there is a light source on the ceiling. There are shadows at the bottom of the image due to the blue square covering the light. The camera does work and can be used with the WASD and arrow keys. I would recommend changing the image size to 400x400 for good performance but use 800x800 for a higher resolution image.
<br>

![image](https://github.com/Expasito/RayTracing/assets/93100379/0c06d83d-9ce9-4cf7-b7e3-d539241446b0.png)
