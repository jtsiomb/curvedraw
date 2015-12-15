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
