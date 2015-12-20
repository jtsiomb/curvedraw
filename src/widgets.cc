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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include <drawtext.h>
#include "opengl.h"
#include "widgets.h"

#define FONT_FILE	"data/droid_sans.ttf"
#define FONT_SIZE	24

static dtx_font *font;

static bool init_font()
{
	if(font) return true;

	if(!(font = dtx_open_font(FONT_FILE, FONT_SIZE))) {
		static bool msg_printed;
		if(!msg_printed) {
			fprintf(stderr, "failed to load font %s\n", FONT_FILE);
			msg_printed = true;
		}
		return false;
	}
	return true;
}

Widget::Widget()
{
	text = 0;
}

Widget::~Widget()
{
	delete [] text;
}

void Widget::set_position(const Vector2 &p)
{
	pos = p;
}

const Vector2 &Widget::get_position() const
{
	return pos;
}

void Widget::set_text(const char *str)
{
	char *newtext = new char[strlen(str) + 1];
	strcpy(newtext, str);

	delete [] text;
	text = newtext;
}

void Widget::set_textf(const char *str, ...)
{
	va_list ap;
	int sz = strlen(str) * 4;
	char *buf = (char*)alloca(sz + 1);

	va_start(ap, str);
	vsnprintf(buf, sz, str, ap);
	va_end(ap);

	set_text(buf);
}

const char *Widget::get_text() const
{
	return text;
}

// ---- label ----

void Label::draw() const
{
	if(!init_font()) return;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0);
	glScalef(0.003, 0.003, 1);

	glColor4f(1, 1, 1, 1);
	dtx_string(text);
	dtx_flush();

	glPopMatrix();
}
