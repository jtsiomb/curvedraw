#ifndef CURVEFILE_H_
#define CURVEFILE_H_

#include <stdio.h>
#include <list>
#include "curve.h"

bool save_curves(const char *fname, const Curve * const *curves, int count);
bool save_curves(FILE *fp, const Curve * const *curves, int count);

std::list<Curve*> load_curves(const char *fname);
std::list<Curve*> load_curves(FILE *fp);

#endif	// CURVEFILE_H_