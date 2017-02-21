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
#include "gsl/gsl_rng.h"

/******************************************************************************/
void compass_op_mutate_sol (compass_prob *prob, op_solution *sol,
    struct op_eacp *eacp)
/******************************************************************************/
{ int i, node;
  int *candidates, *change, *indexes, *scount;
  double runif;

    change = xcalloc ( 1, sizeof(int ));
    if (!change) {
      goto cleanup;
    }
    candidates = xcalloc (prob->n-1, sizeof(int));
    if (!candidates) {
      goto cleanup;
    }
    for (i = 1; i < prob->n; i++)
    {
      candidates[i-1] = i;
    }

    gsl_ran_choose (prob->rstate_gsl, change, 1, candidates, prob->n-1, sizeof (int));
    node= (int) change[0];

    if (sol->selected[node]) {
      OPdrop_node (prob->n, prob->data, node,
      &sol->ns, sol->selected, sol->sposition, sol->cycle, sol->genotype, &sol->length,
      prob->rstate_cc);
      sol->val -= prob->op->s[node];
    } else {
      OPadd_node (prob->kdtree, prob->n, prob->data, node,
      &sol->ns, sol->selected, sol->sposition, sol->cycle, sol->genotype, &sol->length,
      prob->rstate_cc);
      sol->val += prob->op->s[node];
    }

cleanup:
    xfree (change);
    xfree (candidates);
  return;

}


