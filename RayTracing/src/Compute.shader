#version 450 core

// define the local input size for each invocation
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// define how we get the data out of the CS shader. Binding = 0 means we use texture0
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform float testFloat;

uniform vec3 cameraPosition;
uniform mat4 trans;

struct Triangle {
	vec3 p;
	vec3 n;

	vec3 p1, p2, p3;
	vec3 edge0, edge1, edge2;

	vec3 color;

};

uniform Triangle triangle[50];

uniform int numTriangles;

struct PayLoad {
	vec3 point;
	vec3 color;
	float distance;
	int index;
	bool didHit;
};

PayLoad castRay(vec3 orgin, vec3 dir, int index) {
	PayLoad closest = { {0,0,0},{0,0,0},1e9,-1,false };

	for (int i = 0; i < numTriangles; i++) {
		if (i == index) {
			continue;
		}


		Triangle cur = triangle[i];

		float t = dot(cur.p1 - orgin, cur.n) / dot(dir, cur.n);
		if (t < 0) {
			continue;
		}
		//closest.distance = t;
		
		vec3 I = orgin + (t * dir);

		if (t < closest.distance && t > .00001) {
			if (
				dot(cur.n, cross(cur.edge0, I - cur.p1)) > 0.0 &&
				dot(cur.n, cross(cur.edge1, I - cur.p2)) > 0.0 &&
				dot(cur.n, cross(cur.edge2, I - cur.p3)) > 0.0


				) {
				closest.point = I;
				closest.color = cur.color;
				closest.distance = t;
				closest.index = i;
				closest.didHit = true;
			}
		}
	//closest.distance = t;
	}

	//closest.distance = 1e9;

	//closest.distance = .5f;

	//closest.distance = abs(dot(dir, triangle[0].edge0));

	//closest.didHit = true;

	return closest;
}



void main() {


	// get the x and y values of the pixel we are working with
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

	int width = int(gl_NumWorkGroups.x);
	int height = int(gl_NumWorkGroups.y);

	float v = (texelCoord.y - height / 2.0) / float(height);
	float u = (texelCoord.x - width / 2.0) / float(width);

	vec4 pixel2 = trans * vec4(u, v, .5, 1.0);
	vec3 pixel = {pixel2.x,pixel2.y,pixel2.z};

	vec3 dir = pixel;
	vec3 orgin = cameraPosition;

	PayLoad hit = castRay(orgin, dir, -1);


	vec4 value = vec4(0, 0, 0, 1);

	if (hit.didHit == true) {
		value = vec4(hit.color, 1);
		//value = vec4( 1.0, 1.0, 1.0, 1.0);
	}
	else {
		value = vec4( 0, 0, 0, 0 );
	}


	//value = vec4(hit.distance,0,0, 1);

	//value.x = dir.x;
	//value.y = dir.y;
	//value = vec4(orgin, 1);

	//vec4 val = imageLoad(imgOutput, texelCoord);
	imageStore(imgOutput, texelCoord, value);

	//float inc = .001;
	//imageStore(imgOutput, texelCoord, vec4(val.x + inc, val.y + inc, val.z, val.w));
}