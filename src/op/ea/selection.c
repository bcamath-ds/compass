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
#include "op.h"
#include "env.h"
#include "gsl/gsl_rng.h"

#define BIGDOUBLE (1e30)

/******************************************************************************/
void compass_op_choose_sol (compass_prob *prob, op_population *pop,
    int nparsel, int *parents, struct op_cp *opcp)
/******************************************************************************/
{ int i, parent;
  double min=BIGDOUBLE;
  int sum=0;
  double *reproductive_probs, *tprobs;
  int *preselected, *selected;
  int *indexes;
  reproductive_probs = xcalloc (nparsel, sizeof(double));
  if (!reproductive_probs)
    goto cleanup;
  tprobs = xcalloc (nparsel, sizeof(double));
  if (!tprobs)
    goto cleanup;
  selected = xcalloc (nparsel, sizeof(int));
  if (!selected)
    goto cleanup;
  preselected = xcalloc (nparsel, sizeof(int));
  if (!preselected)
    goto cleanup;
  indexes = xcalloc ( pop->size, sizeof(int));
  if (!indexes)
    goto cleanup;
  for (i = 0; i < pop->size; i++)
    indexes[i] = i;
  gsl_ran_choose (prob->rstate_gsl, preselected, nparsel, indexes, pop->size,
      sizeof (int));
  for (i=0; i< nparsel; i++)
  {  op_solution *sol = &pop->solution[preselected[i]];
    if (sol->val < min )
      min = sol->val;
  }
  // We add 1 to ensure that are not null.
  for (i=0; i< nparsel; i++)
    tprobs[i] = pop->solution[preselected[i]].val - min +1;
  for (i=0; i< nparsel; i++)
    sum += tprobs[i];
  for (i=0; i< nparsel; i++)
    reproductive_probs[i] = tprobs[i]/ sum;
  gsl_ran_multinomial(prob->rstate_gsl, nparsel, 2, reproductive_probs, selected);
  parent=0;
  for (i=0; i< nparsel; i++)
  { if (selected[i] == 1) {
      parents[parent] = preselected[i];
      parent ++;
    } else if (selected[i] == 2) {
      parents[0] = preselected[i];
      parents[1] = preselected[i];
      parent +=2;
    }
    if (parent==2) break;
  }
cleanup:
  xfree(reproductive_probs);
  xfree(tprobs);
  xfree(selected);
  xfree(preselected);
  xfree(indexes);
  return;
}
