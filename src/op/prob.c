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


/***********************************************************************
*  NAME
*
*  compass_init_op_prob - create problem object
*
*  SYNOPSIS
*
*  void compass_init_op_prob(op_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_init_op_prob creates a new problem object, which is
*  initially "empty", i.e. has no rows and columns.
*
*  RETURNS
*
*  The routine returns a pointer to the object created, which should be
*  used in any subsequent operations on this object. */

static void op_init_prob(compass_prob *prob)
{ struct op_prob *op = prob->op;
  int i;
  op->magic = COMPASS_PROB_MAGIC;
  op->obj = NULL;
  //op->lp = (glp_prob *) NULL;
  op->nne = 0;
  /* Input data */
  op->s = xcalloc(prob->n, sizeof(double));
  for (i=0; i< prob->n; i++)
    op->s[i] = 0;
  op->d0 = 0.0;
  op->from = 0;
  op->to = 0;
  op->ecount = 0;
  op->elist = (int *) NULL;
  op->noderank = xcalloc(prob->n, sizeof(int));
  for (i=0; i< prob->n; i++)
    op->noderank[i] = i;
  op->sol_stat = COMPASS_UNDEF;
  op->sol = xmalloc(sizeof(op_solution));
  op->population = xmalloc(sizeof(struct op_population));
  return;
}

void compass_op_init_prob( compass_prob *prob)
{ prob->op = xmalloc(sizeof(struct op_prob));
  op_init_prob(prob);
  compass_op_init_sol(prob, prob->op->sol);
  return;
}

/***********************************************************************
*  NAME
*
*  compass_op_set_obj_name - assign (change) objective function name
*
*  SYNOPSIS
*
*  void compass_set_op_obj_name(op_prob *prob, const char *name);
*
*  DESCRIPTION
*
*  The routine compass_op_set_obj_name assigns a given symbolic name (1 up to
*  255 characters) to the objective function of the specified problem
*  object.
*
*  If the parameter name is NULL or empty string, the routine erases an
*  existing name of the objective function. */

void compass_op_set_obj_name(compass_prob *prob, const char *name)
{ if (prob->op->obj != NULL)
  {  dmp_free_atom(prob->pool, prob->op->obj, strlen(prob->op->obj)+1);
     prob->op->obj = NULL;
  }
  if (!(name == NULL || name[0] == '\0'))
  { int k;
    for (k = 0; name[k] != '\0'; k++)
    { if (k == 256)
        xerror("compass_set_op_obj_name: objective name too long\n");
      if (iscntrl((unsigned char)name[k]))
        xerror("compass_set_op_obj_name: objective name contains invalid character(s)\n");
    }
    prob->op->obj = dmp_get_atom(prob->pool, strlen(name)+1);
    strcpy(prob->op->obj, name);
  }
  return;
}

/***********************************************************************
*  NAME
*
*  compass_op_erase_prob - erase problem object content
*
*  SYNOPSIS
*
*  void compass_op_erase_prob(compass_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_erase_op_prob erases the content of the specified
*  problem object. The effect of this operation is the same as if the
*  problem object would be deleted with the routine compass_op_delete_prob and
*  then created anew with the routine compass_op_create_prob, with exception
*  that the handle (pointer) to the problem object remains valid. */

static void op_delete_prob( compass_prob *prob);

void compass_op_erase_prob(compass_prob *prob)
{     //compass_op_tree *tree = prob->tree;
      //if (tree != NULL && tree->reason != 0)
      //   xerror("compass_op_erase_prob: operation not allowed\n");
      op_delete_prob(prob);
      op_init_prob(prob);
      return;
}

/***********************************************************************
*  NAME
*
*  compass_op_delete_prob - delete problem object
*
*  SYNOPSIS
*
*  void compass_op_delete_prob(compass_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_op_delete_prob deletes the specified problem object and
*  frees all the memory allocated to it. */

static void op_delete_prob(compass_prob *prob)
{ struct op_prob *op = prob->op;
  op->magic = 0x3F3F3F3F;
  xfree(op->noderank);
  xfree(op->s);
  if (op->elist != NULL) xfree (op->elist);
  compass_op_delete_sol (op->sol);
  return;
}

void compass_op_delete_prob(compass_prob *prob)
{ op_delete_prob(prob);
  xfree(prob->op);
  return;
}

/* eof */
