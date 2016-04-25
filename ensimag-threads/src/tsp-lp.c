#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "tsp-types.h"
#include "tsp-tsp.h"
#include "tsp-lp.h"
#include "lpsolveurs.h"

/* sauvergarde dans un fichier d'un programme linéaire (Format LP)
   calculant une borne inférieur sur la longueur du TSP de la solution
   partielle */
void save_lp(FILE* f, tsp_path_t path, int hops, int len, uint64_t vpres) {
  bool (*x)[nb_towns];
  const int taillex = sizeof(bool)* nb_towns * nb_towns;
  x = (bool (*)[nb_towns]) malloc(taillex);
  memset(x, 0, taillex);
  
  /* calcul de présence potentielle de chaque lien dans la solution */
  for (int i = 0; i < nb_towns; i++) {
    if (!present (i, hops, path, vpres)) {
      x[0][i] = true;
      if (i > path[hops-1])
	x[path[hops-1]][i] = true;
      else
	x[i][path[hops-1]] = true;
      for(int j = i+1; j < nb_towns; j++) {
	if (!present (j, hops, path, vpres)) {
	  x[i][j] = true;
	}
      }
    }
  }

  /* tous les liens potentiellement présents compte dans la minimisation */
  fprintf(f, "minimize L: ");
  bool plus = false;
  for (int i = 0; i < nb_towns; i++) {
    for(int j = 0; j < nb_towns; j++) {
	if (i != j && x[i][j]) {
	  if (plus)
	    fprintf(f," +");
	  fprintf(f, " %d x%d_%d",tsp_distance[i][j], i, j);
	  plus = true;
      }
    }
  }

  fprintf(f,"\nsubject to\n");

  for (int i = 0; i < nb_towns; i++) {
    if (!present (i, hops, path, vpres) || i == 0 || i == path[hops-1]) {
      fprintf(f, "ville%d: ", i);
      bool plus = false;
      for(int j = 0; j < nb_towns; j++) {
	if (i != j && x[i][j]) {
	  if (plus)
	    fprintf(f," +");
	  fprintf(f, " x%d_%d", i, j);
	  plus = true;
	}
	else if (i != j && x[j][i]) {
	  if (plus)
	    fprintf(f," +");
	  fprintf(f, " x%d_%d", j, i);
	  plus = true;
	}

      }
      if (i == 0 || i == path[hops-1])
	fprintf(f, " = 1\n");
      else
	fprintf(f, " = 2\n");	
    }
  }

  fprintf(f, "\nbinary\n");
  for (int i = 0; i < nb_towns; i++) {
    for(int j = 0; j < nb_towns; j++) {
	if (i != j && x[i][j]) {
	  fprintf(f, "x%d_%d\n",i, j);
	  plus = true;
      }
    }
  }
  fprintf(f,"\nend\n");

  return;
};

/* Calcul d'une borne inférieure en sauvegardant dans un fichier et en
   lancant un solveur linéaire (cbc) sur le fichier et en interprétant
   son résultat */ 

int lower_bound_using_lp(tsp_path_t path, int hops, int len, uint64_t vpres) {

  if (SOLVEUR == SOLVEUR_NONE) {
    return 0;
  } else {
    FILE *f = fopen("toto.lp","w");
    save_lp(f, path, hops, len, vpres);
    fclose(f);

    double val=0.0;
    FILE *sol=0;

    switch(SOLVEUR) {
    case SOLVEUR_CBC:
      sol = popen("cbc toto.lp | tee toto.sol | grep 'Objective value'","r");
      fscanf(sol, "Objective value: %lg",& val);
      fclose(sol);
      break;
    case SOLVEUR_SYMPHONY:
      sol = popen("symphony -L toto.lp | tee toto.sol | grep \"Solution Cost: \"","r");
      fscanf(sol, "Solution Cost: %lg",& val);
      fclose(sol);
      break;
    case SOLVEUR_GLPSOL:
      sol = popen("glpsol --lp toto.lp -o /dev/stdout | tee toto.sol | grep 'Objective:  L = '","r");
      fscanf(sol, "Objective:  L =  %lg",& val);
      fclose(sol);
      break;
    case SOLVEUR_NONE:
      return 0;
      break;
    }
    int longueur = ((int) val) + len;
    return longueur;
  }
  return 0;
}
