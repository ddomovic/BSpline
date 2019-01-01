#include "pch.h"
#include <iostream>
#include <GL/glut.h>
#include <string>
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <math.h>
#include <vector>
#ifdef _unix
#include <unistd.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif

# define PI 3.141592653589793238462643383279502884L

void sleep(int millis);
void parse_points_and_polys();
void draw_polygon(struct poly, double, std::vector<double>);
void draw_object();
void display_f();
void reshape_f(int w, int h);
void mouse_f(int button, int state, int x, int y);
void keyboard_f(unsigned char theKey, int mouseX, int mouseY);
void parse_b_spline_points();
void draw_control_points();
void draw_b_spline();
void calculate_functions();
double calculate_rotation(struct point);
void calculate_orientations();
std::vector<double> calculate_axis(struct point);
void draw_tangents();

std::ifstream infile("frog.obj");

struct point {

	GLdouble c_x;
	GLdouble c_y;
	GLdouble c_z;
};

struct poly {

	struct point point1;
	struct point point2;
	struct point point3;
};

GLuint window;
GLuint width = 800, height = 600;
GLuint object;
std::vector<struct poly> poly_data;
std::vector<struct point> open_poly_points;
std::vector<struct point> pi_points;
std::vector<struct point> pi_orientations;
struct point object_center;
glm::vec3 s(0.0, 0.0, 1.0);

double rot_f = 0;
int cp = 0;

int key_x = -5;
int key_y = -5;
int key_z = -75;

void sleep(int millis) {
#ifdef _unix
	usleep(millis * 1000);
#endif
#ifdef _WIN32
	Sleep(millis);
#endif
}

void parse_points_and_polys() {

	std::string line;
	std::vector<struct point> aux;
	object_center.c_x = 0;
	object_center.c_y = 0;
	object_center.c_z = 0;

	while (std::getline(infile, line)) {

		if (!line.empty()) {

			if (line[0] == 'v') {

				std::istringstream iss(line);
				struct point aux_point;
				double a, b, c;
				char x;

				if (!(iss >> x >> a >> b >> c)) {

					std::cout << "An errror has occured while parsing v's!" << std::endl;
					break;
				}

				aux_point.c_x = a;
				aux_point.c_y = b;
				aux_point.c_z = c;
				object_center.c_x += a;
				object_center.c_y += b;
				object_center.c_z += c;
				aux.push_back(aux_point);
			}
			else if (line[0] == 'f') {

				std::istringstream iss(line);
				struct poly aux_poly;
				int a, b, c;
				char x;

				if (!(iss >> x >> a >> b >> c)) {

					std::cout << "An errror has occured while parsing f's!" << std::endl;
					break;
				}

				aux_poly.point1 = aux.at(a - 1);
				aux_poly.point2 = aux.at(b - 1);
				aux_poly.point3 = aux.at(c - 1);
				poly_data.push_back(aux_poly);
			}
		}
	}
	object_center.c_x /= aux.size();
	object_center.c_y /= aux.size();
	object_center.c_z /= aux.size();
	std::cout << object_center.c_x << " " << object_center.c_y << " " << object_center.c_z << std::endl;
}

void draw_polygon(struct poly polygon, double angle, std::vector<double> axis) {

	struct point p1 = polygon.point1;
	struct point p2 = polygon.point2;
	struct point p3 = polygon.point3;

	glVertex3f(p1.c_x, p1.c_y, p1.c_z);
	glVertex3f(p2.c_x, p2.c_y, p2.c_z);
	glVertex3f(p3.c_x, p3.c_y, p3.c_z);
	glVertex3f(p1.c_x, p1.c_y, p1.c_z);
}

void draw_object() {

	double angle = calculate_rotation(pi_orientations[cp]);
	angle = (angle * 180) / PI;
	std::vector<double> axis = calculate_axis(pi_orientations[cp]);
	glColor3f(0.4f, 0.6f, 0.f);

	glPushMatrix();
	glTranslatef(pi_points[cp].c_x, pi_points[cp].c_y, pi_points[cp].c_z);
	glRotatef(angle, axis[0], axis[1], axis[2]);

	for (std::vector<struct poly>::iterator it = poly_data.begin(); it != poly_data.end(); ++it) {

		glBegin(GL_LINE_STRIP);
		draw_polygon(*it, angle, axis);
		glEnd();
	}
	glPopMatrix();
	glColor3f(1.f, 0.f, 0.f);
}

void display_f() {

	glClearColor(0, 0, 0, 1);

	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTranslatef(key_x, key_y, key_z);
	glColor3f(1.0f, 0, 0);
	
	draw_object();
	draw_b_spline();
	glColor3f(0, 0, 1.0f);
	draw_tangents();
	glColor3f(1.0f, 0, 0);
	glFlush();
	rot_f += 0.02;
	if (rot_f > 360) {
		rot_f -= 360;
	}
	if (cp >= pi_points.size()) {
		cp = 0;
	}
	else {
		cp += 1;
	}
	sleep(20);
}

void reshape_f(int w, int h) {

	width = w;
	height = h;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, ((double)width / height), 0.1, 500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1.0);
	glColor3f(0.0f, 0.0f, 0.0f);

}

void mouse_f(int button, int state, int x, int y) {

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {

	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		reshape_f(width, height);
		glFlush();
	}
}

void keyboard_f(unsigned char theKey, int mouseX, int mouseY) {

	switch (theKey) {

	case 'w':
		key_y++;
		break;
	case 's':
		key_y--;
		break;
	case 'a':
		key_x--;
		break;
	case 'd':
		key_x++;
		break;
	case 'q':
		key_z++;
		break;
	case 'e':
		key_z--;
		break;
	}
}

void parse_b_spline_points() {

	std::string line;
	struct point aux_point;
	std::ifstream b_file("B_spline.txt");

	while (std::getline(b_file, line)) {

		if (!line.empty()) {

			std::istringstream iss(line);
			double a, b, c;

			if (!(iss >> a >> b >> c)) {
				std::cout << "An error has occured!" << std::endl;
			}

			aux_point.c_x = a;
			aux_point.c_y = b;
			aux_point.c_z = c;
			open_poly_points.push_back(aux_point);
		}
	}
}

void draw_b_spline() {

	glBegin(GL_LINE_STRIP);
	for (std::vector<struct point>::iterator it = pi_points.begin(); it != pi_points.end(); ++it) {

		glVertex3f(it->c_x, it->c_y, it->c_z);
	}
	glEnd();
}

void draw_control_points() {

	glBegin(GL_LINE_STRIP);

	for (std::vector<struct point>::iterator it = open_poly_points.begin(); it != open_poly_points.end(); ++it) {

		glVertex3f(it->c_x, it->c_y, it->c_z);
	}
	glEnd();
}

void calculate_functions() {

	double Mb_array[16] = { -1.0 / 6, 0.5, -0.5, 1.0 / 6, 0.5, -1, 0.5, 0, -0.5, 0, 0.5, 0, 1.0 / 6, 2.0 / 3, 1.0 / 6, 0 };
	glm::mat4 Mb = glm::make_mat4(Mb_array);

	for (int i = 1; i < open_poly_points.size() - 2; i++) {

		double rs[12] = { open_poly_points[i - 1].c_x, open_poly_points[i - 1].c_y, open_poly_points[i - 1].c_z,
						open_poly_points[i].c_x, open_poly_points[i].c_y, open_poly_points[i].c_z,
						open_poly_points[i + 1].c_x, open_poly_points[i + 1].c_y, open_poly_points[i + 1].c_z,
						open_poly_points[i + 2].c_x, open_poly_points[i + 2].c_y, open_poly_points[i + 2].c_z };
		for (float t = 0; t <= 1; t += 0.01) {

			glm::vec4 ts(pow(t, 3), pow(t, 2), t, 1);

			glm::vec4 pi_x = Mb * ts;

			double a = pi_x.x * rs[0] + pi_x.y * rs[3] + pi_x.z * rs[6] + pi_x.w * rs[9];
			double b = pi_x.x * rs[1] + pi_x.y * rs[4] + pi_x.z * rs[7] + pi_x.w * rs[10];
			double c = pi_x.x * rs[2] + pi_x.y * rs[5] + pi_x.z * rs[8] + pi_x.w * rs[11];

			struct point p;
			p.c_x = a;
			p.c_y = b;
			p.c_z = c;
			pi_points.push_back(p);
		}
	}
}

void calculate_orientations() {

	double Bi3_array[12] = { -0.5, 1.5, -1.5, 0.5, 1, -2, 1, 0, -0.5, 0, 0.5, 0 };

	for (int i = 1; i < open_poly_points.size() - 2; i++) {

		double rs[12] = { open_poly_points[i - 1].c_x, open_poly_points[i - 1].c_y, open_poly_points[i - 1].c_z,
						open_poly_points[i].c_x, open_poly_points[i].c_y, open_poly_points[i].c_z,
						open_poly_points[i + 1].c_x, open_poly_points[i + 1].c_y, open_poly_points[i + 1].c_z,
						open_poly_points[i + 2].c_x, open_poly_points[i + 2].c_y, open_poly_points[i + 2].c_z };

		for (float t = 0; t <= 1; t += 0.01) {

			glm::vec3 ts(pow(t, 2), t, 1);

			glm::vec4 pi_x;

			pi_x.x = ts.x * Bi3_array[0] + ts.y * Bi3_array[4] + ts.z * Bi3_array[8];
			pi_x.y = ts.x * Bi3_array[1] + ts.y * Bi3_array[5] + ts.z * Bi3_array[9];
			pi_x.z = ts.x * Bi3_array[2] + ts.y * Bi3_array[6] + ts.z * Bi3_array[10];
			pi_x.w = ts.x * Bi3_array[3] + ts.y * Bi3_array[7] + ts.z * Bi3_array[11];

			double a = pi_x.x * rs[0] + pi_x.y * rs[3] + pi_x.z * rs[6] + pi_x.w * rs[9];
			double b = pi_x.x * rs[1] + pi_x.y * rs[4] + pi_x.z * rs[7] + pi_x.w * rs[10];
			double c = pi_x.x * rs[2] + pi_x.y * rs[5] + pi_x.z * rs[8] + pi_x.w * rs[11];

			struct point p;
			p.c_x = a;
			p.c_y = b;
			p.c_z = c;
			pi_orientations.push_back(p);
		}
	}
}

std::vector<double> calculate_axis(struct point current_orientation) {

	std::vector<double> ret;

	ret.push_back(s.y * (current_orientation.c_z) - (current_orientation.c_y) * s.z);
	ret.push_back((-1) * s.x * (current_orientation.c_z) + (current_orientation.c_x) * s.z);
	ret.push_back(s.x * (current_orientation.c_y) - s.y * (current_orientation.c_x));

	return ret;
}

double calculate_rotation(struct point current_orientation) {

	glm::vec3 e(current_orientation.c_x, current_orientation.c_y, current_orientation.c_z);

	double s_e = sqrt(pow(s.x, 2) + pow(s.y, 2) + pow(s.z, 2)) *
		sqrt(pow(e.x, 2) + pow(e.y, 2) + pow(e.z, 2));
	double cos_b = glm::dot(s, e) / s_e;

	return acos(cos_b);
}

void draw_tangents() {

	double lambda = 0.5;

	glBegin(GL_LINES);
	for (int i = 0; i < pi_orientations.size(); i++) {

		if ((i % 25) == 0) {

			glVertex3f(pi_points[i].c_x, pi_points[i].c_y, pi_points[i].c_z);
			glVertex3f(pi_points[i].c_x + lambda * pi_orientations[i].c_x, pi_points[i].c_y + lambda * pi_orientations[i].c_y,
				pi_points[i].c_z + lambda * pi_orientations[i].c_z);
		}
	}
	glEnd();
}

int main(int argc, char *argv[]) {


	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);

	parse_points_and_polys();
	std::cout << "JEDAN!\n";
	parse_b_spline_points();
	std::cout << "DVA!\n";
	calculate_functions();
	std::cout << "TRI!\n";
	calculate_orientations();
	std::cout << "CETIRI!\n";

	window = glutCreateWindow("B-spline");
	glutReshapeFunc(reshape_f);
	glutDisplayFunc(display_f);
	glutIdleFunc(display_f);
	glutMouseFunc(mouse_f);
	glutKeyboardFunc(keyboard_f);
	glutMainLoop();

	return 0;
}
