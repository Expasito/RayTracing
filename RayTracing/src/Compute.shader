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

struct Light {
	vec3 position;
	float intensity;
};


layout(std140, binding = 2) buffer lights
{
	Light data[];
} Lights;

layout(std140, binding = 3) uniform lights2
{
	Light data[25];
} Lights2;


uniform Triangle triangles[50];

//uniform Light lights[25];

uniform int numTriangles;

uniform int numLights;

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


		Triangle cur = triangles[i];

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
				dot(cur.n, cross(cur.edge2, I - cur.p3)) > 0.0) {
					closest.point = I;
					closest.color = cur.color;
					closest.distance = t;
					closest.index = i;
					closest.didHit = true;
			}
		}
	}


	return closest;
}

float mag(vec3 a) {
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

vec4 processLighting(PayLoad hit) {
	vec3 result = vec3(0,0,0);

	if (hit.didHit == false) {
		return vec4(result, 1);
	}

	for (int i = 0; i < numLights; i++) {
		//Light light = Lights.data[i];
		Light light = Lights2.data[i];
		vec3 lightDir = normalize(light.position - hit.point);
		PayLoad hit2 = castRay(hit.point, lightDir, hit.index);
		float dist = mag(light.position - hit.point);
		bool shadow = false;
		// we did not hit any objects at all, then not in shadow
		// or we did hit but the distance is greater than that to the light
		if (hit2.didHit == false || (hit2.didHit == true && hit2.distance > dist)) {
			shadow = false;
		}
		else {
			shadow = true;
		}

		if (shadow == true) {
			//result += .15 * hit.color;
		}
		else {
			//result += 1.0/(mag(light.position - hit.point)) * hit.color;
			float dist_ = mag(light.position - hit.point);
			result += vec3((1.0 / (dist_ * dist_) * light.intensity / 255.0) * hit.color / 255.0);
			//result += hit.color;
		}
		//result += vec3(0, 0, 1);
		//result += intensity;
		//result += vec3(intensity,intensity,intensity);
		//float c = 1.0 / 0.0;
		//result += (light.position - hit.point);
		//result += lightDir;
		//result += vec3(dist/10.0);
	}
	//result = vec3(mag(lights[0].position - hit.point)/10.0);

	return vec4(result.x,result.y,result.z, 1);
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


	// now process the lighting for that pixel
	vec4 value = processLighting(hit);

	////value = vec4(numLights);
	//for (int i = 0; i < numLights; i++) {
	//	value += vec4(lights[i].intensity);
	//}

	//if (hit.didHit == true) {

	//	value  = processLighting(hit);

	//	//value = vec4( 1.0, 1.0, 1.0, 1.0);
	//}
	//else {
	//	value = vec4( 0, 0, 0, 0 );
	//}

	vec3 val = vec3(0);
	for (int i = 0; i < 1; i++) {
		Light l = Lights2.data[i];

		// get direction from light to camera
		vec3 lightDir = l.position - orgin;

		// get the distance from the light
		float dist = length(l.position - orgin);

		float dott = (dot(normalize(lightDir), normalize(dir)));

		val += vec3(dott*dott/(dist*dist));

		// get the dot product of the lightDir and our view dir for this pixel
		//float dott = max(dot(lightDir, dir)*10.0/(dist*dist), 0);
		//val = vec3(dott);
		//val = vec3(dist);
		//val = vec3(dot(lightDir, dir));
	}


	value += vec4(val, 1);

	//value = vec4(hit.distance,0,0, 1);

	//value.x = dir.x;
	//value.y = dir.y;
	//value = vec4(orgin, 1);

	//value = vec4(numLights / 10.0, 0, 0, 1);

	//value = vec4(Lights.data[1].position, 1);

	//vec4 val = imageLoad(imgOutput, texelCoord);
	imageStore(imgOutput, texelCoord, value);

	//float inc = .001;
	//imageStore(imgOutput, texelCoord, vec4(val.x + inc, val.y + inc, val.z, val.w));
}