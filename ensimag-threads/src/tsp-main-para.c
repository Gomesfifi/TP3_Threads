#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <complex.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-job.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"
#include "tsp-lp.h"
#include "tsp-hkbound.h"

// Structure des arguments pour le thread //
struct argument_get_job{
    struct tsp_queue *q;
    tsp_path_t *p;
    int *hops;
    int *len;
    uint64_t *vpres;
    tsp_path_t sol;
    int sol_len;
    long long int cuts;
};

#define ALL_IS_OK (void *)(123456789L)

// Prototypes //
void *traitement_tache(void *arg);

/* macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
  ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))


/* tableau des distances */
tsp_distance_matrix_t tsp_distance ={};

/** Param�tres **/

/* nombre de villes */
int nb_towns=10;
/* graine */
long int myseed= 0;
/* nombre de threads */
int nb_threads=1;

/* affichage SVG */
bool affiche_sol= false;
bool affiche_progress=false;
bool quiet=false;

static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len, uint64_t vpres, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int depth)
{
    if (len >= minimum) {
        (*cuts)++ ;
        return;
    }
    
    if (hops == depth) {
        /* On enregistre du travail � faire plus tard... */
      add_job (q, path, hops, len, vpres);
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
	  if (!present (i, hops, path, vpres)) {
                path[hops] = i;
		vpres |= (1<<i);
                int dist = tsp_distance[me][i];
                generate_tsp_jobs (q, hops + 1, len + dist, vpres, path, cuts, sol, sol_len, depth);
		vpres &= (~(1<<i));
            }
        }
    }
}

static void usage(const char *name) {
  fprintf (stderr, "Usage: %s [-s] <ncities> <seed> <nthreads>\n", name);
  exit (-1);
}

int main (int argc, char **argv)
{
    unsigned long long perf;
    tsp_path_t path;
    uint64_t vpres=0;
    tsp_path_t sol;
    int sol_len;
    long long int cuts = 0;
    struct tsp_queue q;
    struct timespec t1, t2;

    /* lire les arguments */
    int opt;
    while ((opt = getopt(argc, argv, "spq")) != -1) {
      switch (opt) {
      case 's':
	affiche_sol = true;
	break;
      case 'p':
	affiche_progress = true;
	break;
      case 'q':
	quiet = true;
	break;
      default:
	usage(argv[0]);
	break;
      }
    }

    if (optind != argc-3)
      usage(argv[0]);

    nb_towns = atoi(argv[optind]);
    myseed = atol(argv[optind+1]);
    nb_threads = atoi(argv[optind+2]);
    assert(nb_towns > 0);
    assert(nb_threads > 0);
   
    minimum = INT_MAX;
      
    /* generer la carte et la matrice de distance */
    if (! quiet)
      fprintf (stderr, "ncities = %3d\n", nb_towns);
    genmap ();

    init_queue (&q);

    clock_gettime (CLOCK_REALTIME, &t1);

    memset (path, -1, MAX_TOWNS * sizeof (int));
    path[0] = 0;
    vpres=1;

    /* mettre les travaux dans la file d'attente */
    generate_tsp_jobs (&q, 1, 0, vpres, path, &cuts, sol, & sol_len, 3);
    no_more_jobs (&q);
   
    /* calculer chacun des travaux */
    tsp_path_t solution;
    memset (solution, -1, MAX_TOWNS * sizeof (int));
    solution[0] = 0;
    // Calcul du nombre de taches à paralléliser = min(nb_threads,nb_tache)
    int nb_threads_used = nb_threads;
    if (q.nbmax < nb_threads_used){
        nb_threads_used = q.nbmax;
    }
    // Calcul du nombre d emise en parallèle nécessaire
    int nb_para = q.nbmax/nb_threads_used + (((q.nbmax%nb_threads_used)==0)?0:1);
    // Création des différents threads
    pthread_t traitement_tache_pid[nb_threads_used];
    int hops = 0, len = 0;
    // Création des arguments pour le traitement d'une tâche
    struct argument_get_job *arguments;
    arguments->q = &q;
    arguments->hops = &hops;
    arguments->len = &len;
    arguments->vpres = &vpres;
    arguments->p = &solution;
    arguments->cuts = 0;
    // Chaque mise en parallèle
    for(int k=0; k<nb_para;k++) {
        for (int i = 0; i < nb_threads_used; i++) {
            pthread_create(&(traitement_tache_pid[i]), NULL, traitement_tache, (void *) arguments);
        }
        // Parallélisation //
        void *status;
        for (int i = 0; i < nb_threads_used; i++) {
            pthread_join(traitement_tache_pid[i], &status);
            if (status == ALL_IS_OK) {
                printf("Thread %lx completed OK.\n", traitement_tache_pid[i]);
            }
        }
    }
    /*while (!empty_queue (&q)) {
        int hops = 0, len = 0;
        get_job (&q, solution, &hops, &len, &vpres);
	
	// le noeud est moins bon que la solution courante
	if (minimum < INT_MAX
	    && (nb_towns - hops) > 10
	    && ( (lower_bound_using_hk(solution, hops, len, vpres)) >= minimum
		 || (lower_bound_using_lp(solution, hops, len, vpres)) >= minimum)
	    )

	  continue;

	tsp (hops, len, vpres, solution, &cuts, sol, &sol_len);
    }*/
    
    clock_gettime (CLOCK_REALTIME, &t2);

    if (affiche_sol)
      print_solution_svg (sol, sol_len);

    perf = TIME_DIFF (t1,t2);
    printf("<!-- # = %d seed = %ld len = %d threads = %d time = %lld.%03lld ms ( %lld coupures ) -->\n",
	   nb_towns, myseed, sol_len, nb_threads,
	   perf/1000000ll, perf%1000000ll, cuts);

    return 0 ;
}

// Implémentation //
void *traitement_tache(void *arg){
    struct argument_get_job* argu = (struct argument_get_job *)arg;
    get_job(argu->q,*(argu->p),argu->hops,argu->len,argu->vpres);
    // Mutex sur le min
    // le noeud est moins bon que la solution courante
    pthread_mutex_t mutex_min;
    pthread_mutex_lock(&mutex_min);
    if (minimum < INT_MAX
        && (nb_towns - *(argu->hops)) > 10
        && ( (lower_bound_using_hk(*(argu->p), *(argu->hops), *(argu->len), *(argu->vpres))) >= minimum
             || (lower_bound_using_lp(*(argu->p), *(argu->hops), *(argu->len), *(argu->vpres))) >= minimum)
            )
        return ALL_IS_OK;
    pthread_mutex_unlock(&mutex_min);

    tsp (*(argu->hops), *(argu->len), *(argu->vpres), *(argu->p), &(argu->cuts), argu->sol, &(argu->sol_len));
    return ALL_IS_OK;
}
