#ifndef CURVEFILE_H_
#define CURVEFILE_H_

#include <stdio.h>
#include "curve.h"

bool save_curves(const char *fname, const Curve * const *curves, int count);
bool save_curves(FILE *fp, const Curve * const *curves, int count);

Curve **load_curves(const char *fname, int *countret);
Curve **load_curves(FILE *fp, int *countret);

#endif	// CURVEFILE_H_