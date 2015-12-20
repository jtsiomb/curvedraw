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
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include "curvefile.h"

static bool save_curve(FILE *fp, const Curve *curve);

bool save_curves(const char *fname, const Curve * const *curves, int count)
{
	FILE *fp = fopen(fname, "wb");
	if(!fp) return false;

	bool res = save_curves(fp, curves, count);
	fclose(fp);
	return res;
}

bool save_curves(FILE *fp, const Curve * const *curves, int count)
{
	fprintf(fp, "GCURVES\n");

	for(int i=0; i<count; i++) {
		if(!save_curve(fp, curves[i])) {
			return false;
		}
	}
	return true;
}

static const char *curve_type_str(CurveType type)
{
	switch(type) {
	case CURVE_LINEAR:
		return "polyline";
	case CURVE_HERMITE:
		return "hermite";
	case CURVE_BSPLINE:
		return "bspline";
	}
	abort();
}

static bool save_curve(FILE *fp, const Curve *curve)
{
	fprintf(fp, "curve {\n");
	fprintf(fp, "    type %s\n", curve_type_str(curve->get_type()));
	fprintf(fp, "    cpcount %d\n", curve->size());
	for(int i=0; i<curve->size(); i++) {
		Vector4 cp = curve->get_point(i);
		fprintf(fp, "    cp %g %g %g %g\n", cp.x, cp.y, cp.z, cp.w);
	}
	fprintf(fp, "}\n");
	return true;
}

std::list<Curve*> load_curves(const char *fname)
{
	std::list<Curve*> res;
	FILE *fp = fopen(fname, "r");
	if(!fp) return res;

	res = load_curves(fp);
	fclose(fp);
	return res;
}

static std::string next_token(FILE *fp)
{
	std::string s;
	int c;
	while((c = fgetc(fp)) != -1) {
		if(!isspace(c)) {
			ungetc(c, fp);
			break;
		}
	}

	if(feof(fp)) return s;
	while((c = fgetc(fp)) != -1) {
		if(isspace(c)) {
			ungetc(c, fp);
			break;
		}
		s.push_back(c);
	}
	return s;
}

static bool expect_str(FILE *fp, const char *s)
{
	std::string tok = next_token(fp);
	if(tok != std::string(s)) {
		if(tok.empty() && feof(fp)) {
			fprintf(stderr, "expected: %s\n", s);
		}
		return false;
	}
	return true;
}

static bool expect_float(FILE *fp, float *ret)
{
	std::string tok = next_token(fp);
	const char *cs = tok.c_str();
	char *endp;
	*ret = strtod(cs, &endp);
	if(endp != cs + tok.length()) {
		if(!(tok.empty() && feof(fp))) {
			fprintf(stderr, "number expected\n");
		}
		return false;
	}
	return true;
}

static bool expect_int(FILE *fp, int *ret)
{
	std::string tok = next_token(fp);
	const char *cs = tok.c_str();
	char *endp;
	*ret = strtol(cs, &endp, 0);
	if(endp != cs + tok.length()) {
		if(!(tok.empty() && feof(fp))) {
			fprintf(stderr, "integer expected\n");
		}
		return false;
	}
	return true;
}

static Curve *curve_block(FILE *fp)
{
	if(!expect_str(fp, "curve") || !expect_str(fp, "{")) {
		return 0;
	}

	Curve *curve = new Curve;
	int cpcount = -1;
	std::string tok;
	while(!(tok = next_token(fp)).empty() && tok != "}") {
		if(tok == "cpcount") {
			if(cpcount != -1 || !expect_int(fp, &cpcount) || cpcount <= 0) {
				goto err;
			}
		} else if(tok == "type") {
			tok = next_token(fp);
			if(tok == "polyline") {
				curve->set_type(CURVE_LINEAR);
			} else if(tok == "hermite") {
				curve->set_type(CURVE_HERMITE);
			} else if(tok == "bspline") {
				curve->set_type(CURVE_BSPLINE);
			} else {
				goto err;
			}
		} else {
			if(tok != "cp") {
				goto err;
			}
			Vector4 cp;
			for(int i=0; i<4; i++) {
				if(!expect_float(fp, &cp[i])) {
					goto err;
				}
			}
			curve->add_point(Vector3(cp.x, cp.y, cp.z), cp.w);
		}
	}

	if(curve->size() != cpcount) {
		fprintf(stderr, "warning: curve cpcount was %d, but read %d control points\n", cpcount, curve->size());
	}

	return curve;
err:
	fprintf(stderr, "failed to parse curve block\n");
	delete curve;
	return 0;
}

std::list<Curve*> load_curves(FILE *fp)
{
	std::list<Curve*> curves;
	if(!expect_str(fp, "GCURVES")) {
		fprintf(stderr, "load_curves: failed to load, invalid file format\n");
		return curves;
	}

	Curve *curve;
	while((curve = curve_block(fp))) {
		curves.push_back(curve);
	}

	if(!feof(fp)) {
		std::list<Curve*>::iterator it = curves.begin();
		while(it != curves.end()) {
			delete *it++;
		}
		return curves;
	}

	return curves;
}

