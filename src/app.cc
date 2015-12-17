#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include <vector>
#include <algorithm>
#include "opengl.h"
#include "app.h"
#include "curve.h"
#include "widgets.h"

enum SnapMode {
	SNAP_NONE,
	SNAP_GRID,
	SNAP_POINT
};

int win_width, win_height;
float win_aspect;

static void draw_grid(float sz, float sep, float alpha = 1.0f);
static void draw_curve(const Curve *curve);
static void on_click(int bn, float u, float v);

// viewport control
static Vector2 view_pan;
static float view_scale = 0.2f;
static Matrix4x4 view_matrix;

static float grid_size = 1.0;
static SnapMode snap_mode;

static std::vector<Curve*> curves;
static Curve *sel_curve;	// selected curve being edited
static Curve *new_curve;	// new curve being entered
static Curve *hover_curve;	// curve the mouse is hovering over (click to select)
static int sel_pidx = -1;	// selected point of the selected or hovered-over curve

static Label *weight_label;	// floating label for the cp weight

#ifdef DRAW_MOUSE_POINTER
static Vector2 mouse_pointer;
#endif


bool app_init(int argc, char **argv)
{
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	glTranslatef(view_pan.x * view_scale, view_pan.y * view_scale, 0);
	glScalef(view_scale, view_scale, view_scale);

	float max_aspect = std::max(win_aspect, 1.0f / win_aspect);
	draw_grid(max_aspect, grid_size);

	for(size_t i=0; i<curves.size(); i++) {
		draw_curve(curves[i]);
	}
	if(new_curve) {
		draw_curve(new_curve);
	}

#ifdef DRAW_MOUSE_POINTER
	glPointSize(6.0);
	glBegin(GL_POINTS);
	glColor3f(0, 0, 1);
	glVertex2f(mouse_pointer.x, mouse_pointer.y);
	glEnd();
#endif

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(weight_label) {
		weight_label->draw();
	}
}

static void draw_grid(float sz, float sep, float alpha)
{
	float x = 0.0f;
	float s = 1.0 / view_scale;

	sz *= s;
	sz += sep;	// one more step for when we have non-zero fractional pan
	float end = std::min(sz, 100.0f * sep);

	// fractional pan
	Vector2 pan = view_pan;
	Vector2 fpan = Vector2(fmod(pan.x, sep), fmod(pan.y, sep));
	Vector2 offset = fpan - pan;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(offset.x, offset.y, 0);

	glBegin(GL_LINES);
	glColor4f(0.35, 0.35, 0.35, alpha);
	while(x <= end) {
		glVertex2f(-end, x);
		glVertex2f(end, x);
		glVertex2f(-end, -x);
		glVertex2f(end, -x);
		glVertex2f(x, -end);
		glVertex2f(x, end);
		glVertex2f(-x, -end);
		glVertex2f(-x, end);
		x += sep;
	}
	glEnd();
	glPopMatrix();


	glLineWidth(1.0);
	glBegin(GL_LINES);
	glColor4f(0.6, 0.3, 0.2, alpha);
	glVertex2f(-sz + offset.x, 0);
	glVertex2f(sz + offset.x, 0);
	glColor4f(0.2, 0.3, 0.6, alpha);
	glVertex2f(0, -sz + offset.y);
	glVertex2f(0, sz + offset.y);
	glEnd();

}

static void draw_curve(const Curve *curve)
{
	int numpt = curve->size();
	int segm = numpt * 16;

	glLineWidth(curve == hover_curve ? 4.0 : 2.0);
	if(curve == sel_curve) {
		glColor3f(0.3, 0.4, 1.0);
	} else if(curve == new_curve) {
		glColor3f(1.0, 0.75, 0.3);
	} else {
		glColor3f(0.6, 0.6, 0.6);
	}
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<segm; i++) {
		float t = (float)i / (float)(segm - 1);
		Vector2 v = curve->interpolate(t);
		glVertex2f(v.x, v.y);
	}
	glEnd();
	glLineWidth(1.0);

	glPointSize(curve == hover_curve ? 10.0 : 7.0);
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


	switch(key) {
	case 's':
		snap_mode = pressed ? SNAP_GRID : SNAP_NONE;
		break;

	case 'S':
		snap_mode = pressed ? SNAP_POINT : SNAP_NONE;
		break;

	default:
		break;
	}
}

static void calc_view_matrix()
{
	view_matrix.reset_identity();
	view_matrix.scale(Vector3(view_scale, view_scale, view_scale));
	view_matrix.translate(Vector3(view_pan.x, view_pan.y, 0.0));
}

static Vector2 pixel_to_uv(int x, int y)
{
	float u = win_aspect * (2.0 * (float)x / (float)win_width - 1.0);
	float v = 1.0 - 2.0 * (float)y / (float)win_height;

	u = u / view_scale - view_pan.x;
	v = v / view_scale - view_pan.y;
	return Vector2(u, v);
	/*
	Matrix4x4 inv_view_matrix = view_matrix.inverse();
	Vector4 res = Vector4(u, v, 0.0, 1.0).transformed(inv_view_matrix);

	return Vector2(res.x, res.y);
	*/
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

static bool point_hit_test(const Vector2 &pos, Curve **curveret, int *pidxret)
{
	float thres = 0.02 / view_scale;

	for(size_t i=0; i<curves.size(); i++) {
		int pidx = curves[i]->nearest_point(pos);
		if(pidx == -1) continue;

		Vector2 cp = curves[i]->get_point(pidx);
		if((cp - pos).length_sq() < thres * thres) {
			*curveret = curves[i];
			*pidxret = pidx;
			return true;
		}
	}
	*curveret = 0;
	*pidxret = -1;
	return false;
}

static Vector2 snap(const Vector2 &p)
{
	switch(snap_mode) {
	case SNAP_GRID:
		return Vector2(round(p.x / grid_size) * grid_size, round(p.y / grid_size) * grid_size);
	case SNAP_POINT:
		{
			Curve *nearest_curve = 0;
			int nearest_curve_pidx = -1;
			float nearest_dist_sq = FLT_MAX;

			if(new_curve) {
				// find the closest point, ignoring the last
				for(int i=0; i<new_curve->size() - 1; i++) {
					Vector2 cp = new_curve->get_point(i);
					float distsq = (cp - p).length_sq();
					if(distsq < nearest_dist_sq) {
						nearest_curve = new_curve;
						nearest_dist_sq = distsq;
						nearest_curve_pidx = i;
					}
				}
			}


			for(size_t i=0; i<curves.size(); i++) {
				int pidx = curves[i]->nearest_point(p);
				Vector2 cp = curves[i]->get_point(pidx);
				float dist_sq = (cp - p).length_sq();
				if(dist_sq < nearest_dist_sq) {
					nearest_curve = curves[i];
					nearest_curve_pidx = pidx;
					nearest_dist_sq = dist_sq;
				}
			}

			if(nearest_curve) {
				return nearest_curve->get_point(nearest_curve_pidx);
			}
		}
		break;

	default:
		break;
	}
	return p;
}

void app_mouse_motion(int x, int y)
{
	Vector2 prev_uv = pixel_to_uv(prev_x, prev_y);

	int dx = x - prev_x;
	int dy = y - prev_y;
	prev_x = x;
	prev_y = y;

	if(!dx && !dy) return;

	Vector2 uv = pixel_to_uv(x, y);
#ifdef DRAW_MOUSE_POINTER
	mouse_pointer = uv;
	post_redisplay();
#endif

	/* when entering a new curve, have the last (extra) point following
	 * the mouse until it's entered by a click (see on_click).
	 */
	if(new_curve) {
		new_curve->move_point(new_curve->size() - 1, snap(uv));
		post_redisplay();
	}

	if(!new_curve && !bnstate) {
		// not dragging, highlight curve under mouse
		point_hit_test(uv, &hover_curve, &sel_pidx);
		post_redisplay();

	} else {
		// we're dragging with one or more buttons held down

		if(sel_curve && sel_pidx != -1) {
			// we have a curve and a point of the curve selected

			if(bnstate & BNBIT(0)) {
				// dragging point with left button: move it
				sel_curve->move_point(sel_pidx, snap(uv));
				post_redisplay();
			}

			if(bnstate & BNBIT(2)) {
				// dragging point with right button: change weight
				float w = sel_curve->get_weight(sel_pidx);
				w -= dy * 0.01;
				if(w < FLT_MIN) w = FLT_MIN;
				sel_curve->set_weight(sel_pidx, w);

				// popup floating weight label if not already there
				if(!weight_label) {
					weight_label = new Label;
				}
				weight_label->set_position(uv);
				weight_label->set_textf("w=%g", w);
				post_redisplay();
			}
		} else {
			// no selection, we're dragging in empty space: manipulate viewport
			Vector2 dir = uv - prev_uv;

			if(bnstate & (BNBIT(0) | BNBIT(1))) {
				// panning
				view_pan += dir;
				calc_view_matrix();
				post_redisplay();
			}
			if(bnstate & BNBIT(2)) {
				// zooming
				view_scale -= ((float)dy / (float)win_height) * view_scale * 5.0;
				if(view_scale < 1e-4) view_scale = 1e-4;
				calc_view_matrix();
				post_redisplay();
			}
		}
	}
}

static void on_click(int bn, float u, float v)
{
	Vector2 uv = Vector2(u, v);

	switch(bn) {
	case 0:	// ------- LEFT CLICK ------
		if(hover_curve) {
			// if we're hovering: click selects
			sel_curve = hover_curve;
			hover_curve = 0;
		} else if(sel_curve) {
			// if we have a selected curve: click adds point (enter new_curve mode)
			std::vector<Curve*>::iterator it = std::find(curves.begin(), curves.end(), sel_curve);
			assert(it != curves.end());
			curves.erase(it, it + 1);

			new_curve = sel_curve;
			sel_curve = 0;
			sel_pidx = -1;

			new_curve->add_point(uv);
		} else {
			// otherwise, click starts a new curve
			if(!new_curve) {
				new_curve = new Curve;
				new_curve->add_point(uv);
			}
			new_curve->add_point(uv);
		}
		post_redisplay();
		break;

	case 2:	// ------- RIGHT CLICK ------
		if(new_curve) {
			// in new-curve mode: finish curve (cancels last floating segment)
			new_curve->remove_point(new_curve->size() - 1);
			if(new_curve->empty()) {
				delete new_curve;
			} else {
				curves.push_back(new_curve);
			}
			new_curve = 0;

		} else if(sel_curve) {
			// in selected curve mode: delete control point or unselect
			Curve *hit_curve;
			int hit_pidx;
			if(point_hit_test(uv, &hit_curve, &hit_pidx) && hit_curve == sel_curve) {
				hit_curve->remove_point(hit_pidx);
				sel_pidx = -1;
			} else {
				sel_curve = 0;
				sel_pidx = -1;
			}
		}
		post_redisplay();
		break;

	default:
		break;
	}
}
