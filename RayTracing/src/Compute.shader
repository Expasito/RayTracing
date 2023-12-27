#version 450 core

// define the local input size for each invocation
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// define how we get the data out of the CS shader. Binding = 0 means we use texture0
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform float testFloat;

struct Triangle {
	vec3 point;
	vec3 normal;
	vec3 edge0, edge1, edge2;
};

uniform Triangle triangle;


void main() {
	vec4 value = vec4(0, 0, 0, 1);
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

	value.x = float(texelCoord.x) / (gl_NumWorkGroups.x);
	value.y = float(texelCoord.y) / (gl_NumWorkGroups.y);

	//value.x = texelCoord.x;
	//value.y = texelCoord.y;
	value = vec4(triangle.point, 1);

	vec4 val = imageLoad(imgOutput, texelCoord);
	imageStore(imgOutput, texelCoord, value*testFloat);

	//float inc = .001;
	//imageStore(imgOutput, texelCoord, vec4(val.x + inc, val.y + inc, val.z, val.w));
}