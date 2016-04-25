#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

using namespace boost;
using namespace std;

#include "tsp-types.h"
#include "tsp-hkbound.h"
#include "tsp-tsp.h"


typedef property<edge_weight_t, int> ewProperty;
typedef adjacency_list< slistS, vecS, undirectedS,
			no_property, ewProperty > Graph;

typedef Graph::edge_descriptor Edge;



int lower_bound_using_hk(tsp_path_t path, int hops, int len, uint64_t vpres) {
  Graph g;

  /* construire le graph complet avec les villes et arêtes restantes */
  for (int i = 0; i < nb_towns; i++) {
    if ( present( i, hops, path, vpres ) )
      continue;

    /* aretes vers 0 et la dernière ville */
    add_edge( 0, i, tsp_distance[0][i], g);
    add_edge( i, path[hops-1], tsp_distance[i][ path[hops-1] ], g);

    for(int j = i+1; j < nb_towns; j++) {
      if (present( j , hops, path, vpres) )
	continue;
      add_edge( i, j, tsp_distance[i][j], g);
    }
  }

  std::list < Edge > spt;
  kruskal_minimum_spanning_tree ( g, std::back_inserter(spt) );

  int somme = 0;
  for(auto const & e: spt) {
    somme += get(edge_weight, g)[e];
  }

  return somme + len;
}
