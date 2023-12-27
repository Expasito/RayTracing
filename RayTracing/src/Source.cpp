#define _CRT_SECURE_NO_WARNINGS
#include "Utils.h"
#include <iostream>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>
#include <thread>
#include <glm/gtx/string_cast.hpp>
#include <algorithm>
#include <execution>







float magnitude(glm::vec3 a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}


struct Triangle {
	// we have the point vector and its normal
	glm::vec3 p;
	glm::vec3 n;

	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 p3;

	glm::vec3 edge0, edge1, edge2;
	glm::vec3 color;

	float shininess;

	Triangle(glm::vec3 p1_, glm::vec3 p2_, glm::vec3 p3_, glm::vec3 color_, float shininess_) {
		p1 = p1_; p2 = p2_; p3 = p3_;
		// default to p1 here
		p = p1;
		n = normalize(cross(p1_ - p2_, p1_ - p3_));

		// precalculate these since our triangles do not move
		edge0 = p2 - p1;
		edge1 = p3 - p2;
		edge2 = p1 - p3;

		color = color_;

		shininess = shininess_;
	}

};

// keep track of all triangles possible
std::vector<Triangle> triangles;

// keep track of the trianlges that are visible to be hit
std::vector<Triangle> visibleTriangles;

struct Light {
	glm::vec3 position;
	float intensity;
};
std::vector<Light> lights;



std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << vec.x << " " << vec.y << " " << vec.z;
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& vec)
{
	os << vec.x << ' ' << vec.y << ' ' << vec.z << ' ' << vec.w;
	return os;
}




/*
* Camera class for where to send out rays
*/ 
class Camera {
public:

	glm::vec3 position;
	// in degrees
	glm::vec3 rotations;
	// for movement
	glm::vec3 direction;
	glm::vec3 right;
	glm::vec3 up;

	// up is y
	glm::vec3 worldUp = { 0,1,0 };
	float moveBaseSpeed = 1/128.0f;
	float moveSpeed=moveBaseSpeed;
	// make it so the basespeed and rotspaeed are related
	float rotBaseSpeed = moveBaseSpeed*3.14159265 * 2;
	float rotSpeed=rotBaseSpeed;

	// camera view plane sizes. Sort of determines the distance between each pixel
	// these are multipliers for the size of the viewing plane
	float camera_viewport_width = 1.0;
	float camera_viewport_height = 1.0;
	float camera_viewport_depth = .5;



	Camera() {
		position = { 0.0,0.0,0.0 };
		rotations = { 0.0,0.0,0.0 };
		// do special math for direction based on rotations
		direction = glm::normalize(glm::vec3(
			glm::sin(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y)),
			glm::sin(glm::radians(rotations.y)),
			glm::cos(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y))
		));
	}
	Camera(glm::vec3 pos, glm::vec3 rot) {
		position = pos;
		rotations = rot;
		// also do special math based on rotations
		direction = glm::normalize(glm::vec3(
			glm::sin(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y)),
			glm::sin(glm::radians(rotations.y)),
			glm::cos(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y))
		));

	}

	void translate(bool l, bool r, bool u, bool d, bool f, bool b) {
		
		
		// right of the camera is perpendicular vector of the direction of the camera
		// and the world up
		right = glm::normalize(glm::cross(worldUp,direction));
	
		// up vector is direction and right crossed
		up = glm::normalize(glm::cross(direction, right));




		// apply all transformations
		if (l) {
			position -= right * moveSpeed;
		}
		if (r) {
			position += right * moveSpeed;
		}
		if (u) {
			position += up * moveSpeed;
		}
		if (d) {
			position -= up * moveSpeed;
		}
		if (f) {
			position += direction * moveSpeed;
		}
		if (b) {
			position -= direction * moveSpeed;

		}
		

	}

	void rotate(bool l, bool r, bool u, bool d) {
		if (l) {
			rotations -= glm::vec3(rotSpeed, 0, 0);
		}
		if (r) {
			rotations += glm::vec3(rotSpeed, 0, 0);
		}
		if (u) {
			rotations += glm::vec3(0, rotSpeed, 0);
		}
		if (d) {
			rotations -= glm::vec3(0, rotSpeed, 0);
		}
		// prevent the vectors from flipping at 90 and -90 degrees
		if (rotations.y > 89.99) {
			rotations.y = 89.99;
		}
		if (rotations.y < -89.99) {
			rotations.y = -89.99;
		}
		// recalculate the direction
		direction = glm::normalize(glm::vec3(
			glm::sin(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y)),
			glm::sin(glm::radians(rotations.y)),
			glm::cos(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y))
		));


	}
private:

};




void shaderBuildStatus(unsigned int shader, int result) {
	std::cout << "Result" << result << "\n";
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)(alloca(length * (sizeof(char))));
		glGetShaderInfoLog(shader, length, &length, message);
		std::cout << "Failed to compile shader--\n";
		std::cout << message << "\n";
		glDeleteShader(shader);
		return;

	}
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}


//move left, right,...
bool left = false, right = false, up = false, down = false, forward = false, backward = false;


//look left, right,...
bool lleft = false, lright = false, lup = false, ldown = false;
void keycallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	left = glfwGetKey(window, GLFW_KEY_A);
	right = glfwGetKey(window, GLFW_KEY_D);
	up = glfwGetKey(window, GLFW_KEY_SPACE);
	down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
	forward = glfwGetKey(window, GLFW_KEY_W);
	backward = glfwGetKey(window, GLFW_KEY_S);

	lleft = glfwGetKey(window, GLFW_KEY_LEFT);
	lright = glfwGetKey(window, GLFW_KEY_RIGHT);
	lup = glfwGetKey(window, GLFW_KEY_UP);
	ldown = glfwGetKey(window, GLFW_KEY_DOWN);


}




struct PayLoad {
	glm::vec3 point;
	glm::vec3 color;
	float distance;
	Triangle* cur;
	bool didHit;
};

int castRayCalls = 0;
int softMathCalls = 0;
int hardMathCalls = 0;


PayLoad castRay(glm::vec3 orgin, glm::vec3 dir, Triangle* curr) {
	PayLoad closest = { {0,0,0},{0,0,0}, 1e9, NULL, false };
	bool found = false;


	// get the orthogonal basis. Because of how cross products work with 0,0,-1 and 0,1,0, we can derive the crossed vectors using the formulas below
	//glm::vec3 x = (dir);
	//glm::vec3 y = {-dir.y, dir.x, 0};
	//glm::vec3 z = {-dir.z,0,dir.x};


	//glm::mat3 B = { {x}, {y}, {z} };

	// now get the inverse(transpose also works) so we can convert to our new coordinate system
	//glm::mat3 BInv = glm::inverse(B);
	//glm::mat3 BInv = glm::transpose(B);


	//float dotZ = dot(z, z);
	//float dotY = dot(y, y);
	//float dotX = dot(x, x);



	for (Triangle& triangle : visibleTriangles) {
		if (&triangle == curr) {
			// skip the matched one if curr is a Triangle*
			continue;
		}


		// figure out the dot product for each point to the direction
		// if its negative then the traignle should not be visible so we can skip
		//float a = dot(glm::normalize(triangle.p1-orgin), glm::normalize(dir));
		//float b = dot(glm::normalize(triangle.p2-orgin), glm::normalize(dir));
		//float c = dot(glm::normalize(triangle.p3-orgin), glm::normalize(dir));

		//// get adjusted triangle coordinates
		//glm::vec3 tp1 = triangle.p1 - orgin;
		//glm::vec3 tp2 = triangle.p2 - orgin;
		//glm::vec3 tp3 = triangle.p3 - orgin;


		//// see how close the trianlge is to the ray
		//float a = dot(glm::normalize(tp1), glm::normalize(dir));
		//float b = dot(glm::normalize(tp2), glm::normalize(dir));
		//float c = dot(glm::normalize(tp3), glm::normalize(dir));

		//if (a < .5 && b < 0.5 && c < 0.5) {
		//	castRayCalls++;
		//	//std::cout << "SKIPPING!\n";
		//	continue;
		//}



		//// put back at origin
		//glm::vec3 p1 = BInv * (triangle.p1-orgin);
		//glm::vec3 p2 = BInv * (triangle.p2-orgin);
		//glm::vec3 p3 = BInv * (triangle.p3-orgin);

		





		//glm::vec3 p1 = {glm::dot(tp1,x) / dotX,  glm::dot(tp1,y) / dotY,  glm::dot(tp1,z) / dotZ };
		//glm::vec3 p2 = {glm::dot(tp2,x) / dotX,  glm::dot(tp2,y) / dotY,  glm::dot(tp2,z) / dotZ };
		//glm::vec3 p3 = {glm::dot(tp3,x) / dotX,  glm::dot(tp3,y) / dotY,  glm::dot(tp3,z) / dotZ };


		
		//// check if not possible intersection
		//if ((p1.y > 0 && p2.y > 0 && p3.y > 0) ||
		//	(p1.y < 0 && p2.y < 0 && p3.y < 0) ||
		//	(p1.z > 0 && p2.z > 0 && p3.z > 0) ||
		//	(p1.z < 0 && p2.z < 0 && p3.z < 0) ||
		//	(p1.x < 0 && p2.x < 0 && p3.x < 0)
		//	) {
		//	// do nothing becuase no chance of intersection
		//}
		//else {
		//	float t = dot(tp1, triangle.n) / dot(dir, triangle.n);
		//	if (t < 0) {
		//		continue;
		//	}
		//	glm::vec3 I = { orgin.x + t * dir.x,orgin.y + t * dir.y,orgin.z + t * dir.z };
		//
		//	if (t < closest.distance && t > 0.00001) {
		//		if (dot(triangle.n, cross(triangle.edge0, { I - triangle.p1 })) > 0.0 &&
		//			dot(triangle.n, cross(triangle.edge1, { I - triangle.p2 })) > 0.0 &&
		//			dot(triangle.n, cross(triangle.edge2, { I - triangle.p3 })) > 0.0
		//			) {
		//			closest = { I,triangle.color,t ,&triangle,true };
		//			found = true;
		//		}
		//	}
		//}


		
	



		float t = dot(triangle.p1 - orgin, triangle.n) / dot(dir, triangle.n);
		// //negative t values get filtered out already so do it now
		if (t < 0) {
			continue;
		}
		
		glm::vec3 I = { orgin.x + t * dir.x,orgin.y + t * dir.y,orgin.z + t * dir.z };
		
		if (t < closest.distance && t > 0.00001) {
			if (dot(triangle.n, cross(triangle.edge0, { I - triangle.p1 })) > 0.0 &&
				dot(triangle.n, cross(triangle.edge1, { I - triangle.p2 })) > 0.0 &&
				dot(triangle.n, cross(triangle.edge2, { I - triangle.p3 })) > 0.0
				) {
				closest = { I,triangle.color,t ,&triangle,true };
				found = true;
			}
		}

		
	}
	return closest;

}




void addTriangle(glm::vec3 translate_, glm::vec3 rotate, glm::vec3 scale_, glm::vec3 color, float shininess_) {
	// These are our points for the base triangle
	glm::vec4 p1 = glm::vec4(-1, -1, 0, 1);
	glm::vec4 p2 = glm::vec4(-1, 1, 0, 1);
	glm::vec4 p3 = glm::vec4(1, -1, 0, 1);


	glm::mat4 trans(1.0f);

	// These are all of our modifications. Order is translate, rotatex, rotatey, rotatez, scale
	glm::mat4 translate = glm::translate(trans, translate_);
	glm::mat4 rotatex = glm::rotate(trans, glm::radians(rotate.x), glm::vec3(1, 0, 0));
	glm::mat4 rotatey = glm::rotate(trans, glm::radians(rotate.y), glm::vec3(0, 1, 0));
	glm::mat4 rotatez = glm::rotate(trans, glm::radians(rotate.z), glm::vec3(0, 0, 1));
	glm::mat4 scale = glm::scale(trans,scale_);


	glm::mat4 mod = translate * rotatex * rotatey * rotatez * scale;


	p1 = mod * p1;
	p2 = mod * p2;
	p3 = mod * p3;



	triangles.push_back({
		{p1}, {p2}, {p3}, color, shininess_ }
	);
}


// this returns a normal facing the correct direction
glm::vec3 newNormal(glm::vec3 dir, glm::vec3 normal) {
	if (dir.x > 0) {
		normal.x = -abs(normal.x);
	}
	else {
		normal.x = abs(normal.x);
	}

	if (dir.y > 0) {
		normal.y = -abs(normal.y);
	}
	else {
		normal.y = abs(normal.y);
	}

	if (dir.z > 0) {
		normal.z = -abs(normal.z);
	}
	else {
		normal.z = abs(normal.z);
	}

	normal = glm::normalize(normal);
	return normal;
}

float dot(glm::vec3 a, glm::vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}


glm::vec3 processLighting(PayLoad hit, Camera* camera) {
	glm::vec3 color(0);

	// these do not change between lights
	glm::vec3 viewDir = glm::normalize(camera->position - hit.point);

	// calculate diffuse
	glm::vec3 norm = glm::normalize(hit.cur->n);

	for (Light l : lights) {

		color += (l.intensity * hit.color);
		continue;

		glm::vec3 lightDir = glm::normalize(l.position - hit.point);
		PayLoad hit2 = castRay(hit.point, lightDir, hit.cur);
		// this is the distance from the light to the point we hit in castRay
		float dist = magnitude(l.position - hit2.point);



		
		glm::vec3 reflectDir = glm::reflect(-lightDir, hit.cur->n);
		float shiny = 128;
		float spec = pow(float(fmax(dot(viewDir, reflectDir), 0)), shiny);

		glm::vec3 specular = (spec * glm::vec3(128, 128, 128));


		float shadow = 1;
		// and now we check if we hit the light first
		if (hit2.didHit == false || (hit2.didHit == true && hit2.distance > dist)) {
			shadow = 0;
		}


		

		float dot = fabs(glm::dot(lightDir, norm));
		glm::vec3 col = 1.0f/(dist) * hit.color;
		color += (col * dot + specular) * (1-shadow);

		//color += (col);


	}
	return color;
}


void getPixelData(int x, int y, int width, int height, unsigned char* data, glm::mat4 trans, Camera* camera) {
	glm::vec3 dir, origin;

	// convert camera plane pixel to into a range
	float v = (y - height / 2.0) / (float)height;
	float u = (x - width / 2.0) / (float)width;


	// figure out where our pixel is based on the transform matrix
	glm::vec4 pixel2 = trans * glm::vec4(u, v, camera->camera_viewport_depth, 1.0);


	// now get the first 3 elements of pixel2 for the new location of the pixel
	glm::vec3 pixel = { pixel2.x,pixel2.y,pixel2.z };
	dir = pixel;
	origin = camera->position;

	// First ray cast out
	PayLoad hit = castRay(origin, dir, NULL);

	int index = (y * width * 3) + x * 3;
	// This is the output color
	glm::vec3 color = glm::vec3(0);


	// If we hit, we need to check for shadows, else, just set to background color
	// and also send out a bunch of other rays
	if (hit.didHit) {


		// get the normal facing the correct direction relative to the viewer
		glm::vec3 normal = newNormal(dir, hit.cur->n);


		// so now our new normal is normal:

		// define the max bounces before we stop
		int maxBounces = 1;

		// shiny object so reflect
		if (hit.cur->shininess > .5) {
			int bounces = 0;
			glm::vec3 newDir = glm::normalize(glm::reflect(dir, normal));
			PayLoad hitN1 = castRay(hit.point, newDir, hit.cur);
			// keep iterating with new rays until a solid object
			while (hitN1.didHit && bounces < maxBounces && hitN1.cur->shininess > .5) {
				normal = newNormal(newDir, hitN1.cur->n);
				newDir = glm::normalize(glm::reflect(newDir, normal));
				hitN1 = castRay(hitN1.point, newDir, hitN1.cur);
				bounces++;
			}

			if (hitN1.didHit && bounces <= maxBounces) {
				color = processLighting(hitN1, camera);
				// dim by .8 to represent light loss
				float dim = bounces * 1;
				color = glm::vec3(color.x * dim, color.y * dim, color.z * dim);
			}
			else {

				color = glm::vec3(0, 0, 0);
			}


		}
		else {
			// set to opaque color
			color = processLighting(hit, camera);
		}



		if (color.x > 255) {
			color.x = 255;
		}
		if (color.y > 255) {
			color.y = 255;
		}
		if (color.z > 255) {
			color.z = 255;
		}

		data[index] = color.x;
		data[index + 1] = color.y;
		data[index + 2] = color.z;

	}
	// Did not hit so set to background color
	else {
		//data[index] = 135;
		//data[index + 1] = 206;
		//data[index + 2] = 235;

		data[index] = 0;
		data[index + 1] = 0;
		data[index + 2] = 0;
	}

}


void loadModel(const char* path) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		printf("File not found\n");
		exit(1);
	}

	while (true) {
		char c;
		float a1, a2, a3, b1, b2, b3, c1, c2, c3;
		int d1, d2, d3;
		int ret = fscanf(file, "(%f, %f, %f), (%f, %f, %f), (%f, %f, %f), (%d, %d, %d); ", &a1, &a2, &a3, &b1, &b2, &b3, &c1, &c2, &c3, &d1, &d2, &d3);
		//printf("Ret: %d", ret);
		if (ret == -1) {
			return;
		}
		if (ret == 0) {
			printf("Error with loading a model\n");
			exit(1);
		}
		//printf("Adding");
		addTriangle({ a1,a2,a3 }, { b1,b2,b3 }, { c1,c2,c3 }, {d1,d2,d3}, 0);
	}


}

class Edge;
class Node;

class Edge {
public:
	float weightX;
	float weightY;
	Node* start;
	Node* end;


	Edge(float weightX_, float weightY_, Node* start_, Node* end_) {
		weightX = weightX_;
		weightY = weightY_;
		start = start_;
		end = end_;
	}
};

class Node {


public:
	std::vector<Edge*>* edges;
	const char* name;
	short visited;
	Node(const char* name_) {
		name = name_;
		edges = new std::vector<Edge*>();
		visited = 0;
	}

	~Node() {
		delete edges;
	}

	void addEdge(Node* to, float weightX, float weightY) {
		edges->push_back(new Edge(weightX, weightY, this,to));
	}

	void log() {
		std::cout << "Node: " << this->name << " :: [ ";
		for (int i = 0; i < edges->size(); i++) {
			Edge* e = edges->at(i);
			std::cout << "(" <<  e->end->name << " : " << e->end->visited << " :: " << e->weightX << ", " << e->weightY << "),  ";
		}
		std::cout << "]\n\n";
	}
private:


};



void BFS(Node* start) {
	std::queue<Node*> queue;
	//std::priority_queue<Node*> queue;


	std::cout << "\n\n\nSTART BFS:\n";


	queue.push(start);

	while (queue.size() != 0) {
		//Node* q = queue.top();
		Node* q = queue.front();
		queue.pop();
		if (q->visited == 1) {
			continue;
		}

		std::cout << "VISITNG: " << q->name << " : " << q->visited << "\n";
		q->visited = 1;

		for (int i = 0; i < q->edges->size();i++) {
			Edge* e = q->edges->at(i);
			Node* end = (Node*)e->end;
			//std::cout << "     ";
			//end->log();
			//std::cout << "     " << end->name << " : " << end->visited << "\n";
			if (end->visited == 0) {
				//std::cout << "Adding: " << end->name << "\n";
				queue.push(end);
			}

		}
	}
}

void addEdge(Node* one, Node* two, float weightX, float weightY) {
	one->addEdge(two, weightX, weightY);
	two->addEdge(one, weightX, weightY);
}

struct
{
	bool operator()(const int l, const int r) const { return l > r; }
} customLess;


// just raw strings, very simple shaders so will leave as a long string
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 textCoord;\n"

"out vec2 TexCord;\n"
"void main()\n"
"{\n"
"   TexCord=textCoord;\n"
"   gl_Position = vec4(pos,1);\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"   FragColor = texture(texture1,TexCord);\n"
"}\n\0";


int main() {


	std::priority_queue<std::pair<int, int>> pq;

	pq.push(std::pair<int, int>(1, 4));
	pq.push(std::pair<int, int>(2, 4));
	pq.push(std::pair<int, int>(1, 5));
	pq.push(std::pair<int, int>(2, 5));




	while (pq.empty() == false) {
		std::pair<int, int> p = pq.top();
		pq.pop();

		std::cout << p.first << " " << p.second << "\n";
	}



	//exit(1);

	Node n("One");


	Node n2("Two");

	Node n3("Three");

	Node n4("Four");

	Node n5("Five");


	
	addEdge(&n, &n2, 10.0f, 10.0f);
	addEdge(&n2, &n3, 20.0f, 20.0f);
	addEdge(&n3, &n4, 5.0f, 5.0f);
	addEdge(&n, &n5, 15.0f, 15.0f);

	//n.log();
	//n2.log();
	//n3.log();
	//n4.log();
	//n5.log();

	//BFS(&n);


	//exit(1);


	// make camera a public variable
	Camera camera(glm::vec3(-10, 0, -10), glm::vec3(45, 0, 45));
	//Camera camera(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0));
	//Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));



	// wall along z axis
	addTriangle({0,0,10 }, { 0,0,0 }, { 10,10,10 }, {255,0,0}, 0);
	addTriangle({0,0,10}, {0,0,180}, {10,10,10}, {255,0,0}, 0);
	addTriangle({ 0,0,-10 }, { 0,0,0 }, { 10,10,10 }, { 255,0,128 },    0);
	addTriangle({ 0,0,-10 }, { 0,0,180 }, { 10,10,10 }, { 255,0,128 },    0);

	// wall along x axis
	addTriangle({ 10,0, 0 }, { 0,90,0 }, { 10,10,10 }, { 0,255,0 }, 0);
	addTriangle({ 10,0, 0 }, { 0,90,180 }, { 10,10,10 }, { 0,255,0 }, 0);
	addTriangle({ -10,0, 0 }, { 0,90,0 }, { 10,10,10 }, { 0,255,128 },     0);
	addTriangle({ -10,0, 0 }, { 0,90,180 }, { 10,10,10 }, { 0,255,128 },     0);

	// wall along y axis
	addTriangle({ 0,-10,0 }, { 90,0,0 }, { 10,10,10 }, { 0,0,255 }, 0);
	addTriangle({ 0,-10,0 }, { 90,0,180 }, { 10,10,10 }, { 0,0,255 }, 0);
	addTriangle({ 0,10,0 }, { 90,0,0 }, { 10,10,10 }, { 255,0,255 }, 0);
	addTriangle({ 0,10,0 }, { 90,0,180 }, { 10,10,10 }, { 255,0,255 }, 0);


	addTriangle({ 0,-2,0 }, { 90,0,0 }, { 5,5,5 }, { 0,255,255 }, 0);
	addTriangle({ 0,-2,0 }, { 90,0,180 }, { 5,5,5 }, { 0,255,255 }, 0);


	addTriangle({ 4,4,0 }, { 90,0,0 }, { .5,.5,.5 }, { 0,255,255 }, 0);
	addTriangle({ 4,4,0 }, { 90,0,180 }, { .5,.5,.5 }, { 0,255,255 }, 0);


	addTriangle({ 5,0,0 }, { 90,90,0 }, { 1,1,1 }, { 255,255,0 }, 0);
	addTriangle({ 5,0,0 }, { 90,90,180 }, { 1,1,1 }, { 255,255,0 }, 0);



	addTriangle({ 0,0,-40 }, { 0,0,0 }, { 20,20,20 }, { 255,255,255 }, 0);
	addTriangle({ 0,0,-40 }, { 0,0,180 }, { 20,20,20 }, { 255,255,255 }, 0);




	//addTriangle({0,0,0}, {0,0,0}, {1,1,1}, {255,255,255}, 0);




	
	lights.push_back({ {0,8,0},512 });
	//lights.push_back({ {5,8,5} , 64 });
	//lights.push_back({ {9.99,9.99,9.99},128 });

	

	// This is the size for the number of rays to cast out. So length*width is the total.
	int width = 800;
	int height = 800;
	// this is the size of the output(display) window
	int win_width = 800;
	int win_height = 800;





	// opengl stuff here
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(win_width, win_height, "Window", NULL, NULL);
	if (window == NULL) {
		std::cout << "failed to create\n";
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initalize GLAD\n");
		return -1;
	}

	glViewport(0, 0, win_width, win_height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, keycallback);



	//
	//
	//
	//
	//


	char* shaderCode = (char*)malloc(sizeof(char) * 2000);
	FILE* f = fopen("src/Compute.shader", "r");

	char ch;
	int err;
	int ind = 0;
	while (true) {
		err = fscanf(f, "%c", &ch);
		if (err == -1) {
			break;
		}
		//printf("%c", ch);
		shaderCode[ind] = ch;
		ind++;
	}
	shaderCode[ind] = '\n';
	ind++;
	shaderCode[ind] = 0;


	printf("%s", shaderCode);
	//exit(1);

	// create all shaders
	uint32_t compute = genShader(GL_COMPUTE_SHADER, &shaderCode);
	uint32_t vertex = genShader(GL_VERTEX_SHADER, (char**)(& vertexShaderSource));
	uint32_t fragment = genShader(GL_FRAGMENT_SHADER, (char**)(& fragmentShaderSource));

	// create the program based on the shaders provided
	//std::vector<uint32_t> shaders;
	//shaders.push_back(compute);
	//shaders.push_back(vertex);
	//shaders.push_back(fragment);
	uint32_t programCompute = genProgram({compute});
	uint32_t programRender = genProgram({vertex, fragment});
	//program = glCreateProgram();
	//glAttachShader(program, compute);
	//glAttachShader(program, vertex);
	//glAttachShader(program, fragment);


	//glLinkProgram(programCompute);
	glUseProgram(programCompute);
	//std::cout << program << "\n";

	//exit(1);


	const int twidth = 800;
	const int theight = 800;


	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, twidth, theight, 0, GL_RGBA,
		GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glUseProgram(programCompute);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glDispatchCompute((uint32_t)twidth, (uint32_t)theight, 1);


	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//float test[twidth * theight*4];
	////glGetTexImage(0, 0, GL_RGBA32F, GL_FLOAT, sizeof(float) * 99, test);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, test);
	////exit(1);

	//for (int i = 0; i < twidth*theight*4; i+=4) {
	//	std::cout << test[i] << " " << test[i+1] << " " << test[i+2] << " " << test[i+3] << "\n";
	//}
	//exit(1);

	//exit(1);

	//glDispatchCompute(10, 10, 1);



	//
	//
	//
	//
	//


	glUseProgram(programRender);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//order: x,y,z,u,v
	float vertices[] = {
	-1,-1,0,0,0,
	-1,1,0,0,1,
	1,-1,0,1,0,
	-1,1,0,0,1,
	1,1,0,1,1,
	1,-1,0,1,0
	};
	// texture stuff
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);



	// this array will hold the color for each pixel we send a ray out for
	int const channels = 3;
	// width and height are defined above
	unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * width * height * channels);

	// fill the array with zeros
	int c = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
				data[c++] = 0;
				data[c++] = 0;
				data[c++] = 0;
			

		}
	}



	//unsigned int texture;
	//glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


	glGenerateMipmap(GL_TEXTURE_2D);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// do position first
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
	// texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAO);

	
	// stuff for times
	std::chrono::steady_clock::time_point begin;
	std::chrono::steady_clock::time_point end;
	float milis=0;

	// these are the vectors for the for_each loops so they can iterate
	std::vector<int> horizontal(width), vertical(height);
	//std::iota(std::begin(horizontal), std::end(horizontal), -width / 2);
	std::iota(std::begin(horizontal), std::end(horizontal), 0);

	//std::iota(std::begin(vertical), std::end(vertical), -height / 2);
	std::iota(std::begin(vertical), std::end(vertical), 0);

	// For performance, we will update either the top or bottom per frame
	bool writeTop = false;

	// keep track of which triangle we are on
	int cntr = 0;
	
	
	// now for the run loop
	while (!glfwWindowShouldClose(window)) {

		// start clock
		begin = std::chrono::steady_clock::now();
		

		// clear the screen
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);



		// reset counter
		castRayCalls = 0;
		softMathCalls = 0;
		hardMathCalls = 0;


		// rotate the camrea plane pixel around the camera's location
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians((float)camera.rotations.x), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(-(float)camera.rotations.y), glm::vec3(1.0f, 0.0f, 0.0f));

		
		// now load triangle data

		// remove all previous triangles
		visibleTriangles.clear();


		// now filter and add
		for (Triangle& const t: triangles) {
			cntr++;
			/*if (cntr > 1) {
				cntr = 0;
			}*/
			visibleTriangles.push_back(t);
		}


		//// keep this just in case
		//for (int y = 0; y <height; y++) {
		//	for (int x = 0; x < width; x++) {
		//		getPixelData(x, y, width, height, data, trans, &camera);
		//	}
		//}

		// parallelize to make it faster
		//std::for_each(std::execution::par, std::cbegin(vertical), std::cend(vertical),
		//	[data,width,height,&camera,horizontal, trans
		//	](int y) {
		//		std::for_each(std::execution::par, std::cbegin(horizontal), std::cend(horizontal),
		//			[horizontal,y,data,width,height,&camera, trans](int x) {
		//				getPixelData(x, y, width, height, data, trans, &camera);
		//			}
		//		);
		//	}
		//);

		glUseProgram(programCompute);

		glUniform1f(glGetUniformLocation(programCompute, "testFloat"), 2.0f);
		glUniform3f(glGetUniformLocation(programCompute, "triangle.point"), .25f, .5f, 1.0f);


		glDispatchCompute((uint32_t)twidth, (uint32_t)theight, 1);


		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glUseProgram(programRender);
		







		// update image for opengl to draw
		glBindTexture(GL_TEXTURE_2D, texture);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// draw image
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();

		//std::cout << glm::to_string(camera.position) << "\n";

		// move camera
		camera.rotate(lleft, lright, lup, ldown);
		camera.translate(left, right, up, down, forward, backward);

		end = std::chrono::steady_clock::now();

		// get delta time and frame data
		milis = (end - begin).count() / 1000000.0;
		std::cout << "Time difference = " << milis << "[ms]" << " FPS: " << 1000.0 / milis << " and had : " << castRayCalls << " castRay calls in this frame\n";
		//std::cout << " SoftCalls: " << softMathCalls << " HardCalls: " << hardMathCalls << " Frac: " << (float)hardMathCalls / (float)softMathCalls << "\n";



		// convert miliseconds to a delta time
		camera.moveSpeed = camera.moveBaseSpeed*milis;
		camera.rotSpeed = camera.rotBaseSpeed*milis;
		//std::cout << "Camera speed: " << camera.moveSpeed << "\n";
	}

	glfwTerminate();


}