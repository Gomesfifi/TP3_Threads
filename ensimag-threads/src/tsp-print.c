#include <stdio.h>

#include "tsp-print.h"
#include "tsp-types.h"
#include "tsp-genmap.h"


/* impression tableau des distances, pour vérifier au besoin */
void print_distance_matrix (bool svg)
{
    int i, j;
    char *entete= "";
    char *postfix = "";
    if (svg) {
      entete = "<!-- ";
      postfix = " -->";
    }
    printf ("%snb_towns = %d\n", entete, nb_towns);
    
    for (i = 0; i < nb_towns; i++) {
        printf ("distance [%1d]", i);
        for (j = 0; j < nb_towns; j++) {
            printf (" [%2d:%2d] ", j, tsp_distance[i][j]);
        }
        printf (";\n\n");
    }
    printf ("done ...%s\n",postfix);
}


/* Affichage d'une solution possible. */
void print_solution (tsp_path_t path, int len) {
  fprintf (stderr, "found path len = %3d :", len);
  for (int i = 0; i < nb_towns; i++) {
    fprintf (stderr, "%2d ", path[i]);
  }
  fprintf (stderr, "\n") ;
}

void print_solution_svg (tsp_path_t path, int len) {
  printf ("<!-- found path len = %3d -->\n", len);
  printf ("<svg viewBox='-10 -10 120 120'\n\
 xmlns='http://www.w3.org/2000/svg'>\n\
  <g stroke='black' fill='green'>\n\
    <defs>\n\
      <marker id='Triangle'\n\
              viewBox='0 0 10 10' refX='10' refY='5'\n\
              markerUnits='strokeWidth'\n\
              markerWidth='5' markerHeight='4'\n\
              orient='auto'>\n\
        <path d='M 0 0 L 10 5 L 0 10 z' />\n\
      </marker>\n\
    </defs>\n" );
  for (int i = 0; i < nb_towns; i++) {
    printf ("    <text x='%d' y='%d' font-size='5'>%d</text>\n", 
	    towns[path[i]].x+3, towns[path[i]].y+3, path[i]);
    printf ("    <circle cx='%d' cy='%d' r='3' />\n", towns[path[i]].x, towns[path[i]].y);
    printf ("    <line x1='%d' x2='%d' y1='%d' y2='%d' stroke-width='1' marker-end='url(#Triangle)' />\n",
	    towns[path[(i-1+nb_towns)%nb_towns]].x, towns[path[i]].x,
	    towns[path[(i-1+nb_towns)%nb_towns]].y, towns[path[i]].y);
  }
  printf ("</g>\n\
</svg>\n") ;
  print_distance_matrix(true);
}
