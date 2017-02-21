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
#include "op.h"
#include <gsl/gsl_rng.h>


int compass_op_solve (compass_prob *prob, struct op_cp *opcp)
{ int ret;
  struct op_prob *op = prob->op;
  op_solution *best_sol;
  /* Initial solution */
  compass_op_node_ranking(prob, opcp);
  if ( opcp->heur_tech == OP_HEUR_EA)
  { if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("\n");
      xprintf("op   : Building initial tours.\n");
    }
    compass_op_init_pop (prob, op->population, opcp->pop_size );
    op->population->stop_per = opcp->stop_pop;
    compass_op_start_population (prob, op->population, opcp);
    best_sol = &op->population->solution[op->population->best_ind];
    compass_op_copy_sol(prob, best_sol, op->sol);
    if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("op   : Best %.0f , Worst %.0f\n",
        op->population->best_val, op->population->worst_val);
      xprintf("op   : Time: %.2f sec \n", xdifftime(xtime(),opcp->tm_start));
    }
  }
#if 0
  else if ( opcp->heur_tech == OP_HEUR_2PIA)
  { if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("\n");
      xprintf("op   : Building initial tours.\n");
    }
  }
#endif
  else
    xassert(prob != prob);
  /* solve heuristically */
  if ( opcp->heur_tech == OP_HEUR_NONE)
    xprintf("No heuristic technique specified.\n");
  else if ( opcp->heur_tech == OP_HEUR_EA)
  { if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("\n");
      xprintf("op   : Starting the Evolutionary Algorithm...\n");
    }
    compass_op_solve_ea (prob, op->population, opcp);
  }
#if 0
  else if ( opcp->heur_tech == OP_HEUR_2PIA)
  { if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("\n");
      xprintf("op   : Starting the iteractive algorithm...\n");
    }
    compass_op_solve_2pia (prob, opcp);
  }
#endif
  else
    xassert(prob != prob);
  /* solve exactly */
  //if (csa->exact)
    //compass_op_solve_exact(csa->prob, &csa->excp);
  /*--------------------------------------------------------------------------*/
  /* all seems to be ok */
  ret = EXIT_SUCCESS;
  /*--------------------------------------------------------------------------*/
done:
  //if (op->population != NULL)
  compass_op_delete_pop(op->population);
  return ret;
}

int main_op (struct csa *csa, int ac, char **av)
{ int ret;
  compass_prob *prob = csa->prob;
  struct op_prob *op = prob->op;
  struct op_cp *opcp = csa->opcp;
  opcp->stats_file = csa->stats_file;
  //compass_op_init_prob(prob);
  /*--------------------------------------------------------------------------*/
  /* solve the problem */
  opcp->tm_start = xtime();
  compass_op_solve( prob, opcp);
  /*--------------------------------------------------------------------------*/
  /* display statistics */
  if (opcp->msg_lev >= COMPASS_MSG_ON)
  { if (op->sol_stat == COMPASS_OPT || op->sol_stat == COMPASS_FEAS )
    { xprintf ("\n");
      xprintf ("op   : Best solution value: %.0f\n", op->sol->val);
      xprintf ("op   : Visited: %d\n", op->sol->ns);
    }
    else if ( prob->op->sol_stat == COMPASS_NOFEAS )
    { xprintf("\n");
      xprintf ("op    : Error! No feasible solution found\n");
      ret = COMPASS_NOFEAS;
    }
    else
    { xprintf("\n");
      xprintf ("op    : Error! Problem not solved\n");
      ret = COMPASS_UNDEF;
    }
  }
  /*--------------------------------------------------------------------------*/
  /* all seems to be ok */
  ret = EXIT_SUCCESS;
  /*--------------------------------------------------------------------------*/
done: 
  /* close log file, if necessary */
  //if (csa->log_file != NULL) glp_close_tee();
  /* return to the control program */
  return ret;
}

struct op_cp *compass_op_init_cp(void)
{ struct op_cp *opcp;
  opcp = xmalloc(sizeof(struct op_cp));
  opcp->msg_lev = COMPASS_MSG_ON;
  opcp->tm_start = xtime();
  opcp->tm_lim = INT_MAX;
  opcp->pp_tech = OP_PP_NONE;
  opcp->pop_size = 100;
  opcp->stop_pop = 25;
  opcp->pgreedy = 0.;
  opcp->pinit = 0.5;
  opcp->sel_tech = OP_SEL_BERNOULLI;
  opcp->heur_tech = OP_HEUR_EA;
  opcp->exact = 0;
  opcp->nruns = 1;
  opcp->add = OP_ADD_D;
  opcp->drop = OP_DROP_SD;
  opcp->tspcp = compass_tsp_init_cp();
  opcp->eacp = xmalloc(sizeof(struct op_eacp));
  compass_op_init_eacp( opcp->eacp);
  return opcp;
}

void compass_op_delete_cp(struct op_cp* opcp)
{ tfree(opcp->eacp);
  compass_tsp_delete_cp(opcp->tspcp);
  xfree(opcp);
}
