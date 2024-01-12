#version 450 core

// define the local input size for each invocation
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// define how we get the data out of the CS shader. Binding = 0 means we use texture0
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform float testFloat;

uniform vec3 cameraPosition;
uniform mat4 trans;


// define several rotation matricies that I already had from previous
// projects
mat3 RotateX(float phi) {
	return mat3(
		vec3(1., 0., 0.),
		vec3(0., cos(phi), -sin(phi)),
		vec3(0., sin(phi), cos(phi)));
}

mat3 RotateY(float theta) {
	return mat3(
		vec3(cos(theta), 0., -sin(theta)),
		vec3(0., 1., 0.),
		vec3(sin(theta), 0., cos(theta)));
}

mat3 RotateZ(float psi) {
	return mat3(
		vec3(cos(psi), -sin(psi), 0.),
		vec3(sin(psi), cos(psi), 0.),
		vec3(0., 0., 1.));
}


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

// this is what we will update and use for the next random number
// this must be set before using the random generator
uint currentSeed = 0;

uint genRand() {

	uint a = currentSeed;
	uint m = 0x63ad6ee6;
	uint b = 0x34cf359a;
	//uint mod = 0x939394;

	// this generates the random number
	//uint rand = (m * a + b) % mod;

	uint rand = (m * a + b);

	currentSeed = rand;

	return rand;
}


int genAngle() {
	uint one = genRand();

	// mod 180 and then sub by 90 to go from -90 degrees to +90 degrees
	return int((one % 180)) - 90;
}



vec3 genVec3() {
	uint one = genRand();
	uint two = genRand();
	uint three = genRand();


	int range = 100;
	float range_ = float(range);

	// convert it into a range that is usable
	vec3 conv = vec3(
		2.0 * ((one % range) / range_) - 1.0,
		2.0 * ((two % range) / range_) - 1.0,
		2.0 * ((three % range) / range_) - 1.0
		);

	return conv;
}

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

vec3 processLighting(PayLoad hit) {
	vec3 result = vec3(0,0,0);

	if (hit.didHit == false) {
		return result;
	}

	for (int i = 0; i < numLights; i++) {

		// get the light data
		Light light = Lights2.data[i];

		vec3 ray = light.position - hit.point;


		vec3 lightDir = normalize(ray);
		PayLoad hit2 = castRay(hit.point, lightDir, hit.index);
		float dist = mag(ray);
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
		}
		else {
			float dist_ = mag(light.position - hit.point);
			result += vec3((1.0 / (dist_ * dist_) * light.intensity / 255.0));
		}
	}

	return result;
}




void main() {


	// get the x and y values of the pixel we are working with
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

	// get the width and height from the workgroups values
	int width = int(gl_NumWorkGroups.x);
	int height = int(gl_NumWorkGroups.y);

	// now get uv values based on the width and height and the current workgroup values
	float v = (texelCoord.y - height / 2.0) / float(height);
	float u = (texelCoord.x - width / 2.0) / float(width);

	// get the pixel by camera transformation
	vec4 pixel2 = trans * vec4(u, v, .5, 1.0);
	vec3 pixel = {pixel2.x,pixel2.y,pixel2.z};

	// direction and origin data
	vec3 dir = pixel;
	vec3 orgin = cameraPosition;

	// assign the currentSeed value for random number generator
	currentSeed = texelCoord.x * texelCoord.y;


	// get the first point
	PayLoad hit = castRay(orgin, dir, -1);

	vec3 color = vec3(0);

	//color = vec3(1);

	if (hit.didHit == true) {

		// get the triangle hit and its normal
		Triangle t = triangles[hit.index];
		vec3 normal = t.n;

		// if the dot of the direction vector and normal is negative, flip the normal
		// so relfected vectors face the right direction
		if (dot(dir, normal) > 0) {
			normal *= -1;
		}


		// calculate the indirect lighting
		vec3 indirect = vec3(0.0);
		//int numVecs = 0;
		//for (int i = 0; i < numVecs; i++) {

		//	// generate a vector
		//	//vec3 testVec = genVec3();

		//	// using some math I did, we can get a new vector using 2 random valeus and solving
		//	// for the third to make sure the dot product is greater than 0

		//	//

		//	// get two new points
		//	int angleX = genAngle();
		//	int angleY = genAngle();
		//	int angleZ = genAngle();

		//	// normal must be a unit vector
		//	vec3 testVec = normal;



		//	vec3 newVec = vec3(testVec.x + angleX/90.0, testVec.y + angleY/90.0, testVec.z + angleZ/90.0);


		//	PayLoad hit_ = castRay(hit.point, newVec, hit.index);
		//	if (hit_.didHit) {

		//		indirect += processLighting(hit_) * hit_.color / 255.0;
		//		// do lighting calcs for the indirect lighting too
		//		//indirect += vec3(hit_.color/255.0);
		//	}

		//}

		//indirect /= float(numVecs);

		// direct lighting

		//vec3 direct = processLighting(hit);

		vec3 direct = vec3(1);

		//color = vec3(-dot(dir, normal));

		//color = indirect * hit.color;

		// (lighting) * albedo, which is the color of the triangle
		color = (indirect + direct) * hit.color/255.0;

		//color = direct;

		//color = indirect;

		//color = hit.color / 255.0;


	}


	// now process the lighting for that pixel
	//vec4 value = processLighting(hit);


	vec4 value = vec4(color, 1);

	// generate the random vec3
	//vec3 conv = genVec3();

	//conv = genVec3();

	//int mod = 50;



	//vec3 conv = vec3((genRand() % mod) /float(mod));


	//conv = conv * 2 - 1;

	//value = vec4(vec3(conv), 1);


	imageStore(imgOutput, texelCoord, value);

}