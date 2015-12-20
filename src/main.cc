#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "app.h"

static void display();
static void keydown(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1280, 720);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutCreateWindow("Curve Draw");

	glutDisplayFunc(display);
	glutReshapeFunc(app_reshape);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutMouseFunc(mouse);
	glutMotionFunc(app_mouse_motion);
	glutPassiveMotionFunc(app_mouse_motion);

	if(!app_init(argc, argv)) {
		return 1;
	}
	atexit(app_cleanup);

	glutMainLoop();
	return 0;
}

void post_redisplay()
{
	glutPostRedisplay();
}

static void display()
{
	app_draw();
	glutSwapBuffers();
}

static void keydown(unsigned char key, int x, int y)
{
	app_keyboard(key, true);
}

static void keyup(unsigned char key, int x, int y)
{
	app_keyboard(key, false);
}

static void mouse(int bn, int st, int x, int y)
{
	app_mouse_button(bn - GLUT_LEFT_BUTTON, st == GLUT_DOWN, x, y);
}
