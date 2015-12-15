#include <float.h>
#include "curve.h"

Curve::Curve(CurveType type)
{
	this->type = type;
}

void Curve::set_type(CurveType type)
{
	this->type = type;
}

CurveType Curve::get_type() const
{
	return type;
}

void Curve::add_point(const Vector2 &p, float weight)
{
	cp.push_back(Vector3(p.x, p.y, weight));
}

bool Curve::remove_point(int idx)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp.erase(cp.begin() + idx);
	return true;
}

int Curve::nearest_point(const Vector2 &p)
{
	int res = -1;
	float bestsq = FLT_MAX;

	for(size_t i=0; i<cp.size(); i++) {
		float d = (get_point(i) - p).length_sq();
		if(d < bestsq) {
			bestsq = d;
			res = i;
		}
	}

	return res;
}

int Curve::get_point_count() const
{
	return (int)cp.size();
}

const Vector3 &Curve::get_homo_point(int idx) const
{
	return cp[idx];
}

Vector2 Curve::get_point(int idx) const
{
	return Vector2(cp[idx].x, cp[idx].y);
}

float Curve::get_weight(int idx) const
{
	return cp[idx].z;
}

bool Curve::set_point(int idx, const Vector2 &p, float weight)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx] = Vector3(p.x, p.y, weight);
	return true;
}

bool Curve::set_weight(int idx, float weight)
{
	if(idx < 0 || idx >= (int)cp.size()) {
		return false;
	}
	cp[idx].z = weight;
	return true;
}

Vector2 Curve::interpolate(float t, CurveType type) const
{
	int num_cp = (int)cp.size();
	if(!num_cp) {
		return Vector2(0, 0);
	}
	if(num_cp == 1) {
		return Vector2(cp[0].x, cp[0].y);
	}

	Vector3 res;
	int idx0 = (int)floor(t * (num_cp - 1));
	int idx1 = idx0 + 1;

	float dt = 1.0 / (float)(num_cp - 1);
	float t0 = (float)idx0 * dt;
	float t1 = (float)idx1 * dt;

	t = (t - t0) / (t1 - t0);
	if(t < 0.0) t = 0.0;
	if(t > 1.0) t = 1.0;

	if(type == CURVE_LINEAR || num_cp <= 2) {
		res = lerp(cp[idx0], cp[idx1], t);
	} else {
		int idx_prev = idx0 <= 0 ? idx0 : idx0 - 1;
		int idx_next = idx1 >= num_cp - 1 ? idx1 : idx1 + 1;

		if(type == CURVE_HERMITE) {
			res = catmull_rom_spline(cp[idx_prev], cp[idx0], cp[idx1], cp[idx_next], t);
		} else {
			res = bspline(cp[idx_prev], cp[idx0], cp[idx1], cp[idx_next], t);
			if(res.z != 0.0f) {
				res.x /= res.z;
				res.y /= res.z;
			}
		}
	}

	return Vector2(res.x, res.y);
}

Vector2 Curve::interpolate(float t) const
{
	return interpolate(t, type);
}

Vector2 Curve::operator ()(float t) const
{
	return interpolate(t);
}

float Curve::map_param(float x) const
{
	if(x < 0.0f) x = 0.0f;
	if(x >= 1.0f) x = 1.0f;
	return x * ((float)cp.size() - 1);
}
