#include <stdlib.h>
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
	fprintf(stderr, "GCURVES\ncount %d\n", count);

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

Curve **load_curves(const char *fname, int *countret)
{
	FILE *fp = fopen(fname, "rb");
	if(!fp) return false;

	Curve **res = load_curves(fp, countret);
	fclose(fp);
	return res;
}

Curve **load_curves(FILE *fp, int *countret)
{
	return 0;	// TODO
}

