#ifndef TSP_HKBOUND_H
#define TSPHKBOUND_H

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "tsp-types.h"

  
  int lower_bound_using_hk(tsp_path_t path, int hops, int len, uint64_t vpres);


#ifdef __cplusplus
}
#endif

#endif
