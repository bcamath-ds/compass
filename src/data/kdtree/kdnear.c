/****************************************************************************/
/*                                                                          */
/*  This file is part of CONCORDE                                           */
/*                                                                          */
/*  (c) Copyright 1995--1999 by David Applegate, Robert Bixby,              */
/*  Vasek Chvatal, and William Cook                                         */
/*                                                                          */
/*  Permission is granted for academic research use.  For other uses,       */
/*  contact the authors for licensing options.                              */
/*                                                                          */
/*  Use at your own risk.  We make no guarantees about the                  */
/*  correctness or usefulness of this code.                                 */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                 ROUTINES FOR FINDING NEAREST NEIGHBORS                   */
/*                                                                          */
/*  (Based on Jon Bentley's paper "K-d trees for semidynamic point sets")   */
/*                                                                          */
/*                           TSP CODE                                       */
/*                                                                          */
/*                                                                          */
/*  Written by:  Applegate, Bixby, Chvatal, and Cook                        */
/*  Date: February 24, 1995  (cofeb24)                                      */
/*  Changes: August 6 (bico)  -  added wcoord to fixed radius search        */
/*                                                                          */
/*                                                                          */
/*    EXPORTED FUNCTIONS:                                                   */
/*                                                                          */
/*  int CCkdtree_k_nearest (CCkdtree *kt, int ncount, int k,                */
/*      compass_data *dat, double *wcoord, int wantlist, int *ocount,        */
/*      int **olist, int silent, CCrandstate *rstate)                       */
/*    RETURNS the k-nearest neighbor graph.                                 */
/*      -kt can be NULL, otherwise it should point to a CCkdtree built      */
/*       by a call to kdbuild ()                                            */
/*      -ncount is the number of points.                                    */
/*      -k is the number of nearest neighbors wanted.                       */
/*      -wcoord is an array of node weights (like Held-Karp), it can        */
/*       be NULL. The weights should be nonnegative.                        */
/*      -wantlist is 1 if you want the function to return the edges.        */
/*      -ocount returns the number of edges (if wantlist is 1) and          */
/*       olist returns the edgelist is end1 end2 format.                    */
/*      -silent will turn off print messages if set to nonzero value.       */
/*                                                                          */
/*  int CCkdtree_quadrant_k_nearest (CCkdtree *kt, int ncount, int k,       */
/*      compass_data *dat, double *wcoord,                                   */
/*      int wantlist, int *ocount, int **olist, int silent,                 */
/*      CCrandstate *rstate)                                                */
/*    RETURNS the quadrant k-nearest neighbor graph.                        */
/*      -see CCkdtree_k_nearest.                                            */
/*                                                                          */
/*  int CCkdtree_node_k_nearest (CCkdtree *kt, int ncount, int n, int k,    */
/*      compass_data *dat, double *wcoord, int *list, CCrandstate *rstate)   */
/*    RETURNS the k nearest points to point n.                              */
/*      -The k points are return in list (and list must be allocated by     */
/*       calling routine.                                                   */
/*      -kt is a pointer to a CCkdtree previously built by                  */
/*       CCkdtree_build.                                                    */
/*                                                                          */
/*  int CCkdtree_node_quadrant_k_nearest (CCkdtree *kt, int ncount,         */
/*      int n, int k, compass_data *dat, double *wcoord, int *list,          */
/*      CCrandstate *rstate)                                                */
/*    RETURNS the quadrant k nearest point to point n.                      */
/*      -see CCkdtree_node_k_nearest.                                       */
/*                                                                          */
/*  int CCkdtree_node_nearest (ktree *kt, int n, compass_data *dat,          */
/*      double *wcoord)                                                     */
/*    RETURNS the nearest point to point n.                                 */
/*      -kt CANNOT be NULL.                                                 */
/*      -The point is returned as the function value. kt is a pointer       */
/*       to a CCkdtree (previously buildt by a call to CCkdtree_build)      */
/*                                                                          */
/*  int CCkdtree_fixed_radius_nearest (CCkdtree *kt, compass_data *dat,      */
/*      double *wcoord, int n, double rad,                                  */
/*      int (*doit_fn) (int, int, void *), void *pass_param)                */
/*    ACTION: Calls the function doit_fn (n, a, void *), where a ranges     */
/*            over all points within distance rad of the point n. The       */
/*            void * field can be used to bundle a group of parmeters       */
/*            into pass_param that will be passed to doit_fn.               */
/*      -kt CANNOT be NULL.                                                 */
/*      -doit_fn can also call CCkdtree_fixed_radius_nearest (no globals    */
/*       are set by the function calls)                                     */
/*      -pass_param can be NULL or used to point to a structure with        */
/*       with parameters for doit_fn.                                       */
/*                                                                          */
/*  int CCkdtree_nearest_neighbor_tour (CCkdtree *kt, int ncount,           */
/*      int start, compass_data *dat, int *outcycle, double *val,            */
/*      CCrandstate *rstate)                                                */
/*    -kt can be NULL.                                                      */
/*    -Node weights are not used.                                           */
/*    -start is the starting node for the tour.                             */
/*    -if outcycle is not NULL, then it should point to a array of          */
/*     length at least ncount (allocated by the calling routine). The       */
/*     cycle will be returned in the array in node node node format.        */
/*    -the length of the tour is return in val.                             */
/*                                                                          */
/*  int CCkdtree_nearest_neighbor_2match (CCkdtree *kt, int ncount,         */
/*      int start, compass_data *dat, int *outmatch, double *val,            */
/*      CCrandstate *rstate)                                                */
/*    -Like CCkdtree_nearest_neighbor_tour. If outmatch is not NULL         */
/*     then it should point to an array of length at least 2*ncount.        */
/*                                                                          */
/*    NOTES:                                                                */
/*       If memory is tight, use CCkdtree_node_k_nearest to get the         */
/*    edges one node at a time. (CCkdtree_k_nearest () builds a hash        */
/*    table to avoid duplicate edges, and it will use 8 * nedges            */
/*    bytes.)                                                               */
/*       CCkdtree_node_nearest returns the nearest point as the             */
/*    function value; CCkdtree_fixed_radius_nearest returns 1 if            */
/*    doit_fn returns a nonzero value, otherwise it returns 0; all          */
/*    other routines return 0 if successful and 1 otherwise.                */
/*                                                                          */
/****************************************************************************/


#include "compass.h"
#include "env.h"
#include "machdefs.h"
#include "data/kdtree/kdtree.h"
#include "macrorus.h"

#define BIGDOUBLE (1e30)
#define NEAR_HEAP_CUTOFF 100  /* When to switch from list to heap       */
                              /* On an RS6000, the heap started winning */
                              /* at 100 (by 200 it was 3 times faster)  */

typedef struct shortedge {
    int end;
    double length;
} shortedge;

typedef struct intptr {
    int this;
    struct intptr *next;
} intptr;

#define Fedgelen(n1, n2)                                                     \
    (datw != (double *) NULL ?                                               \
      CCutil_dat_edgelen ((n1), (n2), dat)                                   \
            + datw[(n1)] + datw[(n2)] :                                      \
      CCutil_dat_edgelen ((n1), (n2), dat))

#define dtrunc(x) (((x)>0.0)?floor(x):ceil(x))

static void
    node_k_nearest_work (CCkdtree *thetree, compass_data *dat, double *datw,
        CCkdnode *p, CCdheap *near_heap, int *heap_names, int *heap_count,
        int target, int num, shortedge *nearlist, double *worst_on_list,
        CCkdbnds *box),
    node_nearest_work (CCkdtree *thetree, compass_data *dat, double *datw,
         CCkdnode *p, int target, double *ndist, int *nnode);
static int
    run_kdtree_k_nearest (CCkdtree *kt, int ncount, int k, compass_data *dat,
        double *wcoord, int wantlist, int *ocount, int **olist, int doquad,
        int silent, CCrandstate *rstate),
    put_in_table (int i, int j, int *added, intptr **table,
        CCptrworld *intptr_world),
    q_run_it (CCkdtree *thetree, compass_data *dat, double *datw, int *llist,
         int *lcount, int *list, int target, int num, CCkdbnds *box),
    run_kdtree_node_k_nearest (CCkdtree *thetree, compass_data *dat,
         double *datw, int *list, int target, int num, CCkdbnds *box),
    ball_in_bounds (compass_data *dat, CCkdbnds *bnds, int n, double dist),
    fixed_radius_nearest_work (CCkdtree *thetree, CCkdnode *p,
         int (*doit_fn) (int, int, void *),
         int target, double dist, compass_data *dat, double *wcoord,
         double xtarget, double ytarget, void *pass_param);


CC_PTRWORLD_LIST_ROUTINES (intptr, int, intptralloc, intptr_bulk_alloc,
        intptrfree, intptr_listadd, intptr_listfree)
CC_PTRWORLD_LEAKS_ROUTINE (intptr, intptr_check_leaks, this, int)

int CCkdtree_k_nearest (CCkdtree *kt, int ncount, int k, compass_data *dat,
        double *wcoord, int wantlist, int *ocount, int **olist,
        int silent, CCrandstate *rstate)
{
    return run_kdtree_k_nearest (kt, ncount, k, dat, wcoord,
                                 wantlist, ocount, olist, 0, silent, rstate);
}

int CCkdtree_quadrant_k_nearest (CCkdtree *kt, int ncount, int k,
        compass_data *data, double *wcoord, int wantlist, int *ocount,
        int **olist, int silent, CCrandstate *rstate)
{
    return run_kdtree_k_nearest (kt, ncount, k, data, wcoord,
                                 wantlist, ocount, olist, 1, silent, rstate);
}


static int run_kdtree_k_nearest (CCkdtree *kt, int ncount, int k,
        compass_data *dat, double *wcoord, int wantlist, int *ocount,
        int **olist, int doquad, int silent, CCrandstate *rstate)
{
    int i, n;
    intptr *ip, *ipnext;
    int total, onlist;
    CCkdtree localkt, *mykt;
    int added, ntotal = 0;
    int rval = 0;
    int *list = (int *) NULL;
    int goal = (doquad ? (4 * k) : k);
    int newtree = 0;
    intptr **table = (intptr **) NULL;
    CCptrworld intptr_world;

    CCptrworld_init (&intptr_world);
    
    if (wcoord != (double *) NULL) {
        for (i = 0; i < ncount; i++) {
            if (wcoord[i] < -0.00000001) {
                fprintf (stderr, "Cannot CCkdtree with negative node weights\n");
                return 1;
            }
        }
    }

    if (wantlist) {
        *ocount = 0;
        *olist = (int *) NULL;
    }

#if 0
    if (kt == (CCkdtree *) NULL) {
        if (CCkdtree_build (&localkt, ncount, dat, wcoord, rstate)) {
            fprintf (stderr, "Unable to build CCkdtree\n");
            return 1;
        }
        mykt = &localkt;
        newtree = 1;
    } else {
        mykt = kt;
    }
#else
    mykt = kt;
#endif


    table = CC_SAFE_MALLOC (ncount, intptr *);
    if (!table) {
        rval = 1;
        goto CLEANUP;
    }
    for (i = 0; i < ncount; i++)
        table[i] = (intptr *) NULL;
    list = CC_SAFE_MALLOC (goal, int);
    if (!list) {
        rval = 1;
        goto CLEANUP;
    }

    for (n = 0; n < ncount; n++) {
        if (doquad) {
            if (CCkdtree_node_quadrant_k_nearest (mykt, ncount, n, k, dat,
                       wcoord, list, rstate)) {
                rval = 1;
                goto CLEANUP;
            }
        } else {
            if (CCkdtree_node_k_nearest (mykt, ncount, n, k, dat, wcoord,
                                         list, rstate)) {
                rval = 1;
                goto CLEANUP;
            }
        }
        for (i = 0; i < goal; i++) {
            if (list[i] != -1) {
                if (put_in_table (n, list[i], &added, table, &intptr_world))  {
                    rval = 1;
                    goto CLEANUP;
                } else {
                    ntotal += added;
                }
            }
        }
/*
        if (n == 0) {
            printf ("Neighbors of Node %d (%d, %d) :\n", n,
                                      (int) dat->x[n], (int) dat->y[n]);
            for (i = 0; i < goal; i++) {
                if (list[i] != -1) {
                    printf ("%d  %d (%d, %d)\n", list[i],
                      CCutil_dat_edgelen (n, list[i], dat),
                      (int) dat->x[list[i]], (int) dat->y[list[i]]);
                }
            }
        }
*/
        if (!silent) {
            if (n % 1000 == 999) {
                printf ("."); fflush (stdout);
            }
        }
    }
  
    if (!silent) {
        printf (" %d edges\n", ntotal); fflush (stdout);
    }

    if (wantlist) {
        int j = 0;
        *olist = xcalloc (2 * ntotal, sizeof(int));
        if (!(*olist)) {
            rval = 1;
            goto CLEANUP;
        }
        *ocount = ntotal;
        for (i = 0; i < ncount; i++) {
            for (ip = table[i]; ip; ip = ipnext) {
                ipnext =  ip->next;
                (*olist)[j++] = i;
                (*olist)[j++] = ip->this;
                intptrfree (&intptr_world, ip);
            }
            table[i] = (intptr *) NULL;
        }
    } else {
        for (i = 0; i < ncount; i++) {
            intptr_listfree (&intptr_world, table[i]);
            table[i] = (intptr *) NULL;
        }
    }
    if (intptr_check_leaks (&intptr_world, &total, &onlist)) {
        fprintf (stderr, "WARNING: %d outstanding intptrs in kdnear\n",
                 total - onlist);
    }

CLEANUP:

    CCptrworld_delete (&intptr_world);
    if (table)
        CC_FREE(table, intptr *);
    if (list)
        CC_FREE (list, int);
    if (newtree)
        CCkdtree_free (&localkt);

    return rval;
}

static int put_in_table (int i, int j, int *added, intptr **table,
        CCptrworld *intptr_world)
{
    intptr *ip;

    if (j < i) {
        int temp;
        CC_SWAP(i, j, temp);
    }

    for (ip = table[i]; ip; ip = ip->next) {
        if (ip->this == j) {
            *added = 0;
            return 0;
        }
    }
    if (intptr_listadd (&table[i], j, intptr_world)) {
        *added = 0;
        return 1;
    }
    *added = 1;
    return 0;
}

int CCkdtree_node_quadrant_k_nearest (CCkdtree *kt, int ncount, int n, int k,
            compass_data *dat, double *wcoord, int *list, CCrandstate *rstate)
{
    CCkdbnds localbnds;
    int i, lcount = 0;
    int *llist = (int *) NULL;
    int rval = 0;
    CCkdtree localkt;
    CCkdtree *thetree;
    int newtree = 0;

#if 0
    if (kt == (CCkdtree *) NULL) {
        if (CCkdtree_build (&localkt, ncount, dat, wcoord, rstate)) {
            fprintf (stderr, "Unable to build CCkdtree\n");
            return 1;
        }
        thetree = &localkt;
        newtree = 1;
    } else {
        thetree = kt;
    }
#else
    thetree = kt;
#endif

    llist = CC_SAFE_MALLOC (k, int);
    if (!llist) {
        rval = 1;
        goto CLEANUP;
    }

    localbnds.x[0] = dat->x[n];
    localbnds.x[1] = BIGDOUBLE;
    localbnds.y[0] = dat->y[n];
    localbnds.y[1] = BIGDOUBLE;
    if (q_run_it (thetree, dat, wcoord, llist, &lcount, list, n, k,
                  &localbnds)) {
        fprintf (stderr, "run_kdtree_node_k_nearest failed\n");
        rval = 1;
        goto CLEANUP;
    }

    localbnds.x[0] = dat->x[n];
    localbnds.x[1] = BIGDOUBLE;
    localbnds.y[0] = -BIGDOUBLE;
    localbnds.y[1] = dat->y[n];
    if (q_run_it (thetree, dat, wcoord, llist, &lcount, list, n, k,
                  &localbnds)) {
        fprintf (stderr, "run_kdtree_node_k_nearest failed\n");
        rval = 1;
        goto CLEANUP;
    }

    localbnds.x[0] = -BIGDOUBLE;
    localbnds.x[1] = dat->x[n];
    localbnds.y[0] = -BIGDOUBLE;
    localbnds.y[1] = dat->y[n];
    if (q_run_it (thetree, dat, wcoord, llist, &lcount, list, n, k,
                  &localbnds)) {
        fprintf (stderr, "run_kdtree_node_k_nearest failed\n");
        rval = 1;
        goto CLEANUP;
    }

    localbnds.x[0] = -BIGDOUBLE;
    localbnds.x[1] = dat->x[n];
    localbnds.y[0] = dat->y[n];
    localbnds.y[1] = BIGDOUBLE;
    if (q_run_it (thetree, dat, wcoord, llist, &lcount, list, n, k,
                  &localbnds)) {
        fprintf (stderr, "run_kdtree_node_k_nearest failed\n");
        rval = 1;
        goto CLEANUP;
    }

    for (i = lcount; i < (4 * k); i++)
        list[i] = -1;

CLEANUP:

    CC_FREE (llist, int);
    if (newtree)
        CCkdtree_free (&localkt);

    return rval;
}

static int q_run_it (CCkdtree *thetree, compass_data *dat, double *datw,
        int *llist, int *lcount, int *list, int target, int num, CCkdbnds *box)
{
    int i, j;

    if (run_kdtree_node_k_nearest (thetree, dat, datw, llist, target, num,
                                   box))
        return 1;
    for (i = 0; i < num; i++) {
        if (llist[i] != -1) {
            for (j = 0; j < *lcount; j++)
                if (list[j] == llist[i])
                    break;
            if (j == *lcount)
                list[(*lcount)++] = llist[i];
        }
    }
    return 0;
}

int CCkdtree_node_k_nearest (CCkdtree *kt, int ncount, int n, int k,
        compass_data *dat, double *wcoord, int *list, CCrandstate *rstate)
{
    CCkdtree localkt;
    CCkdtree *thetree;
    int newtree = 0;
    int rval = 0;

#if 0
    if (kt == (CCkdtree *) NULL) {
        if (CCkdtree_build (&localkt, ncount, dat, wcoord, rstate)) {
            fprintf (stderr, "Unable to build CCkdtree\n");
            return 1;
        }
        thetree = &localkt;
        newtree = 1;
    } else {
        thetree = kt;
    }
#else
    thetree = kt;
#endif

    rval = run_kdtree_node_k_nearest (thetree, dat, wcoord, list, n, k,
                                      (CCkdbnds *) NULL);
    if (newtree)
        CCkdtree_free (&localkt);
    return rval;
}

static int run_kdtree_node_k_nearest (CCkdtree *thetree, compass_data *dat,
        double *datw, int *list, int target, int num, CCkdbnds *box)
{
    int i;
    CCkdnode *p, *lastp;
    double diff;
    CCdheap near_heap;
    int *heap_names =  (int *) NULL;
    int heap_count = 0;
    shortedge *nearlist = (shortedge *) NULL;
    double worst_on_list = BIGDOUBLE;

    if (num >= NEAR_HEAP_CUTOFF) {
        if (CCutil_dheap_init (&near_heap, num))
            return 1;
        heap_names = CC_SAFE_MALLOC (num, int);
        if (!heap_names) {
            CCutil_dheap_free (&near_heap);
            return 1;
        }
        heap_count = 0;
    } else {
        nearlist = CC_SAFE_MALLOC (num + 1, shortedge);
        if (!nearlist) {
            CCutil_dheap_free (&near_heap);
            CC_FREE (heap_names, int);
            return 1;
        }
        for (i = 0; i < num; i++)
            nearlist[i].length = BIGDOUBLE;
        nearlist[num].length = -BIGDOUBLE;
    }

/*
    To do top down search just use:

        node_k_nearest_work (thetree->root);
*/

    p = thetree->bucketptr[target];
    node_k_nearest_work (thetree, dat, datw, p, &near_heap, heap_names,
                         &heap_count, target, num, nearlist, &worst_on_list,
                         box);
    while (1) {
        lastp = p;
        p = p->father;
        if (p == (CCkdnode *) NULL)
            break;
        switch (p->cutdim) {
        case 0:
            diff = p->cutval - dat->x[target];
            if (lastp == p->loson) {    /* So target is on low side */
               if (worst_on_list > dtrunc(diff))
                   if (box == (CCkdbnds *) NULL || p->cutval <= box->x[1])
                       node_k_nearest_work (thetree, dat, datw, p->hison,
                              &near_heap, heap_names, &heap_count, target,
                              num, nearlist, &worst_on_list, box);
            } else {
               if (worst_on_list > dtrunc(-diff))
                   if (box == (CCkdbnds *) NULL || p->cutval >= box->x[0])
                       node_k_nearest_work (thetree, dat, datw, p->loson,
                              &near_heap, heap_names, &heap_count, target,
                              num, nearlist, &worst_on_list, box);
            }
            break;
        case 1:
            diff = p->cutval - dat->y[target];
            if (lastp == p->loson) {
               if (worst_on_list > dtrunc(diff))
                   if (box == (CCkdbnds *) NULL || p->cutval <= box->y[1])
                       node_k_nearest_work (thetree, dat, datw, p->hison,
                              &near_heap, heap_names, &heap_count, target,
                              num, nearlist, &worst_on_list, box);
            } else {
               if (worst_on_list > dtrunc(-diff))
                   if (box == (CCkdbnds *) NULL || p->cutval >= box->y[0])
                       node_k_nearest_work (thetree, dat, datw, p->loson,
                              &near_heap, heap_names, &heap_count, target,
                              num, nearlist, &worst_on_list, box);
            }
            break;
        case 2:
            if (lastp == p->loson) {
                if (worst_on_list > p->cutval + datw[target])
                    node_k_nearest_work (thetree, dat, datw, p->hison,
                              &near_heap, heap_names, &heap_count, target,
                              num, nearlist, &worst_on_list, box);
            } else {
                node_k_nearest_work (thetree, dat, datw, p->loson, &near_heap,
                              heap_names, &heap_count, target, num, nearlist,
                              &worst_on_list, box);
            }
            break;
        }
        if (datw == (double *) NULL && p->bnds &&
               ball_in_bounds (dat, p->bnds, target, worst_on_list))
              /* Doing extra check for box with quad-nearest appears to slow */
              /* things down.                                                */
            break;
    }

    if (num >= NEAR_HEAP_CUTOFF) {
        if (heap_count < num) {
            if (box == (CCkdbnds *) NULL) {
                fprintf (stderr, "WARNING: There do not exist %d neighbors\n",
                         num);
            }
            for (i = 0; i < heap_count; i++) {
                list[i] = heap_names[i];
            }
            for (; i < num; i++)
                list[i] = -1;
        } else {
            for (i = 0; i < num; i++)
                list[i] = heap_names[i];
        }
    } else {
        int ntot = 0;
        for (i = 0; i < num; i++) {
            if (nearlist[i].length < BIGDOUBLE)
                list[ntot++] = nearlist[i].end;
        }
        if (ntot < num) {
            if (box == (CCkdbnds *) NULL) {
                fprintf (stderr, "WARNING: There do not exist %d neighbors\n",
                         num);
            }
            for (i = ntot; i < num; i++)
                list[i] = -1;
        }
    }

    if (num >= NEAR_HEAP_CUTOFF) {
        CC_FREE (heap_names, int);
        CCutil_dheap_free (&near_heap);
    } else {
        CC_FREE (nearlist, shortedge);
    }
    return 0;
}

static void node_k_nearest_work (CCkdtree *thetree, compass_data *dat,
        double *datw, CCkdnode *p, CCdheap *near_heap, int *heap_names,
        int *heap_count, int target, int num, shortedge *nearlist,
        double *worst_on_list, CCkdbnds *box)
{
    int i, h, k;
    double val, thisx, thisdist;

    if (p->bucket) {
        if (num >= NEAR_HEAP_CUTOFF) {
            for (i = p->lopt; i <= p->hipt; i++) {
                if (thetree->perm[i] != target) {
                    if (box == (CCkdbnds *) NULL ||
                       (dat->x[thetree->perm[i]] >= box->x[0] &&
                        dat->x[thetree->perm[i]] <= box->x[1] &&
                        dat->y[thetree->perm[i]] >= box->y[0] &&
                        dat->y[thetree->perm[i]] <= box->y[1])) {
                        thisdist = Fedgelen (thetree->perm[i], target);
                        if (*heap_count < num) {
                            near_heap->key[*heap_count] = -thisdist;
                            heap_names[*heap_count] = thetree->perm[i];
                            /* this can't fail since the heap is big enough */
                            CCutil_dheap_insert (near_heap, *heap_count);
                            (*heap_count)++;
                        } else if (*worst_on_list > thisdist) {
                            h = CCutil_dheap_deletemin (near_heap);
                            heap_names[h] = thetree->perm[i];
                            near_heap->key[h] = -thisdist;
                            /* this can't fail since the heap is big enough */
                            CCutil_dheap_insert (near_heap, h);
                            h = CCutil_dheap_findmin (near_heap);
                            *worst_on_list = -near_heap->key[h];
                        }
                    }
                }
            }
        } else {
            for (i = p->lopt; i <= p->hipt; i++) {
                if (thetree->perm[i] != target) {
                    if (box == (CCkdbnds *) NULL ||
                       (dat->x[thetree->perm[i]] >= box->x[0] &&
                        dat->x[thetree->perm[i]] <= box->x[1] &&
                        dat->y[thetree->perm[i]] >= box->y[0] &&
                        dat->y[thetree->perm[i]] <= box->y[1])) {
                        thisdist = Fedgelen (thetree->perm[i], target);
                        if (*worst_on_list > thisdist) {
                            for (k = 0; nearlist[k+1].length > thisdist; k++) {
                                nearlist[k].end = nearlist[k + 1].end;
                                nearlist[k].length = nearlist[k + 1].length;
                            }
                            nearlist[k].length = thisdist;
                            nearlist[k].end = thetree->perm[i];
                            *worst_on_list = nearlist[0].length;
                        }
                    }
                }
            }
        }
    } else {
        val = p->cutval;
        switch (p->cutdim) {
        case 0:
            thisx = dat->x[target];
            if (thisx < val) {
                node_k_nearest_work (thetree, dat, datw, p->loson, near_heap,
                        heap_names, heap_count, target, num, nearlist,
                        worst_on_list, box);
                /* Truncation for floating point coords */
                if (*worst_on_list > dtrunc(val - thisx))
                    if (box == (CCkdbnds *) NULL || val >= box->x[0])
                        node_k_nearest_work (thetree, dat, datw, p->hison,
                               near_heap, heap_names, heap_count, target,
                               num, nearlist, worst_on_list, box);
            } else {
                node_k_nearest_work (thetree, dat, datw, p->hison, near_heap,
                               heap_names, heap_count, target, num, nearlist,
                               worst_on_list, box);
                if (*worst_on_list > dtrunc(thisx - val))
                    if (box == (CCkdbnds *) NULL || val <= box->x[1])
                        node_k_nearest_work (thetree, dat, datw, p->loson,
                               near_heap, heap_names, heap_count, target,
                               num, nearlist, worst_on_list, box);
            }
            break;
        case 1:
            thisx = dat->y[target];
            if (thisx < val) {
                node_k_nearest_work (thetree, dat, datw, p->loson, near_heap,
                               heap_names, heap_count, target, num, nearlist,
                               worst_on_list, box);
                if (*worst_on_list > dtrunc(val - thisx))
                    if (box == (CCkdbnds *) NULL || val >= box->y[0])
                        node_k_nearest_work (thetree, dat, datw, p->hison,
                               near_heap, heap_names, heap_count, target,
                               num, nearlist, worst_on_list, box);
            } else {
                node_k_nearest_work (thetree, dat, datw, p->hison, near_heap,
                               heap_names, heap_count, target, num, nearlist,
                               worst_on_list, box);
                if (*worst_on_list > dtrunc(thisx - val))
                    if (box == (CCkdbnds *) NULL || val <= box->y[1])
                        node_k_nearest_work (thetree, dat, datw, p->loson,
                               near_heap, heap_names, heap_count, target,
                               num, nearlist, worst_on_list, box);
            }
            break;
        case 2:
            thisx = datw[target];
            node_k_nearest_work (thetree, dat, datw, p->loson, near_heap,
                               heap_names, heap_count, target, num, nearlist,
                               worst_on_list, box);
            if (*worst_on_list > val + thisx)
                node_k_nearest_work (thetree, dat, datw, p->hison, near_heap,
                               heap_names, heap_count, target, num, nearlist,
                               worst_on_list, box);
            break;
        }
    }
}


int CCkdtree_node_nearest (CCkdtree *kt, int n, compass_data *dat,
        double *wcoord)
{
    CCkdnode *p, *lastp;
    double diff;
    double ndist = BIGDOUBLE;
    int nnode;
    CCkdtree *thetree = (CCkdtree *) NULL;

    if (kt == (CCkdtree *) NULL) {
        fprintf (stderr, "ERROR: kt cannot be NULL in CCkdtree_node_nearest)\n");
        return n;
    }

    thetree = kt;

    ndist = BIGDOUBLE;
    nnode = n;

/*
    To do top down search just use:

        node_nearest_work (kt->root);
        return nnode;
*/

    p = kt->bucketptr[n];
    node_nearest_work (thetree, dat, wcoord, p, n, &ndist, &nnode);
    while (1) {
        lastp = p;
        p = p->father;
        if (p == (CCkdnode *) NULL)
            break;
        switch (p->cutdim) {
        case 0:
            diff = p->cutval - dat->x[n];
            if (lastp == p->loson) {
                if (ndist > dtrunc(diff))
                   node_nearest_work (thetree, dat, wcoord, p->hison, n,
                                      &ndist, &nnode);
            } else {
                if (ndist > dtrunc(-diff))
                   node_nearest_work (thetree, dat, wcoord, p->loson, n,
                                      &ndist, &nnode);
            }
            break;
        case 1:
            diff = p->cutval - dat->y[n];
            if (lastp == p->loson) {
                if (ndist > dtrunc(diff))
                   node_nearest_work (thetree, dat, wcoord, p->hison, n,
                                      &ndist, &nnode);
            } else {
                if (ndist > dtrunc(-diff))
                   node_nearest_work (thetree, dat, wcoord, p->loson, n,
                                      &ndist, &nnode);
            }
            break;
        case 2:
            if (lastp == p->loson) {
                if (ndist > p->cutval + wcoord[n])
                    node_nearest_work (thetree, dat, wcoord, p->hison, n,
                                      &ndist, &nnode);
            } else {
                node_nearest_work (thetree, dat, wcoord, p->loson, n,
                                   &ndist, &nnode);
            }
            break;
        }
        if (wcoord == (double *) NULL && p->bnds &&
               ball_in_bounds (dat, p->bnds, n, ndist))
            break;
    }
    return nnode;
}

static int ball_in_bounds (compass_data *dat, CCkdbnds *bnds, int n,
        double dist)
{
    if (dtrunc(dat->x[n] - bnds->x[0]) < dist ||
        dtrunc(bnds->x[1] - dat->x[n]) < dist ||
        dtrunc(dat->y[n] - bnds->y[0]) < dist ||
        dtrunc(bnds->y[1] - dat->y[n]) < dist)
        return 0;
    return 1;
}

static void node_nearest_work (CCkdtree *thetree, compass_data *dat,
        double *datw, CCkdnode *p, int target, double *ndist, int *nnode)
{
    int i;
    double val, thisx, thisdist;

    if (!p->empty) {
        if (p->bucket) {
            for (i = p->lopt; i <= p->hipt; i++) {
                if (thetree->perm[i] != target) {
                    thisdist = Fedgelen (thetree->perm[i], target);
                    if (*ndist > thisdist) {
                        *ndist = thisdist;
                        *nnode = thetree->perm[i];
                    }
                }
            }
        } else {
            val = p->cutval;
            switch (p->cutdim) {
            case 0:
                thisx = dat->x[target];
                if (thisx < val) {
                    node_nearest_work (thetree, dat, datw, p->loson, target,
                                       ndist, nnode);
                    if (*ndist >  dtrunc(val - thisx))
                        node_nearest_work (thetree, dat, datw, p->hison,
                                       target, ndist, nnode);
                } else {
                    node_nearest_work (thetree, dat, datw, p->hison, target,
                                       ndist, nnode);
                    if (*ndist > dtrunc(thisx - val))
                        node_nearest_work (thetree, dat, datw, p->loson,
                                       target, ndist, nnode);
                }
                break;
            case 1:
                thisx = dat->y[target];
                if (thisx < val) {
                    node_nearest_work (thetree, dat, datw, p->loson, target,
                                       ndist, nnode);
                    if (*ndist >  dtrunc(val - thisx))
                        node_nearest_work (thetree, dat, datw, p->hison,
                                       target, ndist, nnode);
                } else {
                    node_nearest_work (thetree, dat, datw, p->hison, target,
                                       ndist, nnode);
                    if (*ndist > dtrunc(thisx - val))
                        node_nearest_work (thetree, dat, datw, p->loson,
                                       target, ndist, nnode);
                }
                break;
            case 2:
                thisx = datw[target];
                node_nearest_work (thetree, dat, datw, p->loson, target, ndist,
                                       nnode);
                if (*ndist > val + thisx)
                    node_nearest_work (thetree, dat, datw, p->hison, target,
                                       ndist, nnode);
                break;
            }
        }
    }
}

int CCkdtree_fixed_radius_nearest (CCkdtree *kt, compass_data *dat,
        double *datw, int n, double rad, int (*doit_fn) (int, int, void *),
        void *pass_param)
{
    CCkdnode *p, *lastp;
    double dist, diff, xtarget, ytarget;
    int target;

    if (kt == (CCkdtree *) NULL) {
        fprintf (stderr, "ERROR: CCkdtree_fixed_radius_nearest needs a CCkdtree\n");
        return 0;
    }

    dist = (double) rad;
    target = n;
    xtarget = dat->x[target];
    ytarget = dat->y[target];

    p = kt->bucketptr[target];
    if (fixed_radius_nearest_work (kt, p, doit_fn, target, dist, dat, datw,
                                   xtarget, ytarget, pass_param))
        return 1;

    if (datw) {
        double wdist = dist - datw[target];
        while (1) {
            lastp = p;
            p = p->father;
            if (p == (CCkdnode *) NULL)
                return 0;
            if (p->cutdim == 0)
                diff = p->cutval - xtarget;
            else if (p->cutdim == 1)
                diff = p->cutval - ytarget;
            else
                diff = p->cutval;
            if (lastp == p->loson) {
                if (wdist > dtrunc(diff)) {
                    if (fixed_radius_nearest_work (kt, p->hison, doit_fn,
                              target, dist, dat, datw, xtarget, ytarget,
                              pass_param))
                        return 1;
                }
            } else {
                if (wdist > dtrunc(-diff)) {
                    if (fixed_radius_nearest_work (kt, p->loson, doit_fn,
                              target, dist, dat, datw, xtarget, ytarget,
                              pass_param))
                        return 1;
                }
            }
            if (p->bnds &&  /* ball_in_bounds */
              !(dtrunc(xtarget - p->bnds->x[0]) < wdist ||
                dtrunc(p->bnds->x[1] - xtarget) < wdist ||
                dtrunc(ytarget - p->bnds->y[0]) < wdist ||
                dtrunc(p->bnds->y[1] - ytarget) < wdist))
                return 0;
        }
    } else {
        while (1) {
            lastp = p;
            p = p->father;
            if (p == (CCkdnode *) NULL)
                return 0;
            if (p->cutdim == 0)
                diff = p->cutval - xtarget;
            else
                diff = p->cutval - ytarget;

            if (lastp == p->loson) {
                if (dist > dtrunc(diff)) {
                    if (fixed_radius_nearest_work (kt, p->hison, doit_fn,
                              target, dist, dat, datw, xtarget, ytarget,
                              pass_param))
                        return 1;
                }
            } else {
                if (dist > dtrunc(-diff) || p->cutdim == 2) {
                    if (fixed_radius_nearest_work (kt, p->loson, doit_fn,
                              target, dist, dat, datw, xtarget, ytarget,
                              pass_param))
                        return 1;
                }
            }
            if (p->bnds &&  /* ball_in_bounds */
                !(dtrunc(xtarget - p->bnds->x[0]) < dist ||
                  dtrunc(p->bnds->x[1] - xtarget) < dist ||
                  dtrunc(ytarget - p->bnds->y[0]) < dist ||
                  dtrunc(p->bnds->y[1] - ytarget) < dist))
                return 0;
        }
    }
}

static int fixed_radius_nearest_work (CCkdtree *thetree, CCkdnode *p,
            int (*doit_fn) (int, int, void *), int target, double dist,
            compass_data *dat, double *datw,  double xtarget, double ytarget,
            void *pass_param)
{
    int i, c;
    double val, thisx, thisdist;

    if (p->empty)
        return 0;

    if (p->bucket) {
        for (i = p->lopt; i <= p->hipt; i++) {
            c = thetree->perm[i];
            if (c != target) {
                thisdist = Fedgelen (c, target);
                if (thisdist < dist) {
                    if (doit_fn (target, c, pass_param)) {
                        return 1;
                    }
                }
            }
        }
        return 0;
    } else {
        if (datw) {
            double wdist = dist - datw[target];

            val = p->cutval;
            switch (p->cutdim) {
            case 0:
                thisx = xtarget;
                break;
            case 1:
                thisx = ytarget;
                break;
            case 2:
                if (fixed_radius_nearest_work (thetree, p->loson, doit_fn,
                     target, dist, dat, datw, xtarget, ytarget, pass_param)) {
                    return 1;
                }
                if (p->cutval <= wdist) {
                    if (fixed_radius_nearest_work (thetree, p->hison, doit_fn,
                         target, dist, dat, datw, xtarget, ytarget,
                         pass_param)) {
                        return 1;
                    }
                }
                return 0;
            default:
                return 0;
            }
            if (thisx < val) {
                if (fixed_radius_nearest_work (thetree, p->loson, doit_fn,
                        target, dist, dat, datw, xtarget, ytarget,
                        pass_param)) {
                    return 1;
                }
                if (wdist > dtrunc(val - thisx)) {
                    if (fixed_radius_nearest_work (thetree, p->hison, doit_fn,
                            target, dist, dat, datw, xtarget, ytarget,
                            pass_param)) {
                        return 1;
                    }
                }
            } else {
                if (fixed_radius_nearest_work (thetree, p->hison, doit_fn,
                        target, dist, dat, datw, xtarget, ytarget,
                        pass_param)) {
                    return 1;
                }
                if (wdist > dtrunc(thisx - val)) {
                    if (fixed_radius_nearest_work (thetree, p->loson, doit_fn,
                            target, dist, dat, datw, xtarget, ytarget,
                            pass_param)) {
                        return 1;
                    }
                }
            }
        } else {
            val = p->cutval;
            switch (p->cutdim) {
            case 0:
                thisx = xtarget;
                break;
            case 1:
                thisx = ytarget;
                break;
            case 2:
            default:
                fprintf (stderr, "ERROR: split on w without node weights\n");
                return 0;
            }
            if (thisx < val) {
                if (fixed_radius_nearest_work (thetree, p->loson, doit_fn,
                        target, dist, dat, datw, xtarget, ytarget,
                        pass_param)) {
                    return 1;
                }
                if (dist > dtrunc(val - thisx)) {
                    if (fixed_radius_nearest_work (thetree, p->hison, doit_fn,
                            target, dist, dat, datw, xtarget, ytarget,
                            pass_param)) {
                        return 1;
                    }
                }
            } else {
                if (fixed_radius_nearest_work (thetree, p->hison, doit_fn,
                        target, dist, dat, datw, xtarget, ytarget,
                        pass_param)) {
                    return 1;
                }
                if (dist > dtrunc(thisx - val)) {
                    if (fixed_radius_nearest_work (thetree, p->loson, doit_fn,
                            target, dist, dat, datw, xtarget, ytarget,
                            pass_param)) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int CCkdtree_nearest_neighbor_tour (CCkdtree *kt, int ncount, int start,
        compass_data *dat, int *outcycle, double *val, CCrandstate *rstate)
{
    double len;
    int i, current, next;
    CCkdtree localkt, *mykt;
    int newtree = 0;

    if (ncount < 3) {
        fprintf (stderr, "Cannot find tour in an %d node graph\n", ncount);
        return 1;
    }

    if (kt == (CCkdtree *) NULL) {
        if (CCkdtree_build (&localkt, ncount, dat, (double *) NULL, rstate)) {
            fprintf (stderr, "Unable to build CCkdtree\n");
            return 1;
        }
        mykt = &localkt;
        newtree = 1;
    } else {
        mykt = kt;
    }

    /*
        printf ("Grow nearest neighbor tour from node %d\n", start);
        fflush (stdout);
    */

    len = 0.0;
    current = start;
    if (outcycle != (int *) NULL)
        outcycle[0] = start;
    for (i = 1; i < ncount; i++) {
        CCkdtree_delete (mykt, current);
        next = CCkdtree_node_nearest (mykt, current, dat, (double *) NULL);
        if (outcycle != (int *) NULL)
            outcycle [i] = next;
        len += (double) CCutil_dat_edgelen (current, next, dat);
        current = next;
    }
    len += (double) CCutil_dat_edgelen (current, start, dat);
    *val = len;
    if (newtree)
        CCkdtree_free (&localkt);
    else
        CCkdtree_undelete_all (kt, ncount);
    return 0;
}

int CCkdtree_nearest_neighbor_2match (CCkdtree *kt, int ncount, int start,
         compass_data *dat, int *outmatch, double *val, CCrandstate *rstate)
{
    double len;
    int i, j, cur, next;
    CCkdtree localkt, *mykt;
    double szeit;
    int count = 0, cyccount = 0;
    char *mark = (char *) NULL;
    int newtree = 0;
    int rval = 0;

    if (ncount < 3) {
        fprintf (stderr, "Cannot find 2-matching in an %d node graph\n",
                 ncount);
        return 1;
    }

    if (kt == (CCkdtree *) NULL) {
        if (CCkdtree_build (&localkt, ncount, dat, (double *) NULL, rstate)) {
            fprintf (stderr, "Unable to build CCkdtree\n");
            return 1;
        }
        mykt = &localkt;
        newtree = 1;
    } else {
        mykt = kt;
    }

    mark = CC_SAFE_MALLOC (ncount, char);
    if (!mark) {
        rval = 1;
        goto CLEANUP;
    }
    for (i = 0 ; i < ncount; i++)
        mark[i] = 0;

    printf ("Grow nearest neighbor 2-matching from node %d\n", start);
    fflush (stdout);
    szeit = CCutil_zeit ();
    len = 0.0;

    while (count < ncount) {
        for (j = start; j < ncount && mark[j]; j++);
        if (j == ncount) {
            for (j = 0; j < start && mark[j]; j++);
            if (j == start) {
                fprintf (stderr, "ERROR in near-2match\n");
                rval = 1;
                goto CLEANUP;
            }
        }
        start = j;
        mark[start] = 1;
        CCkdtree_delete (mykt, start);
        next = CCkdtree_node_nearest (mykt, start, dat, (double *) NULL);
        mark[next] = 1;
        len += (double) CCutil_dat_edgelen (start, next, dat);
        if (outmatch != (int *) NULL) {
            outmatch[2 * count] = start;
            outmatch[(2 * count) + 1] = next;
        }
        count++;
        CCkdtree_delete (mykt, next);
        cur = CCkdtree_node_nearest (mykt, next, dat, (double *) NULL);
        len += (double) CCutil_dat_edgelen (next, cur, dat);
        if (outmatch != (int *) NULL) {
            outmatch[2 * count] = next;
            outmatch[(2 * count) + 1] = cur;
        }
        count++;
        CCkdtree_undelete (mykt, start);
        while (cur != start && count < ncount - 3) {
            mark[cur] = 1;
            CCkdtree_delete (mykt, cur);
            next = CCkdtree_node_nearest (mykt, cur, dat, (double *) NULL);
            len += (double) CCutil_dat_edgelen (cur, next, dat);
            if (outmatch != (int *) NULL) {
                outmatch[2 * count] = cur;
                outmatch[(2 * count) + 1] = next;
            }
            count++;
            cur = next;
        }
        CCkdtree_delete (mykt, start);

        if (cur != start) {   /* Not enough nodes for another circuit */
            while (count < ncount - 1) {
                mark[cur] = 1;
                CCkdtree_delete (mykt, cur);
                next = CCkdtree_node_nearest (mykt, cur, dat, (double *) NULL);
                len += (double) CCutil_dat_edgelen (cur, next, dat);
                if (outmatch != (int *) NULL) {
                    outmatch[2 * count] = cur;
                    outmatch[(2 * count) + 1] = next;
                }
                count++;
                cur = next;
            }
            len += (double) CCutil_dat_edgelen (cur, start, dat);
            if (outmatch != (int *) NULL) {
                outmatch[2 * count] = cur;
                outmatch[(2 * count) + 1] = start;
            }
            count++;
        }
        cyccount++;
    }

    *val = len;
    printf ("%d cycles in 2-matching\n", cyccount);
    printf ("Running time for Nearest Neighbor 2-match: %.2f\n",
                                                  CCutil_zeit () - szeit);
    fflush (stdout);

CLEANUP:

    if (newtree)
        CCkdtree_free (&localkt);
    else
        CCkdtree_undelete_all (kt, ncount);
    if (mark)
        CC_FREE (mark, char);
    return rval;
}
