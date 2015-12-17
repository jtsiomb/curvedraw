#ifndef CURVE_H_
#define CURVE_H_

#include <vector>
#include <vmath/vmath.h>

enum CurveType {
	CURVE_LINEAR,
	CURVE_HERMITE,
	CURVE_BSPLINE
};

class Curve {
private:
	std::vector<Vector3> cp;
	CurveType type;

public:
	Curve(CurveType type = CURVE_HERMITE);

	void set_type(CurveType type);
	CurveType get_type() const;

	void add_point(const Vector2 &p, float weight = 1.0f);
	bool remove_point(int idx);

	int nearest_point(const Vector2 &p);

	bool empty() const;
	int size() const;
	Vector3 &operator [](int idx);
	const Vector3 &operator [](int idx) const;

	const Vector3 &get_homo_point(int idx) const;	// homogeneous point
	Vector2 get_point(int idx) const;
	float get_weight(int idx) const;

	bool set_point(int idx, const Vector2 &p, float weight = 1.0f);
	bool set_weight(int idx, float weight);
	// move point without changing its weight
	bool move_point(int idx, const Vector2 &p);

	Vector2 interpolate(float t, CurveType type) const;
	Vector2 interpolate(float t) const;
	Vector2 operator ()(float t) const;

	void draw(int res = -1) const;
	void draw_cp(float sz = -1.0f) const;
};

#endif	// CURVE_H_
