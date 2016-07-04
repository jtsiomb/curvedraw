/*
curvedraw - a simple program to draw curves
Copyright (C) 2015-2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
	if(bn == 3 || bn == 4) {
		if(st == GLUT_DOWN) {
			app_mouse_wheel(bn == 3 ? 3 : -3);
		}
	} else {
		app_mouse_button(bn - GLUT_LEFT_BUTTON, st == GLUT_DOWN, x, y);
	}
}
