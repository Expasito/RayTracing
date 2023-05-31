#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>

typedef struct {
	double x;
	double y;
	double z;
} Point;

typedef struct {
	Point p1;
	Point p2;
	Point p3;
} Triangle;

typedef struct {
	Triangle x;
	Triangle y;
} Face;

typedef struct {
	double a, b, c, d;
} abcd;

typedef struct {
	double t;
	abcd eq;
} Node;

typedef struct {
	Point camera;

} Ray;


Point subtract(Point* one, Point* two) {
	Point out = {one->x-two->x,one->y-two->y,one->z-two->z};
	return out;
}

Point cross(Point* one, Point* two) {
	Point out = {
		one->y*two->z-one->z*two->y,
		one->z*two->x-one->x*two->z,
		one->x*two->y-one->y*two->x
	};
	return out;
}

/*
* areaTri calculates the area of a Triangle
*/
double areaTri(Point* a, Point* b, Point* c) {
	//Point diff1 = subtract(a, b);
	//Point diff2 = subtract(a, c);
	Point diff1 = { a->x - b->x,a->y - b->y,a->z - b->z };
	Point diff2 = { a->x - c->x,a->y - c->y,a->z - c->z };
	//Point normal = cross(&diff1, &diff2);
	Point normal = { diff1.y * diff2.z - diff1.z * diff2.y,
		diff1.z * diff2.x - diff1.x * diff2.z,
		diff1.x * diff2.y - diff1.y * diff2.x };
	double mag = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	return mag / 2.0;

}

/*
* calc takes in a Triangle and returns the plane equation from the Triangle's points
*/
abcd calc(Triangle* p) {
	Point diff1 = subtract(&p->p3, &p->p1);
	Point diff3 = subtract(&p->p2, &p->p1);


	Point normal = cross(&diff1, &diff3);
	abcd out = { -normal.x,-normal.y,-normal.z,
	normal.x * p->p1.x + normal.y * p->p1.y + normal.z * p->p1.z
	};
	return out;
}

double getT(abcd* coef, Point* camera, Point* pixel) {
	//solve for Ix, Iy, Iz and find t
	//std::cout << coef.a << " " << coef.b << " " << coef.c << " " << coef.d << "\n";
	//std::cout << camera.x-pixel.x << " " << camera.y-pixel.y << " " << camera.z-pixel.z << " \n";
	double top = -(coef->a * camera->x + coef->b * camera->y + coef->c * camera->z + coef->d);
	double bottom = (coef->a * (-camera->x + pixel->x) + coef->b * (-camera->y + pixel->y) + coef->c * (-camera->z + pixel->z));
	//std::cout << top << " " << bottom << " " << pixel.x << " " << pixel.y << " " << pixel.z << "\n";
	if (bottom == 0) {
		return 10000000;
	}
	else {
		return top / bottom;
	}
}

Point getI(double t, Point* camera, Point* pixel) {
	Point out = {(1-t)*camera->x+t*pixel->x, (1-t)*camera->y + t*pixel->y, (1-t)*camera->z + t*pixel->z};
	return out;
}


bool IinsidePlane(Point* I, Triangle* plane) {
	double area1 = areaTri(I, &plane->p1, &plane->p2);
	double area2 = areaTri(I, &plane->p2, &plane->p3);
	double area3 = areaTri(I, &plane->p1, &plane->p3);
	double area = areaTri(&plane->p1, &plane->p2, &plane->p3);
	if (fabs(area1 + area2 + area3 - area) < 0.0001) {
		return true;
	}
	return false;

}

double sqrt2(double val) {
	return 0;
}


Face createFace(Point start, double width, double height, double depth) {
	Point tl = {start.x-width,start.y+height,start.z+depth};
	Point tr = {start.x+width,start.y+height,start.z + depth };
	Point bl = { start.x-width,start.y - height,start.z + depth };
	Point br = {start.x+width,start.y - height,start.z + depth };
	Triangle one = {tl,bl,br};
	Triangle two = {tl,tr,br};
	return {one,two};

}
// there should only be 4 points because a sqaure has 4 points
// order is top-left, top-right, bottom-left, bottom-right
Face createFace(Point points[]) {
	Triangle one = {points[0],points[2],points[3]};
	Triangle two = {points[0], points[1], points[3]};
	return { one,two };
}

std::ostream& operator<<(std::ostream& os, const Point& vec)
{
	os << vec.x << ' ' << vec.y << ' ' << vec.z;
	return os;
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
void keycallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);



}
int main() {

	std::vector<Face> faces;
	std::vector<Triangle> triangles;
	Point arr[] = { {-1,1,1},{1,1,1},{-1,-1,1},{1,-1,1} };
	Face a = createFace(arr);
	Point arr2[] = {
		{1,1,2},
		{1,1,0},
		{1,-1,1},
		{1,-1,0}
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
	triangles.push_back({ {-2,-1,0},{-1,1,0},{1,-1,5} });

	// camera
	Point camera = { 0,0,-.25 };

	// picture size
	const int length = 300;
	const int width = 300;
	const double scale = length / 10;
	bool inside = false;
	short out_arr[length * width][3];

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 800, "Window", NULL, NULL);
	if (window == NULL) {
		std::cout << "failed to create\n";
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initalize GLAD\n");
		return -1;
	}

	glViewport(0, 0, 800, 800);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, keycallback);

	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"layout (location = 1) in vec2 textCoord;\n"

		"out vec2 TexCord;"
		"void main()\n"
		"{\n"
		"TexCord=textCoord;\n"
		"   gl_Position = vec4(pos,1);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCord;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
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
	-1,-1,0,-1,-1,
	-1,1,0,-1,1,
	1,-1,0,1,-1,
	};

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// do position first
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
	// texture coordinates
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAO);

	float buff[500];
	glBufferSubData(GL_ARRAY_BUFFER, 0, 500, &buff);
	std::cout << buff[0] << " " << buff[1] << "\n";

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);



		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();


	FILE* file = fopen("src/Image.ppm", "w");
	if (file == NULL) {
		printf("Failed to open");
	}
	fprintf(file, "P3\n%d %d\n255\n", width, length);


	
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


	//Triangle out;
	//double mint = 1000000000;
	//bool found = false;
	//Point pixel = { 0,0,camera.z + 1 };
	//for (int k = 0; k < triangles.size(); k++) {
	//	Triangle tr = triangles[k];
	//	abcd eq = calc(&tr);
	//	double t = getT(&eq, &camera, &pixel);
	//	//std::cout << t << "\n";
	//	Point I = getI(t, &camera, &pixel);
	//	std::cout << I << " " << t << "\n";
	//	bool inside = IinsidePlane(&I, &tr);
	//	if (inside && t < mint && t>0) {
	//		out = tr;
	//		mint = t;
	//		found = true;
	//	}
	//}

	//std::exit(0);
	int index = 0;
	for (int i = length/2; i > -length/2; i--) {
		for (int j = -width/2; j < width/2; j++) {

			Triangle out;
			double mint = 1000000000;
			bool found = false;
			Point pixel = { j / scale,i / scale ,camera.z + 1 };
			for (int k = 0; k < triangles.size(); k++) {
				Triangle tr = triangles[k];
				abcd eq = calc(&tr);
				double t = getT(&eq, &camera, &pixel);
				//std::cout << t << "\n";
				Point I = getI(t, &camera, &pixel);
				//std::cout << I << " " << t << "\n";
				bool inside = IinsidePlane(&I, &tr);
				if (inside && t < mint && t>0) {
					out = tr;
					mint = t;
					found = true;
				}
			}

			if (found) {
				out_arr[index][0] = 100;
				out_arr[index][1] = 100;
				out_arr[index][2] = 100;
				//fprintf(file, "%d %d %d\n", 100, 0, 0);
			}
			else {
				//fprintf(file, "0 0 50\n");
				out_arr[index][0] = 255;
				out_arr[index][1] = 255;
				out_arr[index][2] = 255;
			}
			index++;
			
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;
	for (int i = 0; i < index; i++) {
		fprintf(file, "%d %d %d\n", out_arr[i][0], out_arr[i][1], out_arr[i][2]);
	}
	fclose(file);


}