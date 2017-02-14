#include "compass.h"
#include "neigh.h"
#include "data/kdtree/kdtree.h"
#include "tsp.h"
#include "env.h"

static int call_nearest (compass_prob *prob, struct neigh_cp *neighcp);
static int call_quadnearest (compass_prob *prob, struct neigh_cp *neighcp);
static int call_delaunay (compass_prob *prob, struct neigh_cp *neighcp);

int compass_data_k_nearest (compass_prob *prob, struct neigh_cp *neighcp)
{ int ret;
  switch (neighcp->neigh_graph) {
  case NEIGH_NEAREST:
    if (call_nearest(prob, neighcp))
    { put_err_msg("data  :   call_nearest failed.\n");
      return 1;
    }
    goto done;
  case NEIGH_QUADNEAREST:
    if (call_quadnearest (prob, neighcp))
    { put_err_msg(stderr, "data  :   call_quadnearest failed.\n");
      return 1;
    }
    goto done;
  case NEIGH_DELAUNAY:
    if (call_delaunay (prob, neighcp))
    { put_err_msg(stderr, "data  :   call_delaunay failed.\n");
      ret = 1; goto done;
    }
    goto done;
  default:
    put_err_msg(stderr, "data  :   invalid neigh_graph flag.\n");
    return 1;
  }
done:
  return 0;
}

static int call_nearest (compass_prob *prob, struct neigh_cp *neighcp)
{ int ret, silent;
  struct compass_data *data = prob->data;
  struct tsp_prob *tsp = prob->tsp;
  if (neighcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("data  :  Generating %d-nearest Neighbor Graph...", neighcp->k);
    silent = 0;
  }
  else
    silent = 1;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { CCkdtree_build(prob->kdtree, prob->n, prob->data, (double *) NULL, prob->rstate_cc);
    if (CCkdtree_k_nearest (prob->kdtree, prob->n, neighcp->k, data, (double *) NULL,
        1, &tsp->ecount, &tsp->elist, silent, prob->rstate_cc))
    { fprintf (stderr, "CCkdtree_k_nearest failed\n");
      return 1;
    }
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  {
    if (CCedgegen_x_k_nearest (prob->n, neighcp->k, data, (double *) NULL, 1,
        &tsp->ecount, &tsp->elist, silent))
    { fprintf (stderr, "CCedgegen_x_k_nearest failed\n");
      return 1;
    }
  }
  else
  { if (CCedgegen_junk_k_nearest (prob->n, neighcp->k, data, (double *) NULL, 1,
        &tsp->ecount, &tsp->elist, silent))
    { fprintf (stderr, "CCedgegen_junk_k_nearest failed\n");
      return 1;
    }
  }
  return 0;
}


static int call_quadnearest (compass_prob *prob, struct neigh_cp *neighcp)
{ int ret, silent;
  struct compass_data *data = prob->data;
  struct tsp_prob *tsp = prob->tsp;
  if (neighcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("data  :  Generating %d-nearest Neighbor Graph...", neighcp->k);
    silent = 0;
  }
  else
    silent = 1;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { if (CCkdtree_quadrant_k_nearest (prob->kdtree, prob->n, neighcp->k, data,
        (double *) NULL, 1, &tsp->ecount, &tsp->elist, silent, prob->rstate_cc))
    { fprintf (stderr, "CCkdtree_quadrant_k_nearest failed\n");
      return 1;
    }
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (CCedgegen_x_quadrant_k_nearest (prob->n, neighcp->k, data,
        (double *) NULL, 1, &tsp->ecount, &tsp->elist, silent))
    { fprintf (stderr, "CCedgegen_x_quadrant_k_nearest failed\n");
      return 1;
    }
  }
  else
  { if (neighcp->msg_lev >= COMPASS_MSG_ALL)
    { printf ("data  : Cannot run quadrant nearest with JUNK norms\n");
      printf ("data  : Trying %d-nearest instead\n", 2 * neighcp->k);
    }
    if (CCedgegen_junk_k_nearest (prob->n, 2 * neighcp->k, data, (double *) NULL,
          1, &tsp->ecount, &tsp->elist, silent))
    { fprintf (stderr, "CCedgegen_junk_k_nearest failed\n");
      return 1;
    }
  }
  //if (neighcp->msg_lev >= COMPASS_MSG_ALL)
  return ret;
}

static int call_delaunay (compass_prob *prob, struct neigh_cp *neighcp)
{ int ret, silent;
  struct compass_data *data = prob->data;
  struct tsp_prob *tsp = prob->tsp;
  if (neighcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("data  :  Generating Delaunay Neighbor Graph... ", neighcp->k);
    silent = 0;
  }
  else
    silent = 1;
  if (data->norm == CC_EUCLIDEAN || data->norm == CC_EUCLIDEAN_CEIL)
  { if (CCedgegen_delaunay (prob->n, data, 1, &tsp->ecount, &tsp->elist))
    { put_err_msg(stderr, "delaunay failed\n");
      return 1;
    }
  }
  else
  { if (neighcp->msg_lev >= COMPASS_MSG_ALL)
    { xprintf ("data  : No Delaunay triangulation with norm %d\n", data->norm);
      xprintf ("data  : Using nearest neightbor set%d\n");
    }
    call_nearest(prob, neighcp);
    return 0;
  }
  //if (neighcp->msg_lev >= COMPASS_MSG_ALL)
  return 0;
}

struct neigh_cp *compass_neigh_init_cp( void )
{ struct neigh_cp *neighcp;
  neighcp = xmalloc(sizeof(struct neigh_cp));
  neighcp->msg_lev = COMPASS_MSG_ON;
  neighcp->tm_start = xtime();
  neighcp->neigh_graph = NEIGH_NEAREST;
  neighcp->k = 10;
  return neighcp;
}
