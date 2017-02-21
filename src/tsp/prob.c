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

static void tsp_init_sol(int n, struct tsp_solution *sol)
{ int i;
  sol->cycle  = xcalloc (n, sizeof(int));
  sol->val    = 1e30;
  for (i=0;i<n;i++)
    sol->cycle[i] = i;
}

void compass_tsp_init_sol(compass_prob *prob, tsp_solution *sol)
{ tsp_init_sol(prob->n, sol);
  return;
}

static void tsp_delete_sol( struct tsp_solution *sol)
{ xfree(sol->cycle);
}

void compass_tsp_erase_sol(compass_prob *prob, struct tsp_solution *sol)
{ tsp_delete_sol(sol);
  tsp_init_sol(prob->n, sol);
  return;
}

void compass_tsp_delete_sol(tsp_solution *sol)
{ tsp_delete_sol(sol);
  xfree(sol);
  return;
}

void compass_tsp_copy_sol(compass_prob *prob, tsp_solution *insol,
    tsp_solution *outsol)
{ int i;
  compass_tsp_erase_sol(prob, outsol);
  outsol->val = insol->val;
  for (i=0; i<prob->n; i++)
    outsol->cycle[i] = insol->cycle[i];
  return;
}


/***********************************************************************
*  NAME
*
*  compass_init_op_prob - create problem object
*
*  SYNOPSIS
*
*  compass_prob *compass_init_op_prob(void);
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

static void tsp_init_prob( compass_prob *prob)
{ struct tsp_prob *tsp = prob->tsp;
  tsp->magic = COMPASS_PROB_MAGIC;
  tsp->obj = NULL;
  //tsp->lp = (glp_prob *) NULL;
  tsp->ecount = 0;
  tsp->elist = (int *) NULL;
  //tsp->ecount = xcalloc(1, sizeof(int));
  //tsp->elist = xcalloc(2*prob->n, sizeof(int));
  /* Input data */
  /* solution */
  tsp->sol_stat = COMPASS_UNDEF;
  tsp->sol = xmalloc(sizeof(struct tsp_solution ));
  compass_tsp_init_sol(prob, tsp->sol);
  return;
}

void compass_tsp_init_prob( compass_prob *prob)
{ prob->tsp = xmalloc(sizeof(struct tsp_prob));
  tsp_init_prob(prob);
  return;
}

/***********************************************************************
*  NAME
*
*  compass_set_tsp_obj_name - assign (change) objective function name
*
*  SYNOPSIS
*
*  void compass_set_tsp_obj_name(tsp_prob *prob, const char *name);
*
*  DESCRIPTION
*
*  The routine compass_set_tsp_obj_name assigns a given symbolic name (1 up to
*  255 characters) to the objective function of the specified problem
*  object.
*
*  If the parameter name is NULL or empty string, the routine erases an
*  existing name of the objective function. */

void compass_set_tsp_obj_name(compass_prob *prob, const char *name)
{ if (prob->tsp->obj != NULL)
  {  dmp_free_atom(prob->pool, prob->tsp->obj, strlen(prob->tsp->obj)+1);
     prob->tsp->obj = NULL;
  }
  if (!(name == NULL || name[0] == '\0'))
  { int k;
    for (k = 0; name[k] != '\0'; k++)
    { if (k == 256)
        xerror("compass_set_tsp_obj_name: objective name too long\n");
      if (iscntrl((unsigned char)name[k]))
        xerror("compass_set_tsp_obj_name: objective name contains invalid character(s)\n");
    }
    prob->tsp->obj = dmp_get_atom(prob->pool, strlen(name)+1);
    strcpy(prob->tsp->obj, name);
  }
  return;
}

/***********************************************************************
*  NAME
*
*  compass_tsp_erase_prob - erase problem object content
*
*  SYNOPSIS
*
*  void compass_tsp_erase_prob(compass_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_erase_tsp_prob erases the content of the specified
*  problem object. The effect of this operation is the same as if the
*  problem object would be deleted with the routine compass_tsp_delete_prob and
*  then created anew with the routine compass_tsp_create_prob, with exception
*  that the handle (pointer) to the problem object remains valid. */

static void tsp_delete_prob(struct tsp_prob *prob);

void compass_tsp_erase_prob(compass_prob *prob)
{     //compass_op_tree *tree = prob->tree;
      //if (tree != NULL && tree->reason != 0)
      //   xerror("compass_op_erase_prob: operation not allowed\n");
      tsp_delete_prob(prob->tsp);
      tsp_init_prob(prob);
      return;
}

/***********************************************************************
*  NAME
*
*  compass_tsp_delete_prob - delete problem object
*
*  SYNOPSIS
*
*  void compass_delete_delete_prob(compass_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_tsp_delete_prob deletes the specified problem object and
*  frees all the memory allocated to it. */

static void tsp_delete_prob(struct tsp_prob *tsp)
{ tsp->magic = 0x3F3F3F3F;
  if (tsp->elist != (int *) NULL)
    xfree (tsp->elist);
  tsp_delete_sol( tsp->sol);
  xfree (tsp->sol);
  return;
}

void compass_tsp_delete_prob(compass_prob *prob)
{ tsp_delete_prob(prob->tsp);
  xfree(prob->tsp);
  return;
}

/* eof */
