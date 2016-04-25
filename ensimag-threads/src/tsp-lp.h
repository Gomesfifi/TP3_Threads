#ifndef TSP_LP_H
#define TSP_LP_H

#include <stdio.h>
#include "tsp-types.h"

void save_lp(FILE* f, tsp_path_t path, int hops, int len, uint64_t vpres);
int lower_bound_using_lp(tsp_path_t path, int hops, int len, uint64_t vpres);

#endif
