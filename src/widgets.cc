#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <drawtext.h>
#include "opengl.h"
#include "widgets.h"

static dtx_font *font;

static void init_font()
{
	if(font) return;

	if(!(font = dtx_open_font("data/droid_sans.ttf", 16))) {
		fprintf(stderr, "failed to load font\n");
		abort();
	}
	dtx_use_font(font, 16);
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
	text = new char[strlen(str) + 1];
	strcpy(text, str);
}

void Widget::set_textf(const char *str, ...)
{
	va_list ap;
	int sz = strlen(str) * 4;
	char *buf = (char*)alloca(sz);

	va_start(ap, str);
	vsnprintf(buf, sz - 1, text, ap);
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
	init_font();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos.x, pos.y, 0);
	glScalef(0.1, 0.1, 1);

	dtx_string(text);
	dtx_flush();

	glPopMatrix();
}
