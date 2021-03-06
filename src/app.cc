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
#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <imago2.h>
#include "opengl.h"
#include "app.h"
#include "curve.h"
#include "widgets.h"
#include "curvefile.h"

int win_width, win_height;
float win_aspect;

static void draw_grid(float sz, float sep, float alpha = 1.0f);
static void draw_curve(const Curve *curve);
static void draw_bgimage(float sz, float alpha = 1.0f);
static void on_click(int bn, float u, float v);
static int curve_index(const Curve *curve);
static Vector2 snap(const Vector2 &p);

// viewport control
static Vector2 view_pan;
static float view_scale = 0.2f;
static Matrix4x4 view_matrix;

static float grid_size = 1.0;
static SnapMode snap_mode;
static CurveType curve_type = CURVE_HERMITE;

static bool show_bounds;

static std::vector<Curve*> curves;
static Curve *sel_curve;	// selected curve being edited
static Curve *new_curve;	// new curve being entered
static Curve *hover_curve;	// curve the mouse is hovering over (click to select)
static int sel_pidx = -1;	// selected point of the selected curve
static int hover_pidx = -1;	// hovered over point

static Label *weight_label;	// floating label for the cp weight

static Vector2 mouse_pointer;

static unsigned int tex_bg;
static float bg_aspect = 1.0f;

// state change callbacks
static void (*snap_change_callback)(SnapMode, void*);
static void *snap_change_callback_cls;
static void (*type_change_callback)(CurveType, void*);
static void *type_change_callback_cls;
static void (*showbbox_callback)(bool, void*);
static void *showbbox_callback_cls;


bool app_init(int argc, char **argv)
{
	//glewInit();

	glEnable(GL_MULTISAMPLE);
	glGetError();	// eat the error if multisampling isn't supported

	glEnable(GL_CULL_FACE);
	glEnable(GL_POINT_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return true;
}

void app_cleanup()
{
	app_tool_clear();
}

void app_draw()
{
	glClearColor(0.1, 0.1, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(view_pan.x * view_scale, view_pan.y * view_scale, 0);
	glScalef(view_scale, view_scale, view_scale);

	if(tex_bg) {
		draw_bgimage(1.0, 0.5);
	}

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

	if(show_bounds) {
		Vector3 bmin, bmax;
		curve->get_bbox(&bmin, &bmax);

		glLineWidth(1.0);
		glColor3f(0, 1, 0);
		glBegin(GL_LINE_LOOP);
		glVertex2f(bmin.x, bmin.y);
		glVertex2f(bmax.x, bmin.y);
		glVertex2f(bmax.x, bmax.y);
		glVertex2f(bmin.x, bmax.y);
		glEnd();
	}

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
		Vector3 v = curve->interpolate(t);
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
		Vector2 pt = curve->get_point2(i);
		glVertex2f(pt.x, pt.y);
	}
	glEnd();

	// draw the projected mouse point on the selected curve
	if(curve == sel_curve && sel_pidx == -1) {
		Vector3 pp = curve->proj_point(Vector3(mouse_pointer.x, mouse_pointer.y, 0.0));

		glPointSize(5.0);
		glBegin(GL_POINTS);
		glColor3f(1, 0.8, 0.2);
		glVertex2f(pp.x, pp.y);
		glEnd();
	}
	glPointSize(1.0);
}

void draw_bgimage(float sz, float alpha)
{
	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_bg);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, alpha);
	glTexCoord2f(0, 1);
	glVertex2f(-bg_aspect, -1);
	glTexCoord2f(1, 1);
	glVertex2f(bg_aspect, -1);
	glTexCoord2f(1, 0);
	glVertex2f(bg_aspect, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-bg_aspect, 1);
	glEnd();

	glPopAttrib();
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

		case '\b':
		case 127:	/* delete */
			app_tool_delete();
			break;

		case '1':
		case '2':
		case '3':
			app_tool_type((CurveType)((int)CURVE_LINEAR + key - '1'));
			break;

		case 'b':
		case 'B':
			app_tool_showbbox(!show_bounds);
			break;

		case 'n':
		case 'N':
			if(sel_curve) {
				sel_curve->normalize();
				post_redisplay();
			}
			break;

		case 'e':
		case 'E':
			app_tool_save("test.curves");
			break;

		case 'l':
		case 'L':
			if(app_tool_load("test.curves")) {
				post_redisplay();
			}
			break;

		case 'i':
		case 'I':
			if(app_tool_bgimage("bg.png")) {
				post_redisplay();
			}
			break;
		}
	}


	switch(key) {
	case 's':
		app_tool_snap(pressed ? SNAP_GRID : SNAP_NONE);
		break;

	case 'S':
		app_tool_snap(pressed ? SNAP_POINT : SNAP_NONE);
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

#define HIT_TEST_THRES	(0.02 / view_scale)

static bool point_hit_test(const Vector2 &pos, Curve **curveret, int *pidxret)
{
	float thres = HIT_TEST_THRES;

	for(size_t i=0; i<curves.size(); i++) {
		int pidx = curves[i]->nearest_point(pos);
		if(pidx == -1) continue;

		Vector2 cp = curves[i]->get_point2(pidx);
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

static bool hit_test(const Vector2 &pos, Curve **curveret, int *pidxret)
{
	float thres = HIT_TEST_THRES;

	if(point_hit_test(pos, curveret, pidxret)) {
		return true;
	}

	Vector3 pos3 = Vector3(pos.x, pos.y, 0.0f);
	for(size_t i=0; i<curves.size(); i++) {
		float x;
		if((x = curves[i]->distance_sq(pos3)) < thres * thres) {
			*curveret = curves[i];
			*pidxret = -1;
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
	mouse_pointer = uv;
	//post_redisplay();

	/* when entering a new curve, have the last (extra) point following
	 * the mouse until it's entered by a click (see on_click).
	 */
	if(new_curve) {
		new_curve->move_point(new_curve->size() - 1, snap(uv));
		post_redisplay();
	}

	if(!new_curve && !bnstate) {
		// not dragging, highlight curve under mouse
		hit_test(uv, &hover_curve, &hover_pidx);
		if(hover_curve == sel_curve) {
			sel_pidx = hover_pidx;
		}
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

void app_mouse_wheel(int rot)
{
	view_scale += (float)rot * view_scale * 0.025;
	if(view_scale < 1e-4) view_scale = 1e-4;
	calc_view_matrix();
	post_redisplay();
}

static void on_click(int bn, float u, float v)
{
	Vector2 uv = Vector2(u, v);

	switch(bn) {
	case 0:	// ------- LEFT CLICK ------
		if(hover_curve && hover_curve != sel_curve) {
			// if we're hovering: click selects
			sel_curve = hover_curve;
			sel_pidx = hover_pidx;
			hover_curve = 0;
		} else if(sel_curve) {
			// if we have a selected curve: click adds point
			Vector3 uv3 = Vector3(uv.x, uv.y, 0.0);
			float proj_t = sel_curve->proj_param(uv3);

			if(proj_t >= 0.0 && proj_t < 1.0) {
				// insert somewhere in the middle
				sel_curve->insert_point(sel_curve->interpolate(proj_t));
			} else {
				// enter new curve mode and start appending more points
				int cidx = curve_index(sel_curve);
				assert(cidx != -1);
				curves.erase(curves.begin() + cidx);

				new_curve = sel_curve;
				sel_curve = 0;
				sel_pidx = -1;

				new_curve->add_point(uv);
			}
		} else {
			// otherwise, click starts a new curve
			if(!new_curve) {
				new_curve = new Curve;
				new_curve->set_type(curve_type);
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
			if(hit_test(uv, &hit_curve, &hit_pidx) && hit_curve == sel_curve) {
				if(hit_pidx != -1) {
					hit_curve->remove_point(hit_pidx);
					sel_pidx = -1;
					if(hit_curve->empty()) {	// removed the last point
						int cidx = curve_index(sel_curve);
						assert(cidx != -1);
						curves.erase(curves.begin() + cidx);
						delete sel_curve;
						sel_curve = 0;
						sel_pidx = -1;
					}
				}
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

static int curve_index(const Curve *curve)
{
	for(size_t i=0; i<curves.size(); i++) {
		if(curves[i] == curve) {
			return i;
		}
	}
	return -1;
}


// ---- app tool functions ----
void app_tool_clear()
{
	for(size_t i=0; i<curves.size(); i++) {
		delete curves[i];
	}
	curves.clear();
	delete new_curve;
	sel_curve = new_curve = hover_curve = 0;
	sel_pidx = -1;
	hover_pidx = -1;
}

bool app_tool_load(const char *fname)
{
	std::list<Curve*> clist = load_curves(fname);
	if(clist.empty()) {
		fprintf(stderr, "failed to load curves from: %s\n", fname);
		return false;
	}

	app_tool_clear();

	int num = 0;
	std::list<Curve*>::iterator it = clist.begin();
	while(it != clist.end()) {
		curves.push_back(*it++);
		++num;
	}
	printf("imported %d curves from %s\n", num, fname);
	return true;
}

bool app_tool_save(const char *fname)
{
	if(!save_curves(fname, &curves[0], (int)curves.size())) {
		fprintf(stderr, "failed to export curves to %s\n", fname);
		return false;
	}
	printf("exported %d curves to %s\n", (int)curves.size(), fname);
	return true;
}

bool app_tool_bgimage(const char *fname)
{
	int width, height;
	unsigned int tex = 0;

	if(fname) {
		if(!(tex = img_gltexture_load(fname))) {
			fprintf(stderr, "failed to load background image: %s\n", fname);
			return false;
		}
		printf("loaded background image: %s\n", fname);

		glBindTexture(GL_TEXTURE_2D, tex);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		bg_aspect = (float)width / (float)height;
	}

	if(tex_bg) {
		glDeleteTextures(1, &tex_bg);
	}
	tex_bg = tex;
	post_redisplay();
	return true;
}

SnapMode app_tool_snap(SnapMode s)
{
	SnapMode prev = snap_mode;
	snap_mode = s;
	if(snap_change_callback) {
		snap_change_callback(snap_mode, snap_change_callback_cls);
	}
	return prev;
}

CurveType app_tool_type(CurveType type)
{
	CurveType prev = curve_type;
	curve_type = type;

	if(sel_curve) {
		sel_curve->set_type(type);
		post_redisplay();
	}
	if(new_curve) {
		new_curve->set_type(type);
		post_redisplay();
	}

	if(type_change_callback) {
		type_change_callback(type, type_change_callback_cls);
	}
	return prev;
}

void app_tool_delete()
{
	if(new_curve) {
		delete new_curve;
		new_curve = 0;
		post_redisplay();
	} else if(sel_curve) {
		int cidx = curve_index(sel_curve);
		assert(cidx != -1);
		curves.erase(curves.begin() + cidx);

		delete sel_curve;
		sel_curve = 0;
		sel_pidx = -1;
		post_redisplay();
	}
}

void app_tool_showbbox(bool show)
{
	show_bounds = show;
	post_redisplay();

	if(showbbox_callback) {
		showbbox_callback(show, showbbox_callback_cls);
	}
}

void app_tool_snap_callback(void (*func)(SnapMode, void*), void *cls)
{
	snap_change_callback = func;
	snap_change_callback_cls = cls;
}

void app_tool_type_callback(void (*func)(CurveType, void*), void *cls)
{
	type_change_callback = func;
	type_change_callback_cls = cls;
}

void app_tool_showbbox_callback(void (*func)(bool, void*), void *cls)
{
	showbbox_callback = func;
	showbbox_callback_cls = cls;
}
