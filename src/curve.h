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

	float map_param(float x) const;

public:
	Curve(CurveType type = CURVE_HERMITE);

	void set_type(CurveType type);
	CurveType get_type() const;

	void add_point(const Vector2 &p, float weight = 1.0f);
	bool remove_point(int idx);

	int nearest_point(const Vector2 &p);

	int get_point_count() const;
	const Vector3 &get_homo_point(int idx) const;	// homogeneous point
	Vector2 get_point(int idx) const;
	float get_weight(int idx) const;

	bool set_point(int idx, const Vector2 &p, float weight = 1.0f);
	bool set_weight(int idx, float weight);

	Vector2 interpolate(float t, CurveType type) const;
	Vector2 interpolate(float t) const;
	Vector2 operator ()(float t) const;
};

#endif	// CURVE_H_
