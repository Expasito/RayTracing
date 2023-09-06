#define _CRT_SECURE_NO_WARNINGS
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

	Triangle(glm::vec3 p1_, glm::vec3 p2_, glm::vec3 p3_, glm::vec3 color_) {
		p1 = p1_; p2 = p2_; p3 = p3_;
		// default to p1 here
		p = p1;
		n = cross(p1_ - p2_, p1_ - p3_);

		// precalculate these since our triangles do not move
		edge0 = p2 - p1;
		edge1 = p3 - p2;
		edge2 = p1 - p3;

		color = color_;
	}

};
std::vector<Triangle> triangles;

struct Light {
	glm::vec3 position;
	float intensity;
};
std::vector<Light> lights;



std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << vec.x << ' ' << vec.y << ' ' << vec.z;
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
	float moveBaseSpeed = 15.0;
	float moveSpeed=moveBaseSpeed;
	// make it so the basespeed and rotspaeed are related
	float rotBaseSpeed = moveBaseSpeed*3.14159265;
	float rotSpeed=rotBaseSpeed;

	// camera view plane sizes. Sort of determines the distance between each pixel
	// these are multipliers for the size of the viewing plane
	float camera_viewport_width = 2.0;
	float camera_viewport_height = 2.0;
	float camera_viewport_depth = 1.0;



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


//void castRay(unsigned char* data, int i, int j, int width, int height, Camera* camera, std::vector<Triangle>* triangles) {
//	// reference for closest triangle
//	Triangle out_tri;
//	Object::hit out_hit;
//	double mint = 1000000000;
//	bool found = false;
//	
//
//	// convert camera plane pixel to into a range
//	float u = (j) / (float)height * camera->camera_viewport_width;
//	float v = (i) / (float)width * camera->camera_viewport_height;
//
//	// rotate the camrea plane pixel around the camera's location
//	glm::mat4 trans = glm::mat4(1.0f);
//	trans = glm::rotate(trans, glm::radians((float)camera->rotations.x), glm::vec3(0.0f, 1.0f, 0.0f));
//	trans = glm::rotate(trans, glm::radians(-(float)camera->rotations.y), glm::vec3(1.0f, 0.0f, 0.0f));
//	glm::vec4 pixel2 = trans * glm::vec4(u, v, camera->camera_viewport_depth, 1.0);
//
//	// now get the first 3 elements of pixel2 for the new location of the pixel
//	glm::vec3 pixel = { pixel2.x,pixel2.y,pixel2.z };
//
//	// define t out here so we can use it outside of the for loop scope
//	double t;
//	Triangle tri;
//	Object::hit hit;
//
//	for (int k = 0; k < triangles->size(); k++) {
//		tri = (*triangles)[k];
//
//		// get the distance for this triangle
//		hit = tri.getDist(camera->position, pixel);
//
//		// update the closes triangle here
//		if (hit.t > camera->camera_viewport_depth && hit.t < mint) {
//			out_tri = tri;
//			out_hit = hit;
//			mint = hit.t;
//			found = true;
//
//		}
//
//	}
//
//
//	// this is where in the array to set the colors
//	int index = (i + height / 2) * width * 3 + (j + width / 2) * 3;
//
//	if (found) {
//		Object::payload payLoad = out_tri.getPayload(hit.position);
//		data[index] = payLoad.color.x;
//		data[index + 1] = payLoad.color.y;
//		data[index + 2] = payLoad.color.z;
//
//
//	}
//	else {
//		// set background color
//		data[index] = 0;
//		data[index + 1] = 0;
//		data[index + 2] = 0;
//
//
//
//	}
//}


struct PayLoad {
	glm::vec3 point;
	glm::vec3 color;
	float distance;
	Triangle* cur;
	bool didHit;
};

int castRayCalls = 0;

PayLoad castRay(glm::vec3 orgin, glm::vec3 dir, Triangle* curr) {
	// add 1 to the counter
	castRayCalls += 1;
	PayLoad closest = { {0,0,0},{0,0,0},1e9,NULL,false };
	bool found = false;
	for (Triangle& triangle : triangles) {
		if (&triangle == curr) {
			// skip the matched one if curr is a Triangle*
			continue;
		}

		float t = dot(triangle.p1 - orgin, triangle.n) / dot(dir, triangle.n);
		// negative t values get filtered out already so do it now
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


#include <algorithm>
#include <execution>

void addTriangle(glm::vec3 translate_, glm::vec3 rotate, glm::vec3 scale_, glm::vec3 color) {
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
		{p1}, {p2}, {p3}, color }
	);
}

int main() {
	


	// make camera a public variable
	Camera camera(glm::vec3(0, 0, -10), glm::vec3(0, 0, 0));


	addTriangle({0,0,10 }, { 0,0,0 }, { 10,10,10 }, {255,0,0});
	addTriangle({0,0,10}, {0,0,180}, {10,10,10}, {255,0,0});

	addTriangle({ 10,0, 0 }, { 0,90,0 }, { 10,10,10 }, { 0,255,0 });
	addTriangle({ 10,0, 0 }, { 0,90,180 }, { 10,10,10 }, { 0,255,0 });

	addTriangle({ 0,-10,0 }, { 90,0,0 }, { 10,10,10 }, { 0,0,255 });
	addTriangle({ 0,-10,0 }, { 90,0,180 }, { 10,10,10 }, { 0,0,255 });




	


	//// Left wall
	//triangles.push_back( {
	//	{-1,-1,-1},{-1,1,-1},{0,-1,-1},{255,255,255}
	//});

	//triangles.push_back({
	//	{-1,1,-1},{0,1,-1},{0,-1,-1},{255,255,255}
	//});

	//
	//// Right wall
	//triangles.push_back(
	//	{ {0,-1,-1},{0,-1,1},{0,1,1},{0,200,0} }
	//);

	//triangles.push_back(
	//	{ {0,-1,-1},{0,1,0},{0,1,1},{0,200,0} }
	//);



	//triangles.push_back({ {-1,-1,-1},{0,1,1},{1,-1,-1}, {0,0,0} });

	int len = 50;
	int heigh = 1;
	// random floor
	//triangles.push_back({
	//	{-len,-heigh,-len},
	//	{-len,-heigh,len},
	//	{len,-heigh,len},
	//	{0,255,0}
	//	});

	//triangles.push_back({
	//	{len,-heigh,len},
	//	{len,-heigh,-len},
	//	{-len,-heigh,-len},
	//	{0,0,255}
	//	});

	//triangles.push_back({
	//	{-len,-heigh-1,-len},
	//	{-len,-heigh-1,len},
	//	{len,-heigh-1,len},
	//	{0,255,0}
	//	});

	//triangles.push_back({
	//	{len,-heigh-1,len},
	//	{len,-heigh-1,-len},
	//	{-len,-heigh-1,-len},
	//	{0,0,255}
	//	});

	lights.push_back(
		{ {0,5,0},128 });

	//lights.push_back({ {10,10,10},128 });
	//srand(0);
	int max_dist = 5;
	int max_scale = 2;

	for (int i = 0; i < 0; i++) {
		float xrad = rand() / (float)RAND_MAX * 360;
		float yrad = rand() / (float)RAND_MAX * 360;
		float zrad = rand() / (float)RAND_MAX * 360;
		glm::mat4 trans_ = glm::mat4(1);
		glm::mat4 rotx_ = glm::rotate(trans_, glm::radians(xrad), glm::vec3(1, 0, 0));
		glm::mat4 roty_ = glm::rotate(trans_, glm::radians(yrad), glm::vec3(0, 1, 0));
		glm::mat4 rotz_ = glm::rotate(trans_, glm::radians(zrad), glm::vec3(0, 0, 1));

		glm::mat4 scale_ = glm::scale(trans_, glm::vec3(rand() % max_scale - max_scale / 2, rand() % max_scale - max_scale / 2, rand() % max_scale - max_scale / 2));

		glm::mat4 translate_ = glm::translate(trans_, glm::vec3(rand() % max_dist - max_dist/2, rand() % max_dist - max_dist/2, rand() % max_dist -max_dist/2));


		glm::vec4 p1 = glm::vec4(-1, -1, 0, 1);
		glm::vec4 p2 = glm::vec4(0, 1, 0, 1);
		glm::vec4 p3 = glm::vec4(1, -1, 0, 1);

		glm::mat4 mod = translate_ * rotx_ * roty_ * rotz_ * scale_;

		p1 = mod * p1;
		p2 = mod * p2;
		p3 = mod * p3;

		triangles.push_back({
			{p1}, {p2}, {p3}, { 128,128,128 } }
		);
	}

	

	// generate a lot of random triangles
	//for (int i = 0; i < 40; i++) {
	//	triangles.push_back({ { (float)rand() / (float)RAND_MAX * 100 - 50, (float)rand() / (float)RAND_MAX * 100 - 50, (float)rand() / (float)RAND_MAX * 100 - 50 },
	//		{ (float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50},
	//		{ (float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50 },
	//		{100,200,10} });
	//}

	

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


	unsigned int vertexShader;
	unsigned int fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	int result;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	shaderBuildStatus(vertexShader, result);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	shaderBuildStatus(fragmentShader, result);

	unsigned int program;
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glUseProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

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



	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


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
	std::iota(std::begin(horizontal), std::end(horizontal), -width / 2);
	std::iota(std::begin(vertical), std::end(vertical), -height / 2);

	// For performance, we will update either the top or bottom per frame
	bool writeTop = false;


	// now for the run loop
	while (!glfwWindowShouldClose(window)) {

		// start clock
		

		// clear the screen
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);


		begin = std::chrono::steady_clock::now();

	

		//// new execution
		//std::for_each(std::execution::par, std::cbegin(vertical), std::cend(vertical),
		//	[data,width,height,&camera,&triangles,horizontal
		//	](int y) {
		//		std::for_each(std::execution::par, std::cbegin(horizontal), std::cend(horizontal),
		//			[horizontal,y,data,width,height,&camera,&triangles](int x) {
		//				castRay(data, y, x, width, height, &camera, &triangles);
		//			}
		//		);
		//	}
		//);

		// reset counter
		castRayCalls = 0;


		////int index = 0;
		int startY = 0;
		int endY = height;

		//if (writeTop == true) {
		//	startY = 0;
		//	endY = height;
		//	//index = 0;
		//}
		//else {
		//	startY = 1;
		//	endY = height;
		//	//index = height / 2 * 3;
		//}
		//writeTop = !writeTop;
		

		
		// Currently, this updates every other pixel and starts at 0 or 1 
		// So it should look better than updating the top half or bottom half

		// rotate the camrea plane pixel around the camera's location
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians((float)camera.rotations.x), glm::vec3(0.0f, 1.0f, 0.0f));
		trans = glm::rotate(trans, glm::radians(-(float)camera.rotations.y), glm::vec3(1.0f, 0.0f, 0.0f));

		// For now, we will not to test performance better
		startY = 0;
		for (int y = startY; y <endY; y++) {
			for (int x = 0; x < width; x++) {
				glm::vec3 dir, origin;

				// convert camera plane pixel to into a range
				float v = (y - height / 2) / (float)height;
				float u = (x - width / 2) / (float)width;


				// figure out where our pixel is based on the transform matrix
				glm::vec4 pixel2 = trans * glm::vec4(u, v, camera.camera_viewport_depth, 1.0);


				// now get the first 3 elements of pixel2 for the new location of the pixel
				glm::vec3 pixel = { pixel2.x,pixel2.y,pixel2.z };
				dir = pixel;
				origin = camera.position;

				// First ray cast out
				PayLoad hit = castRay(origin, dir, NULL);

				int index = (y * width * 3) + x * 3;
				// This is the output color
				glm::vec3 color = glm::vec3(0);

				// If we hit, we need to check for shadows, else, just set to background color
				// and also send out a bunch of other rays
				if (hit.didHit) {

					// This is the normal to the triangle
					// So now we need to generate a bunch of rays going randomly out
					glm::vec3 normal = hit.cur->n;

					// But also, we need to make sure the normal is facing the opposite direction of the ray hitting it

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

					glm::vec3 colorOut(0);
					int maxI = 0;

					// This is the furthest angle a ray can reflect at
					int maxAngle = 90;
					for (int i = 0; i < maxI; i++) {
						// We will need to generate angles upto 90 degrees off of the normal
						float xchange = (rand() % (maxAngle*2)) - maxAngle;
						float ychange = (rand() % (maxAngle*2)) - maxAngle;
						float zchange = (rand() % (maxAngle*2)) - maxAngle;

						glm::mat4 trans__(1);
						glm::mat4 xrot = glm::rotate(trans__, glm::radians(xchange), glm::vec3(1, 0, 0));
						glm::mat4 yrot = glm::rotate(trans__, glm::radians(ychange), glm::vec3(0, 1, 0));
						glm::mat4 zrot = glm::rotate(trans__, glm::radians(zchange), glm::vec3(0, 0, 1));

						trans__ = xrot * yrot * zrot;

						// This is the new direction
						glm::vec3 newNormal = glm::normalize(glm::vec4(normal.z, normal.y, normal.z, 1) * trans__);

						PayLoad hitN = castRay(hit.point, newNormal, hit.cur);

						if (hitN.didHit) {
							colorOut.x += hitN.color.x / hitN.distance / hitN.distance;
							colorOut.z += hitN.color.y / hitN.distance / hitN.distance;
							colorOut.z += hitN.color.z / hitN.distance / hitN.distance;


						}
					}
					//colorOut /= maxI;
					
					color.x = (hit.color.x + colorOut.x);
					color.y = (hit.color.y + colorOut.y);
					color.z = (hit.color.z + colorOut.z);

					//color.x = hit.color.x + colorOut.x;


					

					//for (Light l : lights) {
					//	PayLoad hit2 = castRay(hit.point, l.position- hit.point, hit.cur);
					//	float dist = magnitude(l.position - hit.point);
					//	//std::cout << dist << "\n";
					//	if (hit2.didHit==false ||(hit2.didHit==true && dist < hit2.distance)) {
					//		color += (l.intensity * hit.color) / (dist * dist);
					//		//color += l.intensity/dist * hit.color;
					//		//color += glm::vec3(.2 * 255, .2 * 255, .2*255);
					//		//std::cout << hit.color * l.intensity << "/n";
					//	}
					//}
					//color = glm::vec3(255, 255, 255);

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
		}
		end = std::chrono::steady_clock::now();
		//writeTop = true;





		// update image for opengl to draw
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// draw image
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();

		// move camera
		camera.rotate(lleft, lright, lup, ldown);
		camera.translate(left, right, up, down, forward, backward);


		// get delta time and frame data
		milis = (end - begin).count() / 1000000.0;
		std::cout << "Time difference = " << milis << "[ms]" << " FPS: " << 1000.0 / milis << " and had : " << castRayCalls << " castRay calls in this frame\n";



		// convert miliseconds to a delta time
		camera.moveSpeed = camera.moveBaseSpeed/milis;
		camera.rotSpeed = camera.rotBaseSpeed/milis;
	}

	glfwTerminate();


}