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
	fprintf(stderr, "GCURVES\n");

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
		Vector3 cp = curve->get_homo_point(i);
		fprintf(fp, "    cp %g %g %g\n", cp.x, cp.y, cp.z);
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
	while((c = fgetc(fp)) != -1 && isspace(c));	// skip whitespace
	if(feof(fp)) return s;
	while((c = fgetc(fp)) != -1 && !isspace(c)) {
		s.push_back(c);
	}
	return s;
}

static bool expect_str(FILE *fp, const char *s)
{
	std::string tok = next_token(fp);
	if(tok != std::string(s)) {
		if(!(tok.empty() && feof(fp))) {
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
			if(!expect_str(fp, "cp")) {
				goto err;
			}
			Vector3 cp;
			for(int i=0; i<3; i++) {
				if(!expect_float(fp, &cp[i])) {
					goto err;
				}
			}
			curve->add_point(Vector2(cp.x, cp.y), cp.z);
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

	int ncurves = (int)curves.size();
	if(!feof(fp)) {
		std::list<Curve*>::iterator it = curves.begin();
		while(it != curves.end()) {
			delete *it++;
		}
		return curves;
	}

	return curves;
}

