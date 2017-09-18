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
  compass_op_init_sol(prob, opcp->initcp->best);
  compass_op_init_sol(prob, opcp->eacp->best);
  /* Initial solution */
  opcp->tm_start = xtime();
  compass_op_node_ranking(prob, opcp);
  if ( opcp->heur_tech == OP_HEUR_EA)
  { if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("\n");
      xprintf("op   : Building initial tours.\n");
    }
    opcp->initcp->tm_start = xtime();
    compass_op_init_pop (prob, op->population, opcp->pop_size );
    op->population->stop_per = opcp->stop_pop;
    compass_op_start_population (prob, op->population, opcp);
    best_sol = &op->population->solution[op->population->best_ind];
    compass_op_copy_sol(prob, best_sol, opcp->initcp->best);
    opcp->initcp->tm_end = xtime();
    if (opcp->msg_lev >= COMPASS_MSG_ON)
    { xprintf("op   : Best %.0f , Worst %.0f\n",
        op->population->best_val, op->population->worst_val);
      xprintf("op   : Time: %.2f sec \n", xdifftime(xtime(),opcp->tm_start));
    }
    compass_op_copy_sol(prob, best_sol, op->sol);
  }
  else
    xassert(prob != prob);
  /* solve heuristically */
  if ( opcp->heur_tech == OP_HEUR_NONE )
  {// xprintf("\n");
  }
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

  opcp->tm_end = xtime();

  if (opcp->stats_file)
  { opcp->tm_end = xtime();
    if (compass_write_op_stats ( prob, opcp, opcp->stats_file ))
    { fprintf (stderr, "could not write the results\n");
      ret = 1; goto done;
    }
  }
  /*--------------------------------------------------------------------------*/
  /* all seems to be ok */
  ret = EXIT_SUCCESS;
  /*--------------------------------------------------------------------------*/
done:
  if (opcp->exact==0 & op->population != NULL)
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

      /* Write solution */
      if (csa->out_sol)
      { if (compass_write_op_sol ( prob, csa->out_sol ))
        { fprintf (stderr, "could not write the solution\n");
          ret = 1; goto done;
        }
      }

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

void compass_op_init_cp(struct op_cp* opcp)
{ opcp->msg_lev = COMPASS_MSG_ON;
  opcp->tm_start = xtime();
  opcp->tm_lim = 18000.;
  opcp->pp_tech = OP_PP_NONE;
  opcp->pop_size = 100;
  opcp->stop_pop = 25;
  opcp->heur_tech = OP_HEUR_NONE;
  opcp->exact = 0;
  opcp->nruns = 1;
  opcp->add = OP_ADD_D;
  opcp->drop = OP_DROP_SD;
  opcp->tspcp = xmalloc(sizeof(struct tsp_cp));
  compass_tsp_init_cp(opcp->tspcp);
  opcp->initcp = xmalloc(sizeof(struct op_initcp));
  compass_op_init_initcp( opcp->initcp);
  opcp->eacp = xmalloc(sizeof(struct op_eacp));
  compass_op_init_eacp( opcp->eacp);
  return;
}

void compass_op_delete_cp(struct op_cp* opcp)
{ tfree(opcp->eacp);
  compass_tsp_delete_cp(opcp->tspcp);
  xfree(opcp);
}
