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

#include "compass.h"
#include "env.h"
#include "tsp.h"

void compass_tsp_solve (compass_prob *prob, struct tsp_cp *tspcp)
{ int ret;
  struct tsp_prob *tsp = prob->tsp;
  /* Initial solution */
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf("\n");
    xprintf("tsp  :   - Building initial tours.\n");
  }
  compass_tsp_start_tour (prob, tsp->sol, tspcp );
  /* solve heuristically */
  if ( tspcp->local_search == TSP_NO_LS)
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf("tsp  :  - Skipping local seach...\n");
  }
  else
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    { xprintf("\n");
      xprintf("tsp  :  - Starting local seach...\n");
    }
    compass_tsp_local_search (prob, prob->tsp->sol, tspcp );
  }
  return;
}

int main_tsp (struct csa *csa, int ac, char **av)
{ int ret;
  compass_prob *prob = csa->prob;
  struct tsp_prob *tsp = prob->tsp;
  struct tsp_cp *tspcp =csa->tspcp;
  /* perform initialization */
#if 0
  /*--------------------------------------------------------------------------*/
  /* parse command-line parameters */
  ret = parse_cmdline(csa, argc, argv);
  if (ret < 0)
  { ret = EXIT_SUCCESS;
    goto done;
  }
  if (ret > 0)
  { ret = EXIT_FAILURE;
    goto done;
  }
#endif
  /*--------------------------------------------------------------------------*/
  /* solve the problem */
  tspcp->tm_start = xtime();
  compass_tsp_solve( prob, tspcp);
  /*--------------------------------------------------------------------------*/
  /* display statistics */

  if (tspcp->msg_lev >= COMPASS_MSG_ON)
  { if ( tsp->sol_stat == COMPASS_OPT | tsp->sol_stat == COMPASS_FEAS )
    { xprintf ("\n");
      xprintf ("tsp   : Best solution: %.0f\n", tsp->sol->val);
    }
    else if ( tsp->sol_stat == COMPASS_NOFEAS )
    { xprintf("\n");
      xprintf ("tsp   : Error! No feasible solution found\n");
      ret = COMPASS_NOFEAS;
    }
    else
    { xprintf("\n");
      xprintf ("tsp   : Error! Problem not solved\n");
      ret = COMPASS_UNDEF;
    }
  }
  /*--------------------------------------------------------------------------*/
  /* all seems to be ok */
  ret = EXIT_SUCCESS;
  /*--------------------------------------------------------------------------*/
done:
  return ret;
}

static struct tsp_lkcp *tsp_init_linkern_cp( void);

struct tsp_cp *compass_tsp_init_cp(void)
{ struct tsp_cp *tspcp;
  tspcp = xmalloc(sizeof(struct tsp_cp));
  tspcp->msg_lev = COMPASS_MSG_OFF;
  tspcp->tm_start = xtime();
  tspcp->tm_lim = INT_MAX;
  tspcp->exact = 0;
  tspcp->nruns = 1;
  tspcp->start_tour = TSP_RANDOM_TOUR;
  tspcp->local_search = TSP_LINKERN_LS;
  tspcp->neighcp = compass_neigh_init_cp();
  tspcp->lkcp = tsp_init_linkern_cp();
  return tspcp;
}

void compass_tsp_delete_cp(struct tsp_cp* tspcp)
{ xfree(tspcp->neighcp);
  xfree(tspcp->lkcp);
  xfree(tspcp);
}


static struct tsp_lkcp *tsp_init_linkern_cp(void)
{ struct tsp_lkcp *lkcp;
  lkcp = xmalloc(sizeof(struct tsp_lkcp));
  lkcp->tm_start = xtime();
  lkcp->tm_lim = INT_MAX;
  lkcp->nruns = 1;
  lkcp->kick_type = CC_LK_WALK_KICK;
  lkcp->nkicks = 0;
  return lkcp;
}
