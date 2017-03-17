/***********************************************************************
*  This code is part of Compass.
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

#ifndef TSP_H
#define TSP_H
#include "env.h"
#include "data/neigh.h"

typedef struct tsp_solution tsp_solution;

struct tsp_solution
{ double      val;
  int         *cycle;
};

struct tsp_prob
{ /* TSP problem object */
  unsigned      magic;
  char *obj;
  /* objective function name (1 to 255 chars); NULL means no name
         is assigned to the objective function */
  /* magic value used for debugging */
  //glp_prob      *lp;
  /* LP/MIP problem object */
  int           n;
  /* number of nodes */
  int           ecount;
  int           *elist;
  /* Edge temp list */
  int           sol_stat;
  /* integer solution status:
    COMPASS_UNDEF  - integer solution is undefined
    COMPASS_OPT    - integer solution is optimal
    COMPASS_FEAS   - integer solution is feasible
    COMPASS_NOFEAS - no integer solution exists */
  tsp_solution  *sol;
};

struct tsp_lkcp
{     /*Linker control parameters */
  int tm_start;             /* starting timestamp */
  int tm_end;               /* ending timestamp */
  int tm_lim;               /* time limit (milliseconds) */
  int nruns;                /* Number of repetitions */
  int kick_type;
#define CC_LK_RANDOM_KICK    (0)
#define CC_LK_GEOMETRIC_KICK (1)
#define CC_LK_CLOSE_KICK     (2)
#define CC_LK_WALK_KICK      (3)
  int nkicks;
};

struct tsp_cp
{     /*TSP specific control parameters */
  int msg_lev;              /* message level: */
#define COMPASS_MSG_OFF   0  /* no output */
#define COMPASS_MSG_ERR   1  /* warning and error messages only */
#define COMPASS_MSG_ON    2  /* normal output */
#define COMPASS_MSG_ALL   3  /* full output */
#define COMPASS_MSG_DBG   4  /* debug output */
  double tm_start;             /* starting timestamp */
  double tm_end;               /* ending timestamp */
  double tm_lim;               /* time limit (milliseconds) */

  struct neigh_cp *neighcp;

  int start_tour;
#define TSP_RANDOM_TOUR   0
#define TSP_NEIGHBOR_TOUR 1
#define TSP_GREEDY_TOUR   2
#define TSP_BORUVKA_TOUR  3
#define TSP_QBORUVKA_TOUR 4

  int local_search;
#define TSP_NO_LS         0
#define TSP_TWOOPT_LS     1
#define TSP_TWOOPT5_LS    2
#define TSP_THREEOPT_LS   3
#define TSP_LINKERN_LS    4
  int nruns;                /* Number of repetitions */
  int exact;                /* Find exact solution */
  struct tsp_lkcp *lkcp;
};

#endif
