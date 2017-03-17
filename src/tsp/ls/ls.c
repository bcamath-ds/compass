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

static int
call_twoopt_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_twoopt5_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_threeopt_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_linkern (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp);

int compass_tsp_local_search (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ int ret;
  if ( prob->n <= 3) {
    put_err_msg("tsp    : Cannot run local search in an %d node graph\n",
           prob->n);
    return 1;
  }
  switch (tspcp->local_search) {
  case TSP_NO_LS:
    if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("tsp   :  warning no local search.");
  case TSP_TWOOPT_LS:
    if (call_twoopt_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_twoopt_tour failed\n");
      return 1;
    }
    goto done;
  case TSP_TWOOPT5_LS:
    if (call_twoopt5_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_twoopt5_tour failed\n");
      return 1;
    }
    goto done;
  case TSP_THREEOPT_LS:
    if (call_threeopt_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_threeopt_tour failed\n");
      return 1;
    }
    goto done;
  case TSP_LINKERN_LS:
    if (call_linkern (prob, sol, tspcp)) {
      put_err_msg("tsp   :   call_linkern failed\n");
      return 1;
    }
    goto done;
  default:
      put_err_msg("tsp   :   invalid local search flag.\n");
      return 1;
  }
done:
 return 0;
}

static int call_twoopt_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ compass_data *data = prob->data;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { struct tsp_solution *tempsol;
    tempsol = xcalloc(1, sizeof(tsp_solution ));
    if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("tsp   :  2-OPT local search...");
    compass_tsp_init_sol(prob, tempsol);
    tempsol->val = sol->val;
    if (CCkdtree_twoopt_tour (prob->kdtree, prob->n, data, sol->cycle,
          tempsol->cycle, &tempsol->val, 0, 1, prob->rstate_cc))
    { put_err_msg("CCkdtree_twoopt_tour failed\n");
      compass_tsp_delete_sol (tempsol);
      return 1;
    }
    if (sol->val > tempsol->val)
    { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
        xprintf("  length %f\% improved\n", tempsol->val/sol->val);
      compass_tsp_copy_sol(prob, tempsol, sol);
    }
    else
    { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
        xprintf(" no improvement\n");
    }
    compass_tsp_delete_sol (tempsol);
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("No X_NORM 2-opt\n");
  }
  else
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("No JUNK_NORM 2-opt\n");
  }
  return 0;
}

static int call_twoopt5_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ compass_data *data = prob->data;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { struct tsp_solution *tempsol;
    tempsol = xcalloc(1, sizeof(tsp_solution ));
    if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("tsp   :  2.5-OPT local search...");
    compass_tsp_init_sol(prob, tempsol);
    tempsol->val = sol->val;
    if (CCkdtree_twoopt_tour (prob->kdtree, prob->n, data, sol->cycle,
          tempsol->cycle, &tempsol->val, 1, 1, prob->rstate_cc))
    { put_err_msg("CCkdtree_twoopt5_tour failed\n");
      compass_tsp_delete_sol (tempsol);
      return 1 ;
    }
    if (sol->val > tempsol->val)
    { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
        xprintf("  length %f\% improved\n", tempsol->val/sol->val);
      compass_tsp_copy_sol(prob, tempsol, sol);
    }
    compass_tsp_delete_sol (tempsol);
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("No X_NORM 2-opt\n");
  }
  else
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("No JUNK_NORM 2-opt\n");
  }
  return 0;
}


static int call_threeopt_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ compass_data *data = prob->data;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { struct tsp_solution *tempsol;
    tempsol = xcalloc(1, sizeof(tsp_solution ));
    if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("tsp   :  3-OPT local search...");
    compass_tsp_init_sol(prob, tempsol);
    compass_tsp_copy_sol(prob, sol , tempsol);
    tempsol->val = sol->val;
    if (CCkdtree_3opt_tour (prob->kdtree, prob->n, data, sol->cycle,
          tempsol->cycle, &tempsol->val, 1, prob->rstate_cc))
    { put_err_msg("CCkdtree_3opt_tour failed\n");
      compass_tsp_delete_sol (tempsol);
      return 1;
    }
    if (tempsol->val < sol->val)
    { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
        xprintf("  length %.2f (percent) improved\n", tempsol->val/sol->val*100);
      compass_tsp_copy_sol(prob, tempsol, sol);
    }

    compass_tsp_delete_sol (tempsol);
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("No X_NORM 3-opt\n");
  }
  else
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf ("No JUNK_NORM 3-opt\n");
  }
  return 0;
}


static int call_linkern (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ struct tsp_prob *tsp = prob->tsp;
  struct tsp_lkcp *lkcp = tspcp->lkcp;
  struct tsp_solution *tempsol;
  tempsol = xmalloc(sizeof(tsp_solution ));
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
     xprintf ("tsp   :  Lin-Kernighan local search...");
  compass_tsp_init_sol(prob, tempsol);
  tempsol->val = sol->val;
  //lkcp->nkicks = prob->n;
  if (CClinkern_tour (prob->n, prob->data, tsp->ecount, tsp->elist,
        100000000, lkcp->nkicks, sol->cycle, tempsol->cycle, &tempsol->val, 1, -1.0, -1.0,
       (char *) NULL, lkcp->kick_type, prob->rstate_cc))
  { put_err_msg("CClinkern_tour failed\n");
    compass_tsp_delete_sol (tempsol);
    return 1;
  }
  if (sol->val > tempsol->val)
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf("  length %f\% improved\n", tempsol->val/sol->val);
    compass_tsp_copy_sol(prob, tempsol, sol);
    tsp->sol_stat = COMPASS_FEAS;
  }
  compass_tsp_delete_sol (tempsol);
  return 0;
}
