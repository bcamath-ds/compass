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
#include "rng.h"
#include "env.h"
#include "tsp.h"
#include "op.h"

static void
op_improve_lenght_pop ( compass_prob *prob, op_population *pop,
    struct tsp_cp *tspcp),
op_check_feasibility_pop ( compass_prob *prob, op_population *pop,
    struct op_cp *opcp);

/***********************************************************************
*  NAME
*
*  compass_op_ea - solve OP problem with a Evolutionary Algorithm
*
*  SYNOPSIS
*
*  int compass_op_ea(compass_prob *P, const op_pcp *eacp);
*
*  DESCRIPTION
*
*  The routine solve_op_ea is a driver to the EA solver.
*  This routine retrieves problem data from the
*  specified problem object, calls the solver to solve the problem
*  instance, and stores results of computations back into the problem
*  object.
*
*  The Evolutionary Algorithm has a set of control parameters. Values of the
*  control parameters can be passed in a structure op_eacp, which the
*  parameter parm points to.
*
*  The parameter parm can be specified as NULL, in which case the EA
*  solver uses default settings.
*
*  RETURNS
*
*  0  The LP problem instance has been successfully solved. This code
*     does not necessarily mean that the solver has found optimal
*     solution. It only means that the solution process was successful.
*
*  COMPASS_EFAIL
*     The search was prematurely terminated due to the solver failure.
*
*  ???COMPASS_EITLIM
*     The search was prematurely terminated, because the
*     iteration limit has been exceeded.
*
*  ????GLP_ETMLIM
*     The search was prematurely terminated, because the time limit has
*     been exceeded. */

int compass_op_solve_ea (compass_prob *prob, op_population *pop, struct op_cp *opcp)
{ int ret = 0;
  int i, worst_ind;
  double time_elapsed;
  struct op_prob *op = prob->op;
  struct op_eacp *eacp = opcp->eacp;
  int *parent = pop->parent;
  op_solution *child;
  op_solution *best_sol ;
  parent = xcalloc( eacp->nparsel, sizeof(int));
  child = xcalloc(1, sizeof(op_solution));
  compass_op_init_sol(prob, child);
  eacp->tm_start = xtime();
  //compass_op_init_sol(prob, best_sol);
/*----------------------------------------------------------------------------*/
  if (prob->n < 4)
  { xprintf ("Less than 4 node problem\n");
    goto done;
  }
/*----------------------------------------------------------------------------*/
/* Main Loop */
  for (eacp->it=1; eacp->it< eacp->it_lim +1;eacp->it++)
  { if (eacp->it   % eacp->d2d != 0)
    { compass_op_choose_sol (prob, op->population, eacp->nparsel, parent, opcp);
      compass_op_crossover (prob, op->population, child, parent, eacp);
      if (rng_unif_01(prob->rstate) < eacp->pmut)
        compass_op_mutate_sol( prob, child, eacp);
      if ( pop->worst_val < child->val)
      { compass_op_set_pop_sol (prob, op->population, child,
          op->population->worst_ind);
        compass_op_update_pop ( op->population);
      }
      compass_op_erase_sol(prob, child);
    }
    else
    {
      if ( eacp->len_improve1)
        op_improve_lenght_pop (prob, op->population, opcp->tspcp);
      op_check_feasibility_pop(prob, op->population, opcp);
      if ( eacp->len_improve2)
        op_improve_lenght_pop (prob, op->population, opcp->tspcp);
      compass_op_update_pop(op->population);
      best_sol = &op->population->solution[op->population->best_ind];
      compass_op_copy_sol(prob, best_sol, op->sol);
      if (eacp->msg_lev >= COMPASS_MSG_ON)
        xprintf("op   | EA :  %d it : best %.0f : worst %.0f (%.2f sec) \n",
            eacp->it ,op->population->best_val, op->population->worst_val,
            xdifftime(xtime(),eacp->tm_start));

      if (opcp->stop_pop)
        if (op->population->best_val == op->population->stop_val)
          break;
      if (xdifftime(xtime(),opcp->tm_start) > opcp->tm_lim ||
          xdifftime(xtime(),eacp->tm_start) > eacp->tm_lim )
          break;
    }
  }
/*----------------------------------------------------------------------------*/
cleanup:
  xfree(parent);
  compass_op_delete_sol(child);
done:
  eacp->tm_end = xtime();
  compass_op_copy_sol(prob, op->sol, eacp->best);
  op->sol_stat = COMPASS_FEAS;

  return ret;
}


/**********************************************************************/
static void op_improve_lenght_pop ( compass_prob *prob, op_population *pop,
    struct tsp_cp *tspcp)
/**********************************************************************/
{ int i;
  compass_prob *tspprob;
  tspprob = compass_init_prob();

  for (i = 0; i < pop->size; i++)
  { op_solution *opsol = &pop->solution[i];
    compass_sub_prob ( prob, tspprob, opsol->selected);
    compass_tsp_init_prob(tspprob);
    tsp_solution *tspsol = tspprob->tsp->sol;
    //compass_tsp_init_sol(tspprob, tspsol);

    compass_convert_sol_op2tsp(prob, opsol, tspsol );

    compass_data_k_nearest (tspprob, tspcp->neighcp );
    if ( tspcp->local_search == TSP_NO_LS)
    { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
        xprintf("tsp  :  - Skipping local seach...\n");
    }
    else
    { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      { xprintf("\n");
      xprintf("tsp  :  - Starting local seach...\n");
      }
      compass_tsp_local_search(tspprob, tspsol, tspcp);
    }

    if (tspsol->val < opsol->length )
    { if (tspcp->msg_lev >= COMPASS_MSG_ON)
        xprintf("op    :  Tour length improved.\n");
      compass_convert_sol_tsp2op(prob, tspsol, opsol, opsol->selected);
    }
    if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    {
      xprintf (" imp %d: nv: %d, length %.0f, fitness %.0f\n",
              i, opsol->ns, opsol->length, opsol->val);
    }
    compass_tsp_delete_prob(tspprob);
    compass_erase_prob(tspprob);
  }
  compass_delete_prob(tspprob);
  return;
}


/**********************************************************************/
static void op_check_feasibility_pop ( compass_prob *prob, op_population *pop,
    struct op_cp *opcp)
/**********************************************************************/
{ int i;

  for (i = 0; i < pop->size; i++)
  { op_solution *opsol = &pop->solution[i];
    compass_op_fit_solution(prob, opsol, opcp);
  }
  return;
}

/***********************************************************************
*  NAME
*
*  compass_init_op_eacp - initialize genetic algorithm control parameters
*
*  SYNOPSIS
*
*  void compass_init_op_eacp(op_eacp *parm);
*
*  DESCRIPTION
*
*  The routine compass_init_op_eacp initializes control parameters, which are
*  used by the genetic algorithm, with default values.
*
*  Default values of the control parameters are stored in a op_eacp
*  structure, which the parameter parm points to. */

void compass_op_init_eacp(struct op_eacp *eacp)
{ eacp->msg_lev = COMPASS_MSG_ON;
  eacp->tm_start = xtime();
  eacp->tm_lim = 18000.;
  eacp->it_lim = INT_MAX;
  eacp->it = 0;
  eacp->pop_size = 100;
  eacp->d2d = 50;
  eacp->nparsel = 10;
  eacp->pmut = 0.01;
  eacp->len_improve1 = 1;
  eacp->len_improve2 = 0;
  eacp->best = xcalloc(1, sizeof(op_solution));
  return;
}
