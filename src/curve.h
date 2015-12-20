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
	std::vector<Vector4> cp;
	CurveType type;

	// bounding box
	mutable Vector3 bbmin, bbmax;
	mutable bool bbvalid;

	void calc_bounds() const;
	void inval_bounds() const;

public:
	Curve(CurveType type = CURVE_HERMITE);
	Curve(const Vector4 *cp, int numcp, CurveType type = CURVE_HERMITE); // homogenous
	Curve(const Vector3 *cp, int numcp, CurveType type = CURVE_HERMITE); // 3D points, w=1
	Curve(const Vector2 *cp, int numcp, CurveType type = CURVE_HERMITE); // 2D points, z=0, w=1

	void set_type(CurveType type);
	CurveType get_type() const;

	void add_point(const Vector4 &p);
	void add_point(const Vector3 &p, float weight = 1.0f);
	void add_point(const Vector2 &p, float weight = 1.0f);
	bool remove_point(int idx);

	void clear();		// remove all control points
	bool empty() const;	// true if 0 control points
	int size() const;	// returns number of control points
	// access operators for control points
	Vector4 &operator [](int idx);
	const Vector4 &operator [](int idx) const;
	const Vector4 &get_point(int idx) const;

	Vector3 get_point3(int idx) const;
	Vector2 get_point2(int idx) const;
	float get_weight(int idx) const;

	bool set_point(int idx, const Vector3 &p, float weight = 1.0f);
	bool set_point(int idx, const Vector2 &p, float weight = 1.0f);
	bool set_weight(int idx, float weight);
	// move point without changing its weight
	bool move_point(int idx, const Vector3 &p);
	bool move_point(int idx, const Vector2 &p);
 
	int nearest_point(const Vector3 &p) const;
	// nearest control point on the 2D plane z=0
	int nearest_point(const Vector2 &p) const;

	/* get_bbox returns the axis-aligned bounding box of the curve's
	 * control points.
	 * NOTE: hermite curves can go outside of the bounding box of their control points
	 * NOTE: lazy calculation of bounds is performed, use calc_bbox in multithreaded programs
	 */
	void get_bbox(Vector3 *bbmin, Vector3 *bbmax) const;
	void calc_bbox(Vector3 *bbmin, Vector3 *bbmax) const;
	// normalize the curve's bounds to coincide with the unit cube
	void normalize();

	// project a point to the curve (nearest point on the curve)
	Vector3 proj_point(const Vector3 &p, float refine_thres = 0.01) const;
	// equivalent to (proj_point(p) - p).length()
	float distance(const Vector3 &p) const;
	// equivalent to fabs((proj_point(p) - p).length_sq())
	float distance_sq(const Vector3 &p) const;

	Vector3 interpolate_segment(int a, int b, float t) const;
	Vector3 interpolate(float t) const;
	Vector2 interpolate2(float t) const;
	Vector3 operator ()(float t) const;
};

#endif	// CURVE_H_
