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
#include "macrorus.h"

static void
randcycle (int ncount, int *cyc, CCrandstate *rstate);
static int
call_random_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_greedy_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_qboruvka_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_boruvka_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp),
call_nearest_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp);

int compass_tsp_start_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp )
{ switch (tspcp->start_tour) {
  case TSP_RANDOM_TOUR:
    if (call_random_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_randcycle failed\n");
      compass_tsp_delete_sol(sol);
      return 1;
    }
    goto done;
  case TSP_GREEDY_TOUR:
    if (call_greedy_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_greedy_tour failed\n");
      compass_tsp_delete_sol(sol);
      return 1;
    }
    goto done;
  case TSP_QBORUVKA_TOUR:
    if (call_qboruvka_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_qboruvka_tour failed\n");
      compass_tsp_delete_sol(sol);
      return 1;
    }
    goto done;
  case TSP_BORUVKA_TOUR:
    if (call_boruvka_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_boruvka_tour failed\n");
      compass_tsp_delete_sol(sol);
      return 1;
    }
    goto done;
  case TSP_NEIGHBOR_TOUR:
    if (call_nearest_tour (prob, sol, tspcp))
    { put_err_msg("tsp   :   call_nearest_tour failed\n");
      compass_tsp_delete_sol(sol);
      return 1;
    }
    goto done;
  default:
    put_err_msg("tsp   :   invalid start_tour flag\n");
    compass_tsp_delete_sol(sol);
    return 1;
  }
done:
  return 1;
}


static int call_random_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ compass_data *data = prob->data;
  int i;
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    xprintf ("tsp   :  Generating Random Rour...");
  randcycle (prob->n, sol->cycle, prob->rstate_cc);
  sol->val  = CCutil_dat_edgelen (sol->cycle[prob->n - 1], sol->cycle[0], data);
  for (i = 1; i < prob->n; i++)
    sol->val += CCutil_dat_edgelen (sol->cycle[i - 1], sol->cycle[i], data);
  //xprintf ("#tsp   : cyc %d\n", sol->cycle[1]);
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    xprintf ("tsp   :  Generating Random Rour...");
  return 0;
}

static void randcycle (int ncount, int *cyc, CCrandstate *rstate)
{ int i, k, temp;
  for (i = 0; i < ncount; i++)
    cyc[i] = i;

  for (i = ncount; i > 2; i--)
  { k = CCutil_lprand (rstate) % i;
    if (k!=0)
      CC_SWAP (cyc[i - 1], cyc[k], temp);
  }
  return;
}


static int call_nearest_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ int ret, start, silent;
  compass_data *data = prob->data;
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("tsp   :  Generating Nearest Tour...");
    silent = 0;
  }
  else
    silent = 1;
  start = CCutil_lprand (prob->rstate_cc) % prob->n;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { if (CCkdtree_nearest_neighbor_tour (prob->kdtree, prob->n, start,
        data, sol->cycle, &sol->val, prob->rstate_cc))
    { put_err_msg("CCkdtree_nearest_neighbor_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (CCedgegen_x_nearest_neighbor_tour (prob->n, start, data,
        sol->cycle, &sol->val))
    { put_err_msg("CCedgegen_x_nearest_neighbor_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  else
  { if (CCedgegen_junk_nearest_neighbor_tour (prob->n, start, data,
        sol->cycle, &sol->val, silent))
    { put_err_msg("CCedgegen_junk_nearest_neighbor_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    xprintf("  length %.2f\n", sol->val);
  return 0;
}

static int call_greedy_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ int ret, start, silent, t;
  compass_data *data = prob->data;
  struct tsp_prob *tsp = prob->tsp;
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("tsp   :  Generating Greedy Tour...");
    silent = 0;
  }
  else
    silent = 1;
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { if (CCkdtree_greedy_tour (prob->kdtree, prob->n, data, sol->cycle,
        &sol->val, silent, prob->rstate_cc))
    { put_err_msg("CCkdtree_greedy_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (CCedgegen_x_greedy_tour (prob->n, data, sol->cycle, &sol->val,
        tsp->ecount, tsp->elist, silent))
    { fprintf (stderr, "CCedgegen_x_greedy_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  else
  { if (prob->n < 9)
      t = prob->n - 1;
    else
      t = 8;
    if (CCedgegen_junk_greedy_tour (prob->n, data, sol->cycle, &sol->val,
          tsp->ecount, tsp->elist, silent))
    { fprintf (stderr, "CCedgegen_junk_greedy_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    xprintf("  length %.2f\n", sol->val);
  return 0;
}

static int call_boruvka_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ int ret, start, silent;
  compass_data *data = prob->data;
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("tsp   :  Generating Boruvka tour...");
    silent = 0;
  }
  else
    silent = 1;
  if (!sol->cycle)
  { fprintf (stderr, "tsp   :   Out of memory in call_boruvka_tour\n");
    compass_tsp_erase_sol(sol);
    return 1;
  }
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { if (CCkdtree_boruvka_tour (prob->kdtree, prob->n, data, sol->cycle,
        &sol->val, prob->rstate_cc))
    { fprintf (stderr, "CCkdtree_boruvka_tour failed\n");
    compass_tsp_erase_sol(sol);
    return 1;
    }
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf (" warning! No X_NORM boruvka tour.\n");
    compass_tsp_erase_sol(sol);
    ret = call_nearest_tour (prob, sol, tspcp);
    return ret;
  }
  else
  { if (tspcp->msg_lev >= COMPASS_MSG_ALL)
      xprintf (" warning! No JUNK_NORM boruvka tour.\n");
    compass_tsp_erase_sol(sol);
    ret = call_nearest_tour (prob, sol, tspcp);
    return ret;
  }
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    xprintf("  length %.2f\n", sol->val);
  return 0;
}

static int call_qboruvka_tour (compass_prob *prob, tsp_solution *sol,
    struct tsp_cp *tspcp)
{ int silent;
  compass_data *data = prob->data;
  struct tsp_prob *tsp = prob->tsp;
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
  { xprintf ("tsp   :  Generating qboruvka tour...");
    silent = 0;
  }
  else
    silent = 1;
  if (!sol->cycle)
  { fprintf (stderr, "tsp   :   Out of memory in call_qboruvka_tour\n");
    compass_tsp_erase_sol(sol);
    return 1;
  }
  if ((data->norm & CC_NORM_BITS) == CC_KD_NORM_TYPE)
  { if (CCkdtree_qboruvka_tour (prob->kdtree, prob->n, data, sol->cycle,
        &sol->val, prob->rstate_cc))
    { fprintf (stderr, "CCkdtree_qboruvka_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  else if ((data->norm & CC_NORM_BITS) == CC_X_NORM_TYPE)
  { if (CCedgegen_x_qboruvka_tour (prob->n, data, sol->cycle, &sol->val,
        tsp->ecount, tsp->elist, 1))
    { put_err_msg("CCedgegen_x_qboruvka_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  else
  { if (CCedgegen_junk_qboruvka_tour (prob->n, data, sol->cycle, &sol->val,
        tsp->ecount, tsp->elist, silent))
    { put_err_msg("CCedgegen_junk_qboruvka_tour failed\n");
      compass_tsp_erase_sol(sol);
      return 1;
    }
  }
  if (tspcp->msg_lev >= COMPASS_MSG_ALL)
    xprintf("  length %.2f\n", sol->val);
  return 0;
}

#if 0

static int call_spanning_tree (int ncount, CCdatagroup *dat, double *wcoord,
        CCkdtree *kt, tabledat *td, int silent, CCrandstate *rstate)
{
    double szeit = CCutil_zeit ();
    int current = td->tabletotal;
    int *tree = (int *) NULL;
    double len;
    int norm;

    tree = CC_SAFE_MALLOC ((2 * ncount) - 2, int);
    if (!tree)
        return 1;

    CCutil_dat_getnorm (dat, &norm);
    if ((norm & CC_NORM_BITS) == CC_KD_NORM_TYPE) {
        if (CCkdtree_prim_spanningtree (kt, ncount, dat, wcoord, tree, &len,
                                        rstate)) {
            fprintf (stderr, "CCkdtree_prim_spanningtree failed\n");
            CC_FREE (tree, int);
            return 1;
        }
    } else if ((norm & CC_NORM_BITS) == CC_X_NORM_TYPE) {
        if (!silent) printf ("No X_NORM spanning tree\n");
        CC_FREE (tree, int);
        return 0;
    } else {
        if (!silent) printf ("No JUNK_NORM spanning tree\n");
        CC_FREE (tree, int);
        return 0;
    }

    if (put_list_in_table (td, ncount-1, tree)) {
        fprintf (stderr, "put_list_in_table failed\n");
        CC_FREE (tree, int);
        return 1;
    }

    if (!silent) {
        printf ("Spanning tree: %.0f, added %d edges (%.2f seconds)\n",
             len, td->tabletotal - current, CCutil_zeit () - szeit);
        fflush (stdout);
    }

    CC_IFFREE (tree, int);

    return 0;
}
#endif
