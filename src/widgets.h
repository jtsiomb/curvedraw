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
#ifndef WIDGETS_H_
#define WIDGETS_H_

#include <vmath/vmath.h>

class Widget {
protected:
	Vector2 pos;
	char *text;

public:
	Widget();
	virtual ~Widget();

	virtual void set_position(const Vector2 &p);
	virtual const Vector2 &get_position() const;

	virtual void set_text(const char *str);
	virtual void set_textf(const char *str, ...);
	virtual const char *get_text() const;

	virtual void draw() const = 0;
};

class Label : public Widget {
public:
	virtual void draw() const;
};

#endif	// WIDGETS_H_
