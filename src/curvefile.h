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
