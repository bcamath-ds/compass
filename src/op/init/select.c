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
#include "rng.h"
#include "op.h"

static void
  eval_sol_obj (compass_prob *prob, op_solution *sol),
  select_nodes_greedily ( compass_prob *prob, op_solution *sol, double p),
  select_bernoulli ( compass_prob *prob, op_solution *sol, double p);

/**********************************************************************/
void compass_op_node_ranking ( compass_prob *prob, struct op_cp *opcp)
/**********************************************************************/
{ int i;
  struct op_prob *op = prob->op;
  double *values;
  values = xcalloc(prob->n, sizeof(double));
  //op->noderank = xcalloc(prob->n, sizeof(int));
  if (!values)
  { put_err_msg("op | init: Error ranking nodes.\n");
    goto cleanup;
  }
  for (i=0; i< prob->n; i++)
  { values[i] = op->s[i];
    op->noderank[i] = i;
  }

  CCutil_double_perm_quicksort (op->noderank, values, prob->n);
cleanup:
  xfree(values);
  return;
}

int cmp(const void *x, const void *y)
{
  double xx = *(double*)x, yy = *(double*)y;
  if (xx < yy) return -1;
  if (xx > yy) return  1;
  return 0;
}

/**********************************************************************/
void compass_op_select_nodes (compass_prob *prob, op_solution *sol,
    struct op_cp *opcp)
/**********************************************************************/
{ struct op_prob *op = prob->op;
  int i,j;

  if (opcp->pgreedy != 0.0)
    select_nodes_greedily(prob, sol,opcp->pgreedy);

  if ( opcp->sel_tech == OP_SEL_BERNOULLI )
  { select_bernoulli(prob, sol, opcp->pinit);
  }

  return;
}

/**********************************************************************/
static void select_nodes_greedily ( compass_prob *prob, op_solution *sol,
    double pgreedy)
/**********************************************************************/
{ int i, j, k, current, next, count, select, candidate, isdepotin, depotcounted;
  double cvalue, nvalue;
  int *candidates, *selected;
  int from = prob->op->from;
  double x;

  if ((1.0 - pgreedy)*prob->n <3)
    sol->greedycount = 3;
  else
    sol->greedycount = ceil((1.0-pgreedy)*prob->n);

  for (i = 0; i < prob->n; i++)
  { if ( i!=from )
      sol->greedylist[i] = 0;
    else
      sol->greedylist[from] = 1;
  }

  //for (i = 0, j= 0; i < prob->n; i++)
  //{
  i=0;
  depotcounted = 0;
  do {
    current = prob->op->noderank[prob->n-(depotcounted+i)-1];
    if (current == from)
    { depotcounted = 1;
      current = prob->op->noderank[prob->n-(depotcounted+i)-1];
    }
    cvalue = prob->op->s[current];

    j = i;
    int ncand=0;
    do {
      j++;
      next = prob->op->noderank[prob->n-(depotcounted+j)-1];
      if (next == from)
      { depotcounted = 1;
        next = prob->op->noderank[prob->n-(depotcounted+j)-1];
      }
      nvalue = prob->op->s[next];
      ncand++;
    } while ( cvalue == nvalue & i+1+ncand+depotcounted<sol->tot_n);

    if ( i+1+ncand < sol->greedycount )// +1 is the depot node
    {
      isdepotin = 0;
      for (k = 0; k < ncand; k++)
      { select = prob->op->noderank[prob->n-(i+isdepotin+k)-1];
        if (select == from)
        { isdepotin=1;
          select = prob->op->noderank[prob->n-(i+isdepotin+k)-1];
        }
        sol->greedylist[select] = 1;
      }
      i += ncand;
    }
    else
    { candidates = xcalloc ( ncand, sizeof(int));
      if (!candidates)
        goto cleanup;
      selected = xcalloc (sol->greedycount-i-1, sizeof(int));
      if (!selected)
        goto cleanup;
      isdepotin = 0;
      for (k = 0; k < ncand; k++)
      { candidate = prob->op->noderank[prob->n-(i+isdepotin+k)-1];
        if (candidate == from)
        { isdepotin=1;
          candidate = prob->op->noderank[prob->n-(i+isdepotin+k)-1];
        }
        candidates[k] = candidate;
      }
      gsl_ran_choose (prob->rstate_gsl, selected, sol->greedycount-i-1,
          candidates, ncand, sizeof (int));
      for (k = 0; k < sol->greedycount-i-1; k++)
      {
        sol->greedylist[selected[k]] = 1;
      }
      i=sol->greedycount-1;
cleanup:
      xfree(candidates);
      xfree(selected);
    }
  } while (i<sol->greedycount-1);
  return;
}

/**********************************************************************/
static void select_bernoulli ( compass_prob *prob, op_solution *sol,
    double p)
/**********************************************************************/
{ int i, j, ns, prev;
  int from = prob->op->from;
  double x;

  do
  {
    for (i = 0, j= 0; i < prob->n; i++)
    { if ( i==from )
      { sol->selected[from] = 1;
        sol->sposition[j++] = from;//FIXME: NO SURE
      }
      else if (!sol->greedylist[i])
      { sol->selected[i] = 0;
        sol->sposition[i] = -1;
      }
      else
      { x = rng_unif_01(prob->rstate);
        if(  x < sqrt(sol->tot_n/sol->greedycount*p) )
          // Obs: If n=greedycount => x<sqrt(p)
        { sol->selected[i] = 1;
          sol->sposition[j++] = i;
        }
        else
        { sol->selected[i] = 0;
          sol->sposition[i] = -1;
        }
      }
    }
    ns = 0;
    for (i = 0; i < prob->n; i++)
      if (sol->selected[i]) ns++;
  } while (ns <= 3);
  sol->ns = ns;
  eval_sol_obj(prob, sol);
  return;
}

/**********************************************************************/
static void eval_sol_obj (compass_prob *prob, op_solution *sol)
/**********************************************************************/
{ struct op_prob *op = prob->op;
  int i;
  sol->val = 0.0;
  for (i = 0; i < prob->n ; i++)
   if (sol->selected[i])
     sol->val += op->s[i];
  return;
}

