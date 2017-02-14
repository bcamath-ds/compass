/***********************************************************************
*  This code is part of Compass.
*
*  Copyright (C) 2000-2013 Andrew Makhorin, Department for Applied
*  Informatics, Moscow Aviation Institute, Moscow, Russia. All rights
*  reserved. E-mail: <gkobeaga@bcamath.org>.
*
*  Compass is free software: you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Compass is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
*  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
*  License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Compass. If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#ifndef OP_H
#define OP_H
#include "env.h"
#include "tsp/tsp.h"
#include "op/ea/ea.h"

typedef struct op_solution op_solution;
typedef struct op_population op_population;

#define COMPASS_ON 1

struct op_solution
{ int         tot_n;
  int         *genotype;
  int         *cycle;
  int         *selected;
  int         *sposition;
  double         val;
  double      length;
  int         ns;
  int         greedycount;
  int         *greedylist;
};

struct op_population
{ int         size;
  int         stop_per;
  struct op_solution *solution;
  int         *rankperm;
  double      mean_val;
  double      best_val;
  int         best_ind;
  double      q25_val;
  int         q25_ind;
  double      q50_val;
  int         q50_ind;
  double      q75_val;
  int         q75_ind;
  double      stop_val;
  int         stop_ind;
  double      worst_val;
  int         worst_ind;
  int         *parent;
};

struct op_prob
{ /* OP problem object */
  unsigned    magic;
  char *obj;
  /* objective function name (1 to 255 chars); NULL means no name
         is assigned to the objective function */
  /* magic value used for debugging */
  //glp_prob    *lp;
  /* LP/MIP problem object */
  int         nne;
  /* number of non-excluded (d/2<d0) nodes in the preproccess , nne >= 0 */
  //int ecount;
  double      *s;
  /* Node scores object */
  double      d0;
  /* OP dist-limit */
  int      from;
  /* Departure node */
  int      to;
  /* Arraiving node */
  op_population *population;
  int         ecount;
  int         *elist;
  int         *noderank;
  /* Edge temp list */
  int         sol_stat;
  /* integer solution status:
    COMPASS_UNDEF  - integer solution is undefined
    COMPASS_OPT    - integer solution is optimal
    COMPASS_FEAS   - integer solution is feasible
    COMPASS_NOFEAS - no integer solution exists */
  op_solution *sol;
};

struct op_cp
{     /* Orienteering problem specific control parameters */
  int msg_lev;              /* message level: */
#define COMPASS_MSG_OFF       0  /* no output */
#define COMPASS_MSG_ERR       1  /* warning and error messages only */
#define COMPASS_MSG_ON        2  /* normal output */
#define COMPASS_MSG_ALL       3  /* full output */
#define COMPASS_MSG_DBG       4  /* debug output */
  double tm_start;              /* starting timestamp */
  double tm_end;                /* ending timestamp */
  double tm_lim;               /* time limit (milliseconds) */
  int pop_size;
  int stop_pop;
  int pp_tech;              /* preprocessing technique: */
#define OP_PP_NONE       0    /* disable preprocessing */
#define OP_PP_ROOT       1    /* preprocessing only on root level */
#define OP_PP_ALL        2    /* preprocessing on all levels */
  int sel_tech;
#define OP_SEL_BERNOULLI 0    /* select using Bernoully */
  double pgreedy;                /* Greediness parameter */
  double pinit;                /* Bernoully p for initial population */
  int heur_tech;            /* heuristic technique: */
#define OP_HEUR_NONE     0    /* disable heuristic */
#define OP_HEUR_EA       1    /* Genetic Algorithm */
#define OP_HEUR_2PIA     2    /* 2 Parameter Iteractive Algorithm */
  int add;
#define OP_ADD_D   0      /* Sort nodes depending only the distance cost*/
#define OP_ADD_SD  1      /* Sort nodes depending distance cost and score increase*/
#define OP_ADD_S   3      /* Sort nodes depending only the score increase*/
  int drop;
#define OP_DROP_D   0      /* Sort nodes depending only the distance cost*/
#define OP_DROP_SD  1      /* Sort nodes depending the distance cost and score decrease*/
#define OP_DROP_S   3      /* Sort nodes depending only the score decrease*/
  int exact;                /* Find exact solution */
  int nruns;                /* Number of repetitions */
  struct tsp_cp *tspcp;             /* TSP control parameters */
  struct op_eacp *eacp;             /* EA control parameters */
  struct op_iacp *iacp;             /* Iteractive Algorithm control parameters */
  const char *stats_file;
};

#endif


#if 0
typedef struct op_data_user {
    double  *x;
    double  *y;
} op_data_user;

typedef struct op_data_rhvector {
    int dist_00;
    int dist_01;
    int dist_02;
    int dist_12;
    int dist_22;
    double p;
    int rhlength;
    char *space;
    char **vectors;
} op_data_rhvector;
#endif

