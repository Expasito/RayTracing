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




std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << vec.x << ' ' << vec.y << ' ' << vec.z;
	return os;
}


/*
* This is the base object class for triangles and spheres
*/
class Object {
public:
	typedef struct hit {
		glm::vec3 position;
		double t;
	};

	virtual hit getDist(glm::vec3 point, glm::vec3 dir) = 0;

	typedef struct payload {
		glm::vec3 color;
	};

	virtual payload getPayload(glm::vec3 point)=0;

	
private:
};


/*
* This is just a plane for the triangle based on a point and normal to it
*/
struct plane {
	glm::vec3 n;
	glm::vec3 p;
};



class Triangle : public Object {
public:
	glm::vec3 p1 = { 0,0,0 };
	glm::vec3 p2 = { 0,0,0 };
	glm::vec3 p3 = { 0,0,0 };

	// this is for other calculations
	plane p;
	glm::vec3 normal;
	glm::vec3 color;

	Triangle() {
		p1 = { -1,-1,0 };
		p2 = { 1,-1,0 };
		p3 = { 0,1,0 };

		color = { 100,100,100 };

		glm::vec3 b = p1 - p2;
		glm::vec3 c = p1 - p3;
		normal = glm::cross(b, c);

		// this is our plane equation
		p = { normal,p1 };
		
	}
	Triangle(glm::vec3 p1_, glm::vec3 p2_, glm::vec3 p3_, glm::vec3 color_) {
		p1 = p1_;
		p2 = p2_;
		p3 = p3_;

		color = color_;

		glm::vec3 b = p1 - p2;
		glm::vec3 c = p1 - p3;
		normal = glm::cross(b, c);

		// this is our plane equation
		p = { normal,p1 };
	}
	hit getDist(glm::vec3 orgin, glm::vec3 dir) {

		// get distance along line
		double t = glm::dot((p1 - orgin), normal) / glm::dot(dir, normal);

		// get location of where the ray hits the plane
		glm::vec3 I = { orgin.x + t * dir.x,orgin.y + t * dir.y,orgin.z + t * dir.z };
		
		// triangle intersection formula
		glm::vec3 edge0 = p2 - p1;
		glm::vec3 edge1 = p3 - p2;
		glm::vec3 edge2 = p1 - p3;
		glm::vec3 C0 = I - p1;
		glm::vec3 C1 = I - p2;
		glm::vec3 C2 = I - p3;

		// check if the point is inside of the triangle
		bool inside =
			glm::dot(normal, glm::cross(edge0, C0)) > 0.0 &&
			glm::dot(normal, glm::cross(edge1, C1)) > 0.0 &&
			glm::dot(normal, glm::cross(edge2, C2)) > 0.0;

		if (inside) {
			return {I,t};
		}
		else {
			return { I,-1 };
		}

	}

	payload getPayload(glm::vec3 position) {
		return {color};
	}


private:

};

/*
* Will implement later. Used for spheres (obviously)
*/ 
class Sphere : public Object {
public:
	Sphere() {

	};
	hit getDist(glm::vec3 point, glm::vec3 dir) {
		return { glm::vec3(1,1,1),1 };
	}

	payload getPayload(glm::vec3 position) {
		return { position };
	}

private:
};


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
	float moveBaseSpeed = 10.0;
	float moveSpeed=moveBaseSpeed;
	float rotBaseSpeed = 40.0;
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


void castRay(unsigned char* data, int i, int j, int width, int height, Camera* camera, std::vector<Triangle>* triangles) {
	// reference for closest triangle
	Triangle out_tri;
	Object::hit out_hit;
	double mint = 1000000000;
	bool found = false;
	

	// convert camera plane pixel to into a range
	float u = (j) / (float)height * camera->camera_viewport_width;
	float v = (i) / (float)width * camera->camera_viewport_height;

	// rotate the camrea plane pixel around the camera's location
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians((float)camera->rotations.x), glm::vec3(0.0f, 1.0f, 0.0f));
	trans = glm::rotate(trans, glm::radians(-(float)camera->rotations.y), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec4 pixel2 = trans * glm::vec4(u, v, camera->camera_viewport_depth, 1.0);

	// now get the first 3 elements of pixel2 for the new location of the pixel
	glm::vec3 pixel = { pixel2.x,pixel2.y,pixel2.z };

	// define t out here so we can use it outside of the for loop scope
	double t;
	Triangle tri;
	Object::hit hit;

	for (int k = 0; k < triangles->size(); k++) {
		tri = (*triangles)[k];

		// get the distance for this triangle
		hit = tri.getDist(camera->position, pixel);

		// update the closes triangle here
		if (hit.t > camera->camera_viewport_depth && hit.t < mint) {
			out_tri = tri;
			out_hit = hit;
			mint = hit.t;
			found = true;

		}

	}


	// this is where in the array to set the colors
	int index = (i + height / 2) * width * 3 + (j + width / 2) * 3;

	if (found) {
		Object::payload payLoad = out_tri.getPayload(hit.position);
		data[index] = payLoad.color.x;
		data[index + 1] = payLoad.color.y;
		data[index + 2] = payLoad.color.z;


	}
	else {
		// set background color
		data[index] = 0;
		data[index + 1] = 0;
		data[index + 2] = 0;



	}
}
#include <algorithm>
#include <execution>

int main() {
	


	// make camera a public variable
	Camera camera(glm::vec3(0, 0, -10), glm::vec3(0, 0, 0));


	// vector of all triangles to draw
	std::vector<Triangle> triangles;

	triangles.push_back({ {-1,-1,1},{-1,1,1},{1,-1,1}, {0,0,100} });

	int len = 10;
	// random floor
	triangles.push_back({
		{-len,-len,-len},
		{-len,-len,len},
		{len,-len,len},
		{0,100,0}
		});

	triangles.push_back({
		{len,-len,len},
		{len,-len,-len},
		{-len,-len,-len},
		{100,0,0}
		});

	// generate a lot of random triangles
	//for (int i = 0; i < 40; i++) {
	//	triangles.push_back({ { (float)rand() / (float)RAND_MAX * 100 - 50, (float)rand() / (float)RAND_MAX * 100 - 50, (float)rand() / (float)RAND_MAX * 100 - 50 },
	//		{ (float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50},
	//		{ (float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50 ,(float)rand() / (float)RAND_MAX * 100 - 50 },
	//		{100,200,10} });
	//}

	

	// This is the size for the number of rays to cast out. So length*width is the total.
	int width = 800;
	int height = 600;
	// this is the size of the output(display) window
	int win_width = 1200;
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


	// now for the run loop
	while (!glfwWindowShouldClose(window)) {

		// start clock
		begin = std::chrono::steady_clock::now();
		

		// clear the screen
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);



	

		// new execution
		std::for_each(std::execution::par, std::cbegin(vertical), std::cend(vertical),
			[data,width,height,&camera,&triangles,horizontal
			](int y) {
				std::for_each(std::execution::par, std::cbegin(horizontal), std::cend(horizontal),
					[horizontal,y,data,width,height,&camera,&triangles](int x) {
						castRay(data, y, x, width, height, &camera, &triangles);
					}
				);
			}
		);

		



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
		end = std::chrono::steady_clock::now();
		milis = (end - begin).count() / 1000000.0;
		std::cout << "Time difference = " << milis << "[ms]" << " FPS: " << 1000.0 / milis << "\n";

		// convert miliseconds to a delta time
		camera.moveSpeed = camera.moveBaseSpeed * milis/1000.0;
		camera.rotSpeed = camera.rotBaseSpeed * milis/1000.0;
	}

	glfwTerminate();


}