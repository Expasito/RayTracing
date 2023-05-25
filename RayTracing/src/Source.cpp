#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <chrono>


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
int main() {

	std::vector<Face> faces;
	std::vector<Triangle> triangles;
	Face a = createFace({ 1,1,0 }, 1, 1, 1);
	Face b = createFace({ -1,-2,1 }, 2, .5, 1);
	Face c = createFace({-1,-3,1}, 3, 3, 4);
	faces.push_back(a);
	faces.push_back(b);
	faces.push_back(c);

	for (Face const & f : faces) {
		triangles.push_back(f.x);
		triangles.push_back(f.y);
	}


	// camera
	Point camera = { 0,0,0 };

	// picture size
	const int length = 1080;
	const int width = 1080;
	const double scale = length/10;
	bool inside = false;
	//short out_arr[length * width][3];




	//FILE* file = fopen("src/Image.ppm", "w");
	//if (file == NULL) {
	//	printf("Failed to open");
	//}
	//fprintf(file, "P3\n%d %d\n255\n", width, length);

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	int index = 0;
	for (int i = length/2; i > -length/2; i--) {
		for (int j = -width/2; j < width/2; j++) {

			Triangle out;
			double mint = 1000000000;
			bool found = false;
			Point pixel = {j/scale,i/scale,1};
			for (int k = 0; k < triangles.size(); k++) {
				Triangle tr = triangles[k];
				abcd eq = calc(&tr);
				double t = getT(&eq, &camera, &pixel);
				Point I = getI(t, &camera, &pixel);
				bool inside = IinsidePlane(&I, &tr);
				if (inside && t < mint && t>0) {
					out = tr;
					mint = t;
					found = true;
				}
			}

			if (found) {
				//out_arr[index][0] = 100;
				//out_arr[index][1] = 100;
				//out_arr[index][2] = 40;
				//fprintf(file, "%d %d %d\n", 100, 0, 0);
			}
			else {
				//fprintf(file, "0 0 50\n");
				//out_arr[index][0] = 255;
				//out_arr[index][1] = 255;
				//out_arr[index][2] = 255;
			}
			index++;
			
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;
	/*for (int i = 0; i < index; i++) {
		fprintf(file, "%d %d %d\n", out_arr[i][0], out_arr[i][1], out_arr[i][2]);
	}
	fclose(file);*/


}