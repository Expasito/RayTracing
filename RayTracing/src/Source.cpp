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
//#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>




std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << vec.x << ' ' << vec.y << ' ' << vec.z;
	return os;
}

class Object {
public:
	virtual double getDist(glm::vec3 point, glm::vec3 dir) = 0;
private:
};

struct plane {
	glm::vec3 n;
	glm::vec3 p;
};

struct ray {
	glm::vec3 o;
	glm::vec3 d;
};

class Triangle : public Object {
public:
	glm::vec3 p1 = { 0,0,0 };
	glm::vec3 p2 = { 0,0,0 };
	glm::vec3 p3 = { 0,0,0 };

	// this is for other calculations
	plane p;
	glm::vec3 normal;

	Triangle() {
		p1 = { -1,-1,0 };
		p2 = { 1,-1,0 };
		p3 = { 0,1,0 };

		glm::vec3 b = p1 - p2;
		glm::vec3 c = p1 - p3;
		normal = glm::cross(b, c);

		// this is our plane equation
		p = { normal,p1 };
		
	}
	Triangle(glm::vec3 p1_, glm::vec3 p2_, glm::vec3 p3_) {
		p1 = p1_;
		p2 = p2_;
		p3 = p3_;

		glm::vec3 b = p1 - p2;
		glm::vec3 c = p1 - p3;
		normal = glm::cross(b, c);

		// this is our plane equation
		p = { normal,p1 };
	}
	double getDist(glm::vec3 orgin, glm::vec3 dir) {


		double t = glm::dot((p1 - orgin), normal) / glm::dot(dir, normal);
		//if (dir.x+orgin.x == 0 && dir.y+orgin.y == 0) {
		//	std::cout << "data\n";
		//	std::cout << orgin << "\n";
		//	std::cout << dir << "\n";
		//	std::cout << normal << "\n";
		//	std::cout << glm::dot(dir, normal) << "\n";
		//	std::cout << p1 - orgin << "\n";
		//	std::cout << dot(p1 - orgin, normal) << "\n";
		//	std::cout << t << "\n";
		//}

		// get location of where the ray hits the plane
		glm::vec3 I = { orgin.x + t * dir.x,orgin.y + t * dir.y,orgin.z + t * dir.z };
		//if (dir.x + orgin.x == 0 && dir.y + orgin.y == 0) {
		//	std::cout << I << "\n";
		//	std::cout << t << "\n";
		//	std::cout << orgin << "\n";
		//	std::cout << dir << "\n";
		//}
		// triangle intersection formula
		glm::vec3 edge0 = p2 - p1;
		glm::vec3 edge1 = p3 - p2;
		glm::vec3 edge2 = p1 - p3;
		glm::vec3 C0 = I - p1;
		glm::vec3 C1 = I - p2;
		glm::vec3 C2 = I - p3;

		bool inside =
			glm::dot(normal, glm::cross(edge0, C0)) > 0.0 &&
			glm::dot(normal, glm::cross(edge1, C1)) > 0.0 &&
			glm::dot(normal, glm::cross(edge2, C2)) > 0.0;

		//bool inside = IinsidePlane(&I);

		if (inside) {
			return t;
		}
		else {
			return -1;
		}

	}


	typedef struct {
		double a, b, c, d;
	} abcd;
	glm::vec3 subtract(glm::vec3* one, glm::vec3* two) {
		return { one->x - two->x,one->y - two->y,one->z - two->z };
	}

	glm::vec3 getI(double t, glm::vec3* camera, glm::vec3* pixel) {
		return { (1 - t) * camera->x + t * pixel->x, (1 - t) * camera->y + t * pixel->y, (1 - t) * camera->z + t * pixel->z };
	}

	abcd calc() {
		glm::vec3 diff1 = subtract(&this->p3, &this->p1);
		glm::vec3 diff3 = subtract(&this->p2, &this->p1);


		glm::vec3 normal = glm::cross(diff1, diff3);
		abcd out = { -normal.x,-normal.y,-normal.z,
		normal.x * this->p1.x + normal.y * this->p1.y + normal.z * this->p1.z
		};
		return out;
	}
	
	double getT(abcd* coef, glm::vec3* camera, glm::vec3* pixel) {
		//double top = -(coef->a * camera->x + coef->b * camera->y + coef->c * camera->z + coef->d);
		//double bottom = (coef->a * (-camera->x + pixel->x) + coef->b * (-camera->y + pixel->y) + coef->c * (-camera->z + pixel->z));
		////std::cout << top << " " << bottom << " " << pixel.x << " " << pixel.y << " " << pixel.z << "\n";
		//if (bottom == 0) {
		//	return 10000000;
		//}
		//else {
		//	return top / bottom;
		//}
		glm::vec3 p2 = { pixel->x - camera->x,pixel->y - camera->y,pixel->z - camera->z };
		double t = -coef->d / (coef->a * p2.x + coef->b * p2.y + coef->c);
		return t;
	}
	double areaTri(glm::vec3* a, glm::vec3* b, glm::vec3* c) {
		glm::vec3 diff1 = { a->x - b->x,a->y - b->y,a->z - b->z };
		glm::vec3 diff2 = { a->x - c->x,a->y - c->y,a->z - c->z };
		//Point normal = cross(&diff1, &diff2);
		glm::vec3 normal = { diff1.y * diff2.z - diff1.z * diff2.y,
			diff1.z * diff2.x - diff1.x * diff2.z,
			diff1.x * diff2.y - diff1.y * diff2.x };
		double mag = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		return mag / 2.0;
	}

	bool IinsidePlane(glm::vec3* I) {
	double area1 = areaTri(I, &this->p1, &this->p2);
	double area2 = areaTri(I, &this->p2, &this->p3);
	double area3 = areaTri(I, &this->p1, &this->p3);
	double area = areaTri(&this->p1, &this->p2, &this->p3);
	if (fabs(area1 + area2 + area3 - area) < 0.0001) {
		return true;
	}
	return false;
}
private:

};

class Sphere : public Object {
public:
	Sphere() {

	};
	double getDist(glm::vec3 point, glm::vec3 dir) {

	}
private:
};


typedef struct {
	Triangle x;
	Triangle y;
} Face;












/*
* Calculates the area of a triangle
*/


/*
* calc takes in a Triangle and returns the plane equation from the Triangle's points
*/







class Camera {
public:
	glm::vec3 position;
	// in degrees
	glm::vec3 rotations;

	glm::vec3 direction;

	// up is y
	glm::vec3 worldUp = { 0,1,0 };
	float base_speed = .2;
	float speed = base_speed;
	float rotation_speed = 2;

	Camera() {
		position = { 0.0,0.0,0.0 };
		rotations = { 0.0,0.0,0.0 };
		direction = { 0,0,1 };
	}
	Camera(glm::vec3 pos, glm::vec3 rot) {
		position = pos;
		rotations = rot;
		direction = { 0,0,1 };
	}
	void translate(bool l, bool r, bool u, bool d, bool f, bool b) {
		
		// right of the camera is perpendicular vector of the direction of the camera
		// and the world up
		glm::vec3 right = glm::normalize(glm::cross(worldUp,direction));
		// almost finished with implementing camera motion
		glm::vec3 up = glm::normalize(glm::cross(direction, right));

		glm::vec3 forward = glm::normalize(glm::cross(right, up));
		forward.y *= -1;

		if (l) {
			position -= right * speed;
		}
		if (r) {
			position += right * speed;
		}
		if (u) {
			position -= up * speed;
		}
		if (d) {
			position += up * speed;
		}
		if (f) {
			position += forward * speed;
		}
		if (b) {
			position -= forward * speed;
		}
		

	}

	void rotate(bool l, bool r, bool u, bool d) {
		if (l) {
			rotations -= glm::vec3(rotation_speed, 0, 0);
		}
		if (r) {
			rotations += glm::vec3(rotation_speed, 0, 0);
		}
		if (u) {
			rotations += glm::vec3(0, rotation_speed, 0);
		}
		if (d) {
			rotations -= glm::vec3(0, rotation_speed, 0);
		}
		/*direction = {
			glm::cos(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y)),
			glm::sin(glm::radians(rotations.y)),
			glm::sin(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y))
		};*/

		direction = {
			glm::sin(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y)),
			glm::sin(glm::radians(rotations.y)),
			glm::cos(glm::radians(rotations.x)) * glm::cos(glm::radians(rotations.y))
		};
		direction = glm::normalize(direction);

		
	}
private:

};








// fix later
//Face createFace(Point start, double width, double height, double depth) {
//	Point tl = {start.x-width,start.y+height,start.z+depth};
//	Point tr = {start.x+width,start.y+height,start.z + depth };
//	Point bl = { start.x-width,start.y - height,start.z + depth };
//	Point br = {start.x+width,start.y - height,start.z + depth };
//	Triangle one = {tl,bl,br};
//	Triangle two = {tl,tr,br};
//	return {one,two};
//
//}
// there should only be 4 points because a sqaure has 4 points
// order is top-left, top-right, bottom-left, bottom-right
Face createFace(glm::vec3 points[]) {
	Triangle one = {points[0],points[2],points[3]};
	Triangle two = {points[0], points[1], points[3]};
	return { one,two };
}



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

//glm::vec3 camera = { 0,0,-10 };
Camera camera(glm::vec3(0,3,-10), glm::vec3(0,0,0));

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

	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
	//	std::cout << "thing\n";
	//	camera.x -= dx;
	//}
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
	//	camera.x += dx;
	//}

	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
	//	camera.z += dx;
	//}
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
	//	camera.z -= dx;
	//}

	//if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
	//	camera.y -= dx;
	//}
	//if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
	//	camera.y += dx;
	//}

	//if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
	//	t0 -= 1;
	//}
	//if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
	//	t0 += 1;
	//}


}





int main() {



	/*
	*  Need to fix camera orientation, y axis is upside down
	* only need to see the image once, so fix the way we write to the buffer
	* make it so the camera can move around without the image being distorted
	* allow for other shapes
	*/
	std::vector<Face> faces;
	std::vector<Triangle> triangles;
	glm::vec3 arr[] = { {-1,1,1},{1,1,1},{-1,-1,1},{1,-1,1} };
	Face a = createFace(arr);
	glm::vec3 arr2[] = {
		{-1,-1,1},
		{1,1,1},
		{-1,-1,1},
		{1,-1,1}
	};
	Face b = createFace(arr2);
	//std::cout << a.x.p1.x << "\n";
	//Face a = createFace({ 1,1,0 }, 1, 1, 1);
	//Face b = createFace({ 1,1,0 }, 1, 1, 1);
	//Face b = createFace({ -1,-2,1 }, 2, .5, 1);
	//Face c = createFace({-1,-3,1}, 3, 3, 4);
	//faces.push_back(a);
	//faces.push_back(b);
	//faces.push_back(c);

	for (Face const& f : faces) {
		triangles.push_back(f.x);
		triangles.push_back(f.y);
	}
	triangles.push_back({ {-1,-1,1},{-1,1,1},{1,-1,1} });

	//triangles.push_back({ {-1,1,2},{1,1,2},{1,-1,2} });

	// camera
	

	// picture size
	const int length = 800;
	const int width = 800;
	const double scale = length / 10;
	bool inside = false;


	// camera information
	double camera_viewport_width = 1.0;
	double camera_viewport_height = 1.0;
	double camera_viewport_depth = 1.0;

	//short out_arr[length * width][3];

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(length, width, "Window", NULL, NULL);
	if (window == NULL) {
		std::cout << "failed to create\n";
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initalize GLAD\n");
		return -1;
	}

	glViewport(0, 0, length, width);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, keycallback);

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
		//"   FragColor = vec4(TexCord,1,1);\n"
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width_t = length;
	int height_t = width;
	int channels = 3;

	unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * width_t * height_t * channels);
	int c = 0;
	for (int i = 0; i < width_t; i++) {
		for (int j = 0; j < height_t; j++) {

			data[c++] = 0;
			data[c++] = 0;
			data[c++] = 0;
		}
	}


	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_t, height_t, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


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

	float theta = 0.0;




	//end testing

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		//std::cout << camera << "\n";

		//double theta = (90 - t0) * 3.1415 / 180.0;

		//glm::vec3 pixel = { j/scale+camera.x,i/scale+camera.y,camera.z+1.0 };

		// this is the zero point. Found when the cos(t) and sin(t) are a point on the slope
		double x0 = cos(theta);

		//std::cout << camera << "\n";

		std::cout << camera.position << " " << camera.rotations << " " << camera.direction << "\n";

		

		//std::cout << t0 << " " << theta << "\n";

		int index = 0;
		for (int i = length / 2; i > -length / 2; i--) {
			for (int j = -width / 2; j < width / 2; j++) {

				Triangle out;
				double mint = 1000000000;
				bool found = false;
				// need to figure out this part
				//Point pixel_start = { j*sin(10) + camera.x,-i*cos(10) + camera.y,camera.z + 14};
				//sin(theta * 3.1415 / 180.0)
				//Point pixel = { j ,-i, 10};

				/*
				* 
				* formula: z = -(cos t)/(sin t) (x-cos t) + sin t
				* 
				* where t is the theta, x is the x value of the pixel and z is the correct z value of the pixel
				* 
				*/
				
				
				//std::cout << theta << " " << t0 << " ";
				//std::cout << "  slope: " << -sin(theta) / cos(theta) << " ";
				//std::cout << "  slope2: " << -sin(90-t0) / cos(90-t0) << " " << sin(theta) << " " << cos(theta) << "\n";
				//double x = x0 - j;
				//double z = -cos(theta) / sin(theta) * (x - cos(theta)) + sin(theta);
				float u = j / (float)height_t/camera_viewport_width;
				float v = i / (float)width_t/camera_viewport_height;

				glm::mat4 trans = glm::mat4(1.0f);
				trans = glm::rotate(trans, glm::radians((float)camera.rotations.x), glm::vec3(0.0f, 1.0f, 0.0f));
				trans = glm::rotate(trans, glm::radians((float)camera.rotations.y), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::vec4 pixel2 = trans * glm::vec4(u, v, camera_viewport_depth, 1.0);


				glm::vec3 pixel = {pixel2.x,pixel2.y,pixel2.z};

				//if (j == 0 && i == 0) {
				//	std::cout << pixel << " ";
				//	std::cout << t0 << " ";
				//	std::cout << camera << "\n";
				//}

				// OKAY SOMETHING WORKS. 
				// the issue is that when rotating 90 degrees, the x and z get flipped. But the x values is in 400ish range while z is 1.
				// so the triangle does not move much on screen. So we need to scale things correctly before rotating the camera

				/*
				* 0,0,1.1 triangle
				* 
				* 0,0,1 pixel
				* 
				* 0,0,0 camera
				*/

				//glm::vec3 pixel = { i,j,camera.z+.99};
				//std::cout << pixel2 << " " << pixel << "\n";
				
				//glm::vec3 pixel = glm::rotate(glm::radians(30), glm::vec3(j, i, z));

				//glm::vec3 pixel = {(j-camera.x)*cos(theta)-(i-camera.z)*sin(theta)+camera.x,0,(j-camera.x)*sin(theta)+(i-camera.x)*cos(theta)+camera.z};
				//theta += .001;
				if (theta > 360) {
					theta = 0.0;
				}

				for (int k = 0; k < triangles.size(); k++) {
					Triangle tr = triangles[k];
					//std::cout << tr.p1.x << " " << tr.p1.y << " " << tr.p1.z << "\n";
					double t = tr.getDist(camera.position, pixel);
					if (i == 0 && j == 0) {
						//std::cout << t << "\n";
					}
					if (t > camera_viewport_depth && t < mint) {
						out = tr;
						mint = t;
						found = true;
					}

				}

				// update the colors here
				if (found) {
					data[index++] = 255;
					data[index++] = 100;
					data[index++] = 100;
					//std::cout << 1 << "";

				}
				else {
					//fprintf(file, "0 0 50\n");
					data[index++] = 0;
					data[index++] = 0;
					data[index++] = 0;
					//std::cout << 0 << "";

				}
				

			}
			//std::cout << "\n";
		}

		//std::cout << "\n\n\n\n";
		


		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_t, height_t, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//c = 0;
		//for (int i = 0; i < width_t; i++) {
		//	for (int j = 0; j < height_t; j++) {
		//		if ((int)data[c++] == 255) {
		//			std::cout << 1 << "";
		//		}
		//		else {
		//			std::cout << 0 << "";
		//		}
		//		//std::cout << (int)data[c++] << " ";
		//		c++;
		//		c++;
		//	}
		//	std::cout << "\n";
		//}
		


		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();

		camera.rotate(lleft, lright, lup, ldown);
		camera.translate(left, right, up, down, forward, backward);
		//std::exit(1);

		//glfwSetWindowShouldClose(window, true);
		
	}

	glfwTerminate();
	std::cout << "here1\n";

	//FILE* file = fopen("src/Image.ppm", "w");
	//if (file == NULL) {
	//	printf("Failed to open");
	//}
	//fprintf(file, "P3\n%d %d\n255\n", width, length);


	//
	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


	////Triangle out;
	////double mint = 1000000000;
	////bool found = false;
	////Point pixel = { 0,0,camera.z + 1 };
	////for (int k = 0; k < triangles.size(); k++) {
	////	Triangle tr = triangles[k];
	////	abcd eq = calc(&tr);
	////	double t = getT(&eq, &camera, &pixel);
	////	//std::cout << t << "\n";
	////	Point I = getI(t, &camera, &pixel);
	////	std::cout << I << " " << t << "\n";
	////	bool inside = IinsidePlane(&I, &tr);
	////	if (inside && t < mint && t>0) {
	////		out = tr;
	////		mint = t;
	////		found = true;
	////	}
	////}

	////std::exit(0);
	//int index = 0;
	//for (int i = length/2; i > -length/2; i--) {
	//	for (int j = -width/2; j < width/2; j++) {

	//		Triangle out;
	//		double mint = 1000000000;
	//		bool found = false;
	//		Point pixel = { j / scale,i / scale ,camera.z + 1 };
	//		for (int k = 0; k < triangles.size(); k++) {
	//			Triangle tr = triangles[k];
	//			abcd eq = calc(&tr);
	//			double t = getT(&eq, &camera, &pixel);
	//			//std::cout << t << "\n";
	//			Point I = getI(t, &camera, &pixel);
	//			//std::cout << I << " " << t << "\n";
	//			bool inside = IinsidePlane(&I, &tr);
	//			if (inside && t < mint && t>0) {
	//				out = tr;
	//				mint = t;
	//				found = true;
	//			}
	//		}

	//		if (found) {
	//			out_arr[index][0] = 100;
	//			out_arr[index][1] = 100;
	//			out_arr[index][2] = 100;
	//			//fprintf(file, "%d %d %d\n", 100, 0, 0);
	//		}
	//		else {
	//			//fprintf(file, "0 0 50\n");
	//			out_arr[index][0] = 255;
	//			out_arr[index][1] = 255;
	//			out_arr[index][2] = 255;
	//		}
	//		index++;
	//		
	//	}
	//}
	//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	//std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;
	//for (int i = 0; i < index; i++) {
	//	fprintf(file, "%d %d %d\n", out_arr[i][0], out_arr[i][1], out_arr[i][2]);
	//}
	//fclose(file);


}