/*
curvedraw - a simple program to draw curves
Copyright (C) 2015  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef APP_H_
#define APP_H_

#include "curve.h"

enum SnapMode {
	SNAP_NONE,
	SNAP_GRID,
	SNAP_POINT
};


extern int win_width, win_height;
extern float win_aspect;

bool app_init(int argc, char **argv);
void app_cleanup();

void app_draw();
void app_reshape(int x, int y);
void app_keyboard(int key, bool pressed);
void app_mouse_button(int bn, bool pressed, int x, int y);
void app_mouse_motion(int x, int y);
void app_mouse_wheel(int rot);

void app_tool_clear();
bool app_tool_load(const char *fname);
bool app_tool_save(const char *fname);
bool app_tool_bgimage(const char *fname);
SnapMode app_tool_snap(SnapMode s);
CurveType app_tool_type(CurveType type);
void app_tool_delete();

void app_tool_snap_callback(void (*func)(SnapMode, void*), void *cls = 0);
void app_tool_type_callback(void (*func)(CurveType, void*), void *cls = 0);

void post_redisplay();	// in main.cc

#endif	// APP_H_
