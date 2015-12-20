#include <float.h>
#include <assert.h>
#include <algorithm>
#include "curve.h"

Curve::Curve(CurveType type)
{
	this->type = type;
	bbvalid = true;
}

Curve::Curve(const Vector4 *cp, int numcp, CurveType type)
	: Curve(type)
{
	this->cp.resize(numcp);
	for(int i=0; i<numcp; i++) {
		this->cp[i] = cp[i];
	}
}

Curve::Curve(const Vector3 *cp, int numcp, CurveType type)
	: Curve(type)
{
	this->cp.resize(numcp);
	for(int i=0; i<numcp; i++) {
		this->cp[i] = Vector4(cp[i].x, cp[i].y, cp[i].z, 1.0f);
	}
}

Curve::Curve(const Vector2 *cp, int numcp, CurveType type)
	: Curve(type)
{
	this->cp.resize(numcp);
	for(int i=0; i<numcp; i++) {
		this->cp[i] = Vector4(cp[i].x, cp[i].y, 0.0f, 1.0f);
	}
}

void Curve::set_type(CurveType type)
{
	this->type = type;
}

CurveType Curve::get_type() const
{
	return type;
}

void Curve::add_point(const Vector4 &p)
{
	cp.push_back(p);
	inval_bounds();
}

void Curve::add_point(const Vector3 &p, float weight)
{
	add_point(Vector4(p.x, p.y, p.z, weight));
}

void Curve::add_point(const Vector2 &p, float weight)
{
	add_point(Vector4(p.x, p.y, 0.0f, weight));
}

bool Curve::remove_point(int idx)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp.erase(cp.begin() + idx);
	inval_bounds();
	return true;
}

void Curve::clear()
{
	cp.clear();
	inval_bounds();
}

bool Curve::empty() const
{
	return cp.empty();
}

int Curve::size() const
{
	return (int)cp.size();
}

Vector4 &Curve::operator [](int idx)
{
	inval_bounds();
	return cp[idx];
}

const Vector4 &Curve::operator [](int idx) const
{
	return cp[idx];
}

const Vector4 &Curve::get_point(int idx) const
{
	return cp[idx];
}

Vector3 Curve::get_point3(int idx) const
{
	return Vector3(cp[idx].x, cp[idx].y, cp[idx].z);
}

Vector2 Curve::get_point2(int idx) const
{
	return Vector2(cp[idx].x, cp[idx].y);
}

float Curve::get_weight(int idx) const
{
	return cp[idx].w;
}

bool Curve::set_point(int idx, const Vector3 &p, float weight)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx] = Vector4(p.x, p.y, p.z, weight);
	inval_bounds();
	return true;
}

bool Curve::set_point(int idx, const Vector2 &p, float weight)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx] = Vector4(p.x, p.y, 0.0, weight);
	inval_bounds();
	return true;
}

bool Curve::set_weight(int idx, float weight)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx].w = weight;
	return true;
}

bool Curve::move_point(int idx, const Vector3 &p)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx] = Vector4(p.x, p.y, p.z, cp[idx].w);
	inval_bounds();
	return true;
}

bool Curve::move_point(int idx, const Vector2 &p)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx] = Vector4(p.x, p.y, 0.0f, cp[idx].w);
	inval_bounds();
	return true;
}


int Curve::nearest_point(const Vector3 &p) const
{
	int res = -1;
	float bestsq = FLT_MAX;

	for(size_t i=0; i<cp.size(); i++) {
		float d = (get_point3(i) - p).length_sq();
		if(d < bestsq) {
			bestsq = d;
			res = i;
		}
	}
	return res;
}

int Curve::nearest_point(const Vector2 &p) const
{
	int res = -1;
	float bestsq = FLT_MAX;

	for(size_t i=0; i<cp.size(); i++) {
		float d = (get_point2(i) - p).length_sq();
		if(d < bestsq) {
			bestsq = d;
			res = i;
		}
	}
	return res;
}


void Curve::inval_bounds() const
{
	bbvalid = false;
}

void Curve::calc_bounds() const
{
	calc_bbox(&bbmin, &bbmax);
	bbvalid = true;
}

void Curve::get_bbox(Vector3 *bbmin, Vector3 *bbmax) const
{
	if(!bbvalid) {
		calc_bounds();
	}
	*bbmin = this->bbmin;
	*bbmax = this->bbmax;
}

void Curve::calc_bbox(Vector3 *bbmin, Vector3 *bbmax) const
{
	if(empty()) {
		*bbmin = *bbmax = Vector3(0, 0, 0);
		return;
	}

	Vector3 bmin = cp[0];
	Vector3 bmax = bmin;
	for(size_t i=1; i<cp.size(); i++) {
		const Vector4 &v = cp[i];
		for(int j=0; j<3; j++) {
			if(v[j] < bmin[j]) bmin[j] = v[j];
			if(v[j] > bmax[j]) bmax[j] = v[j];
		}
	}
	*bbmin = bmin;
	*bbmax = bmax;
}

void Curve::normalize()
{
	if(!bbvalid) {
		calc_bounds();
	}

	Vector3 bsize = bbmax - bbmin;
	Vector3 boffs = (bbmin + bbmax) * 0.5;

	Vector3 bscale;
	bscale.x = bsize.x == 0.0f ? 1.0f : 1.0f / bsize.x;
	bscale.y = bsize.y == 0.0f ? 1.0f : 1.0f / bsize.y;
	bscale.z = bsize.z == 0.0f ? 1.0f : 1.0f / bsize.z;

	for(size_t i=0; i<cp.size(); i++) {
		cp[i].x = (cp[i].x - boffs.x) * bscale.x;
		cp[i].y = (cp[i].y - boffs.y) * bscale.y;
		cp[i].z = (cp[i].z - boffs.z) * bscale.z;
	}
	inval_bounds();
}

/* Projection to the curve is not correct, but should be good enough for
 * most purposes.
 *
 * First we find the nearest segment (pair of control points), and then
 * we subdivide between them to find the nearest interpolated point in that
 * segment.
 * The incorrect assumption here is that the nearest segment as defined by
 * the distance of its control points to p, will contain the nearest point
 * on the curve. This only holds for polylines, and *possibly* bsplines, but
 * certainly not hermite splines.
 */
Vector3 Curve::proj_point(const Vector3 &p) const
{
	// TODO fix: select nearest segment based on point-line distance, not distance from the CPs
	int num_cp = size();
	if(num_cp <= 0) return p;
	if(num_cp == 1) return cp[0];

	int idx0 = nearest_point(p);
	int next_idx = idx0 + 1;
	int prev_idx = idx0 - 1;

	float next_distsq = next_idx >= num_cp ? FLT_MAX : (get_point3(next_idx) - p).length_sq();
	float prev_distsq = prev_idx < 0 ? FLT_MAX : (get_point3(prev_idx) - p).length_sq();
	int idx1 = next_distsq < prev_distsq ? next_idx : prev_idx;
	assert(idx1 >= 0 && idx1 < num_cp - 1);
	if(idx0 > idx1) std::swap(idx0, idx1);

	float t0 = 0.0f, t1 = 1.0f;
	Vector3 pp0 = interpolate_segment(idx0, idx1, 0.0f);
	Vector3 pp1 = interpolate_segment(idx0, idx1, 1.0f);
	float dist0 = (pp0 - p).length_sq();
	float dist1 = (pp1 - p).length_sq();
	Vector3 pp;

	for(int i=0; i<32; i++) {	// max iterations
		float t = (t0 + t1) / 2.0;
		pp = interpolate_segment(idx0, idx1, t);
		float dist = (pp - p).length_sq();

		// mid point more distant than both control points, nearest cp is closest
		if(dist > dist0 && dist > dist1) {
			pp = dist0 < dist1 ? pp0 : pp1;
			break;
		}

		if(dist0 < dist1) {
			t1 = t;
			dist1 = dist;
			pp1 = pp;
		} else {
			t0 = t;
			dist0 = dist;
			pp0 = pp;
		}

		if(fabs(dist0 - dist1) < 1e-4) {
			break;
		}
	}
	return pp;
}

Vector3 Curve::interpolate_segment(int a, int b, float t) const
{
	int num_cp = size();

	if(t < 0.0) t = 0.0;
	if(t > 1.0) t = 1.0;

	Vector4 res;
	if(type == CURVE_LINEAR || num_cp == 2) {
		res = lerp(cp[a], cp[b], t);
	} else {
		int prev = a <= 0 ? a : a - 1;
		int next = b >= num_cp - 1 ? b : b + 1;

		if(type == CURVE_HERMITE) {
			res = catmull_rom_spline(cp[prev], cp[a], cp[b], cp[next], t);
		} else {
			res = bspline(cp[prev], cp[a], cp[b], cp[next], t);
			if(res.w != 0.0f) {
				res.x /= res.w;
				res.y /= res.w;
			}
		}
	}

	return Vector3(res.x, res.y, res.z);
}

Vector3 Curve::interpolate(float t) const
{
	if(empty()) {
		return Vector3(0, 0, 0);
	}

	int num_cp = (int)cp.size();
	if(num_cp == 1) {
		return Vector3(cp[0].x, cp[0].y, cp[0].z);
	}

	int idx0 = std::min((int)floor(t * (num_cp - 1)), num_cp - 2);
	int idx1 = idx0 + 1;

	float dt = 1.0 / (float)(num_cp - 1);
	float t0 = (float)idx0 * dt;
	float t1 = (float)idx1 * dt;

	t = (t - t0) / (t1 - t0);

	return interpolate_segment(idx0, idx1, t);
}

Vector2 Curve::interpolate2(float t) const
{
	Vector3 res = interpolate(t);
	return Vector2(res.x, res.y);
}

Vector3 Curve::operator ()(float t) const
{
	return interpolate(t);
}
