#include <stdlib.h>
#include <float.h>
#include <vector>
#include <algorithm>
#include "opengl.h"
#include "app.h"
#include "curve.h"
#include "widgets.h"

int win_width, win_height;
float win_aspect;

static void draw_grid(float sz, float sep, float alpha = 1.0f);
static void draw_curve(const Curve *curve);
static void on_click(int bn, float u, float v);

static float view_pan_x, view_pan_y;
static float view_scale = 1.0f;

static std::vector<Curve*> curves;
static Curve *sel_curve, *new_curve;
static int sel_pidx = -1;

static Label *weight_label;


bool app_init(int argc, char **argv)
{
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	return true;
}

void app_cleanup()
{
	for(size_t i=0; i<curves.size(); i++) {
		delete curves[i];
	}
	curves.clear();
}

void app_draw()
{
	glClearColor(0.1, 0.1, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-view_pan_x, -view_pan_y, 0);
	glScalef(view_scale, view_scale, view_scale);

	draw_grid(std::max(win_aspect, 1.0f / win_aspect), 0.1);

	for(size_t i=0; i<curves.size(); i++) {
		draw_curve(curves[i]);
	}
	if(new_curve) {
		// TODO special drawing with more feedback
		draw_curve(new_curve);
	}
	if(weight_label) {
		weight_label->draw();
	}
}

static void draw_grid(float sz, float sep, float alpha)
{
	float x = 0.0f;

	glLineWidth(1.0);
	glBegin(GL_LINES);
	glColor4f(0.6, 0.3, 0.2, alpha);
	glVertex2f(-sz, 0);
	glVertex2f(sz, 0);
	glColor4f(0.2, 0.3, 0.6, alpha);
	glVertex2f(0, -sz);
	glVertex2f(0, sz);
	glColor4f(0.35, 0.35, 0.35, alpha);
	while(x < sz) {
		x += sep;
		glVertex2f(-sz, x);
		glVertex2f(sz, x);
		glVertex2f(-sz, -x);
		glVertex2f(sz, -x);
		glVertex2f(x, -sz);
		glVertex2f(x, sz);
		glVertex2f(-x, -sz);
		glVertex2f(-x, sz);
	}
	glEnd();
}

static void draw_curve(const Curve *curve)
{
	int numpt = curve->get_point_count();
	int segm = numpt * 16.0f;

	glLineWidth(2.0);
	if(curve == sel_curve) {
		glColor3f(0.3, 0.4, 1.0);
	} else if(curve == new_curve) {
		glColor3f(1.0, 0.75, 0.3);
	} else {
		glColor3f(0.75, 0.75, 0.75);
	}
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<segm; i++) {
		float t = (float)i / (float)(segm - 1);
		Vector2 v = curve->interpolate(t);
		glVertex2f(v.x, v.y);
	}
	glEnd();
	glLineWidth(1.0);

	glPointSize(7.0);
	glBegin(GL_POINTS);
	if(curve == new_curve) {
		glColor3f(1.0, 0.0, 0.0);
	} else {
		glColor3f(0.6, 0.3, 0.2);
	}
	for(int i=0; i<numpt; i++) {
		if(curve == sel_curve) {
			if(i == sel_pidx) {
				glColor3f(1.0, 0.2, 0.1);
			} else {
				glColor3f(0.2, 1.0, 0.2);
			}
		}
		Vector2 pt = curve->get_point(i);
		glVertex2f(pt.x, pt.y);
	}
	glEnd();
	glPointSize(1.0);
}

void app_reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;

	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-win_aspect, win_aspect, -1, 1, -1, 1);
}

void app_keyboard(int key, bool pressed)
{
	if(pressed) {
		switch(key) {
		case 'q':
		case 'Q':
			exit(0);

		case 27:
			if(new_curve) {
				delete new_curve;
				new_curve = 0;
				post_redisplay();
			}
			break;

		case 'l':
		case 'L':
			if(sel_curve) {
				sel_curve->set_type(CURVE_LINEAR);
				post_redisplay();
			}
			if(new_curve) {
				new_curve->set_type(CURVE_LINEAR);
				post_redisplay();
			}
			break;

		case 'b':
		case 'B':
			if(sel_curve) {
				sel_curve->set_type(CURVE_BSPLINE);
				post_redisplay();
			}
			if(new_curve) {
				new_curve->set_type(CURVE_BSPLINE);
				post_redisplay();
			}
			break;

		case 'h':
		case 'H':
			if(sel_curve) {
				sel_curve->set_type(CURVE_HERMITE);
				post_redisplay();
			}
			if(new_curve) {
				new_curve->set_type(CURVE_HERMITE);
				post_redisplay();
			}
			break;
		}
	}
}

static Vector2 pixel_to_uv(int x, int y)
{
	float u = win_aspect * (2.0 * (float)x / (float)win_width - 1.0);
	float v = 1.0 - 2.0 * (float)y / (float)win_height;
	return Vector2(u, v);
}

static int prev_x, prev_y;
static int click_pos[8][2];
static unsigned int bnstate;

#define BNBIT(x)	(1 << (x))

void app_mouse_button(int bn, bool pressed, int x, int y)
{
	prev_x = x;
	prev_y = y;
	if(pressed) {
		bnstate |= BNBIT(bn);
	} else {
		bnstate &= ~BNBIT(bn);
	}

	if(pressed) {
		click_pos[bn][0] = x;
		click_pos[bn][1] = y;
	} else {
		int dx = x - click_pos[bn][0];
		int dy = y - click_pos[bn][1];

		if(abs(dx) + abs(dy) < 3) {
			Vector2 uv = pixel_to_uv(x, y);
			on_click(bn, uv.x, uv.y);
		}

		if(!(bnstate & BNBIT(2))) {
			delete weight_label;
			weight_label = 0;
			post_redisplay();
		}
	}
}

static void hover(int x, int y)
{
	float thres = 0.02;

	Vector2 uv = pixel_to_uv(x, y);
	for(size_t i=0; i<curves.size(); i++) {
		int pidx = curves[i]->nearest_point(uv);
		if(pidx == -1) continue;

		Vector2 cp = curves[i]->get_point(pidx);
		if((cp - uv).length_sq() < thres * thres) {
			sel_curve = curves[i];
			sel_pidx = pidx;
			return;
		}
	}

	sel_curve = 0;
	sel_pidx = -1;
}

void app_mouse_motion(int x, int y)
{
	int dx = x - prev_x;
	int dy = y - prev_y;
	prev_x = x;
	prev_y = y;

	if(!dx && !dy) return;

	if(!new_curve && !bnstate) {
		hover(x, y);
		post_redisplay();
	}

	if(sel_curve && sel_pidx != -1) {
		if(bnstate & BNBIT(0)) {
			float w = sel_curve->get_weight(sel_pidx);
			sel_curve->set_point(sel_pidx, pixel_to_uv(x, y), w);
			post_redisplay();
		}

		if(bnstate & BNBIT(2)) {
			float w = sel_curve->get_weight(sel_pidx);
			w -= dy * 0.01;
			if(w < FLT_MIN) w = FLT_MIN;
			sel_curve->set_weight(sel_pidx, w);

			if(!weight_label) {
				weight_label = new Label;
			}
			weight_label->set_position(pixel_to_uv(x, y));
			weight_label->set_textf("w=%g", w);
			post_redisplay();
		}
	}
}

static void on_click(int bn, float u, float v)
{
	switch(bn) {
	case 0:
		if(!new_curve) {
			new_curve = new Curve;
		}
		new_curve->add_point(Vector2(u, v));
		post_redisplay();
		break;

	case 2:
		curves.push_back(new_curve);
		new_curve = 0;
		post_redisplay();
		break;

	default:
		break;
	}
}
