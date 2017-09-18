#include "compass.h"
#include "rng.h"
#include "data/kdtree/kdtree.h"
#include "util.h"
#include "env.h"
#include <gsl/gsl_rng.h>
#include "dmp.h"

static const gsl_rng_type *T = NULL;


/***********************************************************************
*  NAME
*
*  compass_create_prob - create problem object
*
*  SYNOPSIS
*
*  compass_prob *compass_create_prob(void);
*
*  DESCRIPTION
*
*  The routine compass_create_prob creates a new problem object, which is
*  initially "empty", i.e. has no rows and columns.
*
*  RETURNS
*
*  The routine returns a pointer to the object created, which should be
*  used in any subsequent operations on this object. */

static void init_prob(compass_prob *prob)
{ prob->magic = COMPASS_PROB_MAGIC;
  xassert(sizeof(void *) <= 8);
  prob->pool = talloc(1, DMP);
  dmp_create_pool(prob->pool);
  prob->name = xcalloc(100,sizeof(char));
  memcpy(prob->name, "null", 5);
  prob->n = 1;
  prob->data = xmalloc(sizeof(compass_data ));
  compass_init_data(prob->data);
  prob->cacheind  = (int *) NULL;
  prob->cacheval  = (int *) NULL;
  prob->cacheM = 0;
  //prob->kdtree = (CCkdtree *) NULL;
  prob->kdtree = xmalloc(sizeof(CCkdtree));
  prob->kdtree->root = (CCkdtree *) NULL;
  //prob->kdtree = xcalloc(1, sizeof(CCkdtree *));
  prob->tsp = (struct tsp_prob *) NULL;
  prob->op = (struct op_prob *) NULL;
  prob->hash = xcalloc(32,sizeof(unsigned char));
  prob->ctx = xmalloc(sizeof(SHA256_CTX));
  return;
}

void compass_init_prob(compass_prob *prob)
{ init_prob(prob);
  return;
}


void compass_init_rng(compass_prob *prob, int seed)
{ T = gsl_rng_default;
  CCrandstate rstate_cc;
  prob->rstate_cc = talloc(1, CCrandstate);
  CCutil_sprand (seed, prob->rstate_cc);
  prob->rstate_gsl = gsl_rng_alloc (T);
  gsl_rng_set (prob->rstate_gsl, seed);
  prob->rstate = rng_create_rand();
}

void compass_free_rng(compass_prob *prob)
{ gsl_rng_free (prob->rstate_gsl);
  rng_delete_rand(prob->rstate);
  xfree(prob->rstate_cc);
}

/***********************************************************************
*  NAME
*
*  compass_set_prob_name - assign (change) problem name
*
*  SYNOPSIS
*
*  void compass_set_prob_name(compass_prob *prob, const char *name);
*
*  DESCRIPTION
*
*  The routine compass_set_prob_name assigns a given symbolic name (1 up to
*  255 characters) to the specified problem object.
*
*  If the parameter name is NULL or empty string, the routine erases an
*  existing symbolic name of the problem object. */

void compass_set_prob_name(compass_prob *prob, const char *name)
{ //compass_tree *tree = prob->tree;
  //if (tree != NULL && tree->reason != 0)
  //   xerror("compass_op_set_prob_name: operation not allowed\n");
  if (prob->name != NULL)
  {  dmp_free_atom(prob->pool, prob->name, strlen(prob->name)+1);
     prob->name = NULL;
  }
  if (!(name == NULL || name[0] == '\0'))
  {  int k;
     for (k = 0; name[k] != '\0'; k++)
     {  if (k == 256)
           xerror("compass_op_set_prob_name: problem name too long\n");
        if (iscntrl((unsigned char)name[k]))
           xerror("compass_op_set_prob_name: problem name contains invalid"
              " character(s)\n");
     }
     prob->name = dmp_get_atom(prob->pool, strlen(name)+1);
     strcpy(prob->name, name);
  }
  return;
}

/***********************************************************************
*  NAME
*
*  compass_erase_prob - erase problem object content
*
*  SYNOPSIS
*
*  void compass_erase_prob(compass_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_erase_prob erases the content of the specified
*  problem object. The effect of this operation is the same as if the
*  problem object would be deleted with the routine compass_op_delete_prob and
*  then created anew with the routine compass_create_prob, with exception
*  that the handle (pointer) to the problem object remains valid. */

static void delete_prob(compass_prob *prob);

void compass_erase_prob(compass_prob *prob)
{ //compass_op_tree *tree = prob->tree;
  //if (tree != NULL && tree->reason != 0)
  //   xerror("compass_op_erase_prob: operation not allowed\n");
  delete_prob(prob);
  init_prob(prob);
  return;
}

/***********************************************************************
*  NAME
*
*  compass_delete_prob - delete problem object
*
*  SYNOPSIS
*
*  void compass_delete_prob(compass_op_prob *prob);
*
*  DESCRIPTION
*
*  The routine compass_delete_prob deletes the specified problem object and
*  frees all the memory allocated to it. */


static void delete_prob(compass_prob *prob)
{ //if ((prob->data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  if (prob->kdtree->root != (CCkdtree *) NULL)
    CCkdtree_free(prob->kdtree);
  xfree(prob->kdtree);
  compass_delete_data(prob->data);
  xfree(prob->name);
  dmp_delete_pool(prob->pool);
  //xfree(prob->cacheind);
  //xfree(prob->cacheval);
  return;
}

void compass_delete_prob(compass_prob *prob)
{ delete_prob(prob);
  xfree(prob);
  return;
}

/***********************************************************************
*  NAME
*
*  compass_sub_prob - assign (change) select a subproblem
*
*  SYNOPSIS
*
*  void compass_sub_prob(compass_prob *inprob, compass_prob *outprob, int *nodes);
*
*  DESCRIPTION
*
*  The routine compass_set_prob_name assigns a given symbolic name (1 up to
*  255 characters) to the specified problem object.
*
*  If the parameter name is NULL or empty string, the routine erases an
*  existing symbolic name of the problem object. */

int compass_sub_data ( struct compass_data *indata, struct compass_data *outdata, int *selected)
{ int ret;
  int i, j, ri, rj, nout;
  compass_erase_data(outdata);
  nout = 0;
  for ( i=0; i< indata->n; i++)
  { if (selected[i])
      nout++;
  }
  outdata->n=nout;
  outdata->norm = indata->norm;
  if (indata->x != (double *) NULL)
  { double *tempx;
    int j;
    tempx = xcalloc ( outdata->n, sizeof(double));
    if (!tempx)
    { put_err_msg("out of memory in CCutil_copy_datagroup\n");
      ret = 1; goto cleanup;
    }
    for (i = 0, j = 0; i < indata->n; i++)
    { if (selected[i])
       tempx[j++] = indata->x[i];
    }
    outdata->x = tempx;
  }
  if (indata->y != (double *) NULL)
  { double *tempy;
    int j;
    tempy = xcalloc ( outdata->n, sizeof(double) );
    if (!tempy)
    { put_err_msg("out of memory in CCutil_copy_datagroup\n");
      ret = 1; goto cleanup;
    }

    for (i = 0, j=0; i < indata->n; i++)
    { if (selected[i])
      { tempy[j++] = indata->y[i];
      }
    }
    outdata->y = tempy;
  }
  if (indata->z != (double *) NULL)
  { double *tempz;
    int j;
    tempz = xcalloc (outdata->n, sizeof(double));
    if (!tempz)
    { put_err_msg("out of memory in CCutil_copy_datagroup\n");
      ret = 1; goto cleanup;
    }
    for (i = 0, j=0; i < indata->n; i++)
    { if (selected[i])
      { tempz[j++] = indata->z[i];
      }
    }
    outdata->z = tempz;
  }
  if (indata->adj != (int **) NULL)
  { int **tempadj     = (int **) NULL;
    int *tempadjspace = (int *) NULL;
    tempadj      = xcalloc (outdata->n, sizeof(int *));
    tempadjspace = xcalloc (outdata->n * (outdata->n+1) / 2, sizeof(int));

    if (!tempadj ||!tempadjspace)
    { put_err_msg("out of memory in CCutil_copy_datagroup\n");
      xfree (tempadj);
      xfree (tempadjspace);
      ret = 1; goto cleanup;
    }
    for (i = 0, j = 0; i < outdata->n; i++)
    { tempadj[i] = tempadjspace + j;
      j += (i+1);
    }

    ri = 0;
    for (i = 0, j = 0; i < indata->n; i++)
    { if (selected[i])
      { rj = 0;
        for (j = 0; j <= i; j++)
        { if (selected[j])
          { tempadj[ri][rj] = indata->adj[i][j];
            rj++;
          }
        }
      ri++;
      }
    }
    outdata->adj = tempadj;
    outdata->adjspace = tempadjspace;
  }
  compass_data_set_norm(outdata, outdata->norm);
  ret = 0;
cleanup:
  if (ret)
   compass_erase_data (outdata);
  return ret;
}

int compass_sub_prob(compass_prob *inprob, compass_prob *outprob, int *selected)
{ int ret, i, nout;
  //if (outprob != NULL)
  nout = 0;
  for ( i=0; i< inprob->n; i++)
  { if (selected[i])
      nout++;
  }
  outprob->n = nout;
  if (compass_sub_data( inprob->data, outprob->data, selected))
  { put_err_msg( "out of memory in compass_sub_data\n");
    ret = 1; goto done;
  }
  outprob->kdtree = talloc(1, CCkdtree);
  outprob->kdtree->root = (CCkdtree *) NULL;

  outprob->tsp = (struct tsp_prob *) NULL;
  outprob->op = (struct op_prob *) NULL;
  outprob->rstate_cc = inprob->rstate_cc;
  outprob->rstate_gsl = inprob->rstate_gsl;
  outprob->rstate = inprob->rstate;
  ret = 0;
done:
  return ret;
}


/* eof */
