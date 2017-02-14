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

#include "compass.h"
#include "env.h"
#include "tsp.h"
#include "op.h"

static void op_init_sol(int n, op_solution *sol)
{ int i;
  sol->tot_n       = n;
  sol->genotype    = xcalloc(n, sizeof(int));
  sol->selected    = xcalloc(n, sizeof(int));
  sol->sposition   = xcalloc(n, sizeof(int));
  sol->cycle       = xcalloc(n, sizeof(int));
  sol->greedylist  = xcalloc(n, sizeof(int));
  sol->greedycount = n;
  sol->val=0.0;
  sol->length      = 1e30;
  sol->ns=0;
  for (i=0; i<n; i++)
  { sol->selected[i]   =  0;
    sol->genotype[i]   =  i;
    sol->cycle[i]      = -1;
    sol->sposition[i]  =  n;
    sol->greedylist[i] =  1;
  }
}

void compass_op_init_sol(compass_prob *prob, op_solution *sol)
{ op_init_sol(prob->n, sol);
  return;
}

static void op_delete_sol(op_solution *sol)
{ xfree(sol->genotype);
  xfree(sol->selected);
  xfree(sol->sposition);
  xfree(sol->cycle);
  xfree(sol->greedylist);
}

void compass_op_erase_sol(compass_prob *prob, op_solution *sol)
{ op_delete_sol(sol);
  op_init_sol(prob->n, sol);
  return;
}

void compass_op_delete_sol(struct op_solution *sol)
{ op_delete_sol(sol);
  xfree(sol);
  return;
}

void compass_op_copy_sol(compass_prob *prob, op_solution *insol,
    op_solution *outsol)
{ int i;
  op_delete_sol(outsol);
  op_init_sol(prob->n, outsol);
  for (i=0; i<prob->n; i++)
  { outsol->genotype[i]   = insol->genotype[i];
    outsol->selected[i]   = insol->selected[i];
    outsol->sposition[i]  = insol->sposition[i];
    outsol->cycle[i]      = insol->cycle[i];
    outsol->greedylist[i] = insol->greedylist[i];
  }
  outsol->val         = insol->val;
  outsol->length      = insol->length;
  outsol->ns          = insol->ns;
  outsol->greedycount = insol->greedycount;
  return;
}



static void op_init_pop( op_population *pop, int size)
{ int i;
  pop->size      = size;
  pop->solution  = talloc(size, struct op_solution );
  pop->rankperm  = xcalloc(size, sizeof(int));
  for(i=0; i<size; i++)
    pop->rankperm[i] = i;
  pop->mean_val  = 0.0;
  pop->best_val  = 0.0;
  pop->best_ind  = 0;
  pop->q25_ind   = 0;
  pop->q25_val   = 0.0;
  pop->q50_ind   = 0;
  pop->q50_val   = 0.0;
  pop->q75_ind   = 0;
  pop->q75_val   = 0.0;
  pop->stop_val  = 0.0;
  pop->stop_ind  = 0;
  pop->worst_val = 0.0;
  pop->worst_ind = 0;
  pop->parent    = (int *) NULL;
}

void compass_op_init_pop(compass_prob *prob, op_population *pop, int size)
{ int i;
  op_init_pop( pop, size);
  for (i=0; i<size; i++ )
  { struct op_solution *sol = &pop->solution[i];
    compass_op_init_sol(prob, sol);
  }
  return;
}

void compass_op_set_pop_sol(compass_prob *prob, op_population *pop,
    op_solution *sol, int pos)
{  op_solution *popsol = &pop->solution[pop->worst_ind];
  compass_op_copy_sol(prob, sol, popsol);
  return;
}

static void op_update_pop ( op_population *pop)
{ int i, qstep, stoppos;
  double *values;
  values = xcalloc(pop->size, sizeof(double));
  if (!values)
  { put_err_msg("op | pop: Error updating population.\n");
    goto cleanup;
  }
  pop->mean_val = 0.0;
  for (i=0; i< pop->size; i++)
  { op_solution *sol = &pop->solution[i];
    values[i] = sol->val;
    pop->rankperm[i] = i;
    pop->mean_val += sol->val/pop->size;
  }
  CCutil_double_perm_quicksort (pop->rankperm, values, pop->size);

  pop->best_ind = pop->rankperm[pop->size -1];
  pop->best_val = values[pop->best_ind];
  qstep = floor(pop->size/4.0);
  pop->q25_ind = pop->rankperm[pop->size-1 - qstep];
  pop->q25_val = values[pop->q25_ind];
  pop->q50_ind = pop->rankperm[pop->size-1 - 2*qstep];
  pop->q50_val = values[pop->q50_ind];
  pop->q75_ind = pop->rankperm[pop->size-1 - 3*qstep];
  pop->q75_val = values[pop->q75_ind];
  if (pop->stop_per)
  { stoppos = floor(pop->size/100.0*pop->stop_per)-1;
    pop->stop_ind = pop->rankperm[pop->size - stoppos];
    pop->stop_val = values[pop->stop_ind];
  }
  pop->worst_ind = pop->rankperm[0];
  pop->worst_val = values[pop->worst_ind];
cleanup:
  xfree(values);
  return;
}

void compass_op_update_pop(op_population *pop)
{ op_update_pop(pop);
  return;
}

static void op_delete_pop(op_population *pop)
{ int i;
  if (pop->parent != (int*) NULL)
    xfree(pop->parent);
  for (i=0; i<pop->size; i++ )
  { op_solution *sol = &pop->solution[i];
    op_delete_sol(sol);
  }
  tfree(pop->solution);
  xfree(pop->rankperm);
}

void compass_op_erase_pop(op_population *pop)
{ int size;
  size = pop->size;
  op_delete_pop(pop);
  op_init_pop(pop, size);
  return;
}

void compass_op_delete_pop(op_population *pop)
{ op_delete_pop(pop);
  xfree(pop);
  return;
}

void compass_convert_sol_op2tsp(compass_prob *prob, op_solution *opsol,
    tsp_solution *tspsol)
{ int i,j;
  for (i=0; i<opsol->ns; i++)
  { for (j=0; j<opsol->ns; j++)
    { if (opsol->sposition[j] == opsol->cycle[i])
      { tspsol->cycle[i] = j; break; }
    }
  }
  tspsol->val = opsol->length;
  return;
}

void compass_convert_sol_tsp2op(compass_prob *prob, tsp_solution *tspsol,
    op_solution *opsol, int *selected)
{ int i, j;
  int next, prev, ns;
  int *tselected;
  tselected = xcalloc(prob->n, sizeof(int));
  for (i=0; i<prob->n;i++)
    tselected[i] = selected[i];
  compass_op_erase_sol(prob, opsol);
  for (i=0; i<prob->n; i++)
  { opsol->selected[i]  = tselected[i];
    opsol->sposition[i] = prob->n;
    opsol->genotype[i]  =  i;
    opsol->cycle[i]     = -1;
  }
  j = 0;
  for (i = 0; i < prob->n; i++)
  { if (opsol->selected[i])
      opsol->sposition[j++] = i;
  }
  opsol->ns = j;
  for (i = 0; i < opsol->ns; i++) {
    opsol->cycle[i] = opsol->sposition[tspsol->cycle[i]];
  }
  prev= opsol->cycle[0];
  for (i = 1; i < opsol->ns; i++)
  { next = opsol->cycle[i];
    opsol->genotype[prev] = next;
    prev = next;
  }
  opsol->genotype[prev] = opsol->cycle[0];
  opsol->val = 0.0;
  for (i = 0; i < prob->n; i++)
  { if (opsol->selected[i])
      opsol->val += prob->op->s[i];
  }
  opsol->length = tspsol->val;
  xfree(tselected);
  return;
}

void compass_op_fit_solution ( compass_prob *prob, op_solution *sol,
    struct op_cp *opcp)
{ int i;

  OPdrop_operator ( prob->n, prob->data,
  &sol->ns, sol->selected, sol->sposition, sol->cycle, sol->genotype, &sol->length,
  prob->op->s, prob->op->d0, prob->rstate_cc);

  OPadd_operator (prob->kdtree, prob->n, prob->data,
  &sol->ns, sol->selected, sol->sposition, sol->cycle, sol->genotype, &sol->length,
  prob->op->d0, prob->rstate_cc);

  sol->val = 0.0;
  for (i=0; i<prob->n; i++)
  { if ( sol->selected[i] )
      sol->val += prob->op->s[i];
  }

  return;
}

