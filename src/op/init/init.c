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
#include "op.h"

/*****************************************************************************/
int compass_op_start_cycle ( compass_prob *opprob, struct op_solution *opsol,
    int *selected, struct tsp_cp *tspcp)
/*****************************************************************************/
{ int ret = 0;
  compass_prob *tspprob;
  tspprob = compass_init_prob();
  compass_sub_prob ( opprob, tspprob, selected);
  compass_tsp_init_prob(tspprob);
  compass_data_k_nearest (tspprob, tspcp->neighcp );
  if (tspprob->kdtree == (CCkdtree *) NULL)
    xprintf("kdtree_start null\n");
  tsp_solution *tspsol = tspprob->tsp->sol;
  compass_convert_sol_op2tsp(opprob, opsol, tspsol );
  compass_tsp_solve(tspprob, tspcp);
  compass_convert_sol_tsp2op(opprob, tspsol, opsol, selected);
  compass_tsp_delete_prob(tspprob);
  compass_delete_prob(tspprob);
  return ret;
}

/*****************************************************************************/
int compass_op_start_solution ( compass_prob *prob, op_solution *sol,
    struct op_cp *opcp)
/*****************************************************************************/
{ int ret = 0;

  struct op_initcp *initcp = opcp->initcp;

  if (initcp->init_tech == OP_INIT_BEST3)
  { compass_op_select_best3nodes(prob, sol, opcp);
  }
  else if (initcp->init_tech == OP_INIT_RAND)
  { compass_op_select_nodes(prob, sol, opcp);
    compass_op_start_cycle(prob, sol, sol->selected, opcp->tspcp);
  }
  else
    ret =1;

  compass_op_fit_solution(prob, sol, opcp);

  return ret;
}

/*****************************************************************************/
int compass_op_start_population ( compass_prob *prob, op_population *pop,
    struct op_cp *opcp)
/*****************************************************************************/
{ int ret = 0;
  int i;
  xprintf ("op   : > Population size: %d\n", pop->size);
  for (i = 0; i < pop->size; i++)
  { op_solution *sol = &pop->solution[i];
    compass_op_erase_sol(prob, sol);
    compass_op_start_solution(prob, sol, opcp);
    if (opcp->msg_lev >= COMPASS_MSG_ALL)
    { xprintf(" %d: nvis: %d, length %.0f, value %.0f\n",
          i, sol->ns, sol->length, sol->val);
    }
    if (xdifftime(xtime(), opcp->tm_start) > opcp->tm_lim )
    { pop->size = opcp->pop_size = i+1;
      break;
    }
  }
  compass_op_update_pop(pop);

done:
  return ret;
}

void compass_op_init_initcp(struct op_initcp *initcp)
{ initcp->msg_lev = COMPASS_MSG_ON;
  initcp->tm_start = xtime();
  initcp->tm_lim = 18000.;
  initcp->pop_size = 100;
  initcp->pgreedy = 0.;
  initcp->pinit = 0;
  initcp->init_tech = OP_INIT_RAND;
  initcp->sel_tech = OP_SEL_BERNOULLI;
  initcp->best = xcalloc(1, sizeof(op_solution));
  return;
}
