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
#include "data/kdtree/kdtree.h"
#include <gsl/gsl_vector.h>

#define BIGDOUBLE (1e30)
#define Edgelen(data, n1, n2)  CCutil_dat_edgelen (n1, n2, data)

typedef struct neighbour {
    int this;
    int *node;
} neighbour;

typedef struct position {
    int prev;
    int next;
    double cost;
} position;

static void
  get_node_3_nearest (CCkdtree *kt, int ncount, compass_data *data, int scount, int node, int *selected,
      struct neighbour *neighbour, int first, CCrandstate *rstate),
  get_best_position (compass_data *data, int scount, int *genotype, int node, struct neighbour *neighbour,
    struct position *pos);

int OPadd_operator (CCkdtree *kt, int ncount, compass_data *data,
    int *scount, int *selected, int *sposition, int *cycle, int *genotype, double *val,
    double cost_limit, CCrandstate *rstate)
{
    int rval;
    int i, j, prev, next;
    int nodesel, nodeprev, nodenext;
    int first, completed;
    double len, cost, best;
    struct neighbour *neighbour = (struct neighbour *) NULL;
    struct position *pos = (struct position *) NULL;

    neighbour = xcalloc (ncount, sizeof(struct neighbour));

    for (i = 1; i < ncount; i++) {
      if (!selected[i]) {
        struct neighbour *nodeneigh = &(neighbour[i]);
        nodeneigh->this=i;
        nodeneigh->node = xcalloc(4, sizeof(int));
      }
    }

    if (ncount < 3) {
        fprintf (stderr, "Cannot find tour in an %d node graph\n", ncount);
        return 1;
    }

  if (kt->root != (CCkdtree *) NULL) {

      CCkdtree_undelete_all (kt, ncount);
      for (i = 0; i < ncount; i++) {
        if (!selected[i])
          CCkdtree_delete (kt, i);
      }
    }

    len = *val;
    completed = 0;
    first = 1;

    pos = xcalloc(1, sizeof(struct position));

    do {

      best = BIGDOUBLE;
      for (i = 1; i < ncount; i++) {
        if (!selected[i]) {
          struct neighbour *nodeneigh = &neighbour[i];
          get_node_3_nearest (kt, ncount, data, *scount, i, selected, nodeneigh, first, rstate);
          get_best_position (data, *scount, genotype, i, nodeneigh, pos);

          if (pos->cost < best) {
            nodesel   = i;
            nodeprev  = pos->prev;
            nodenext  = pos->next;
            best = pos->cost;
          }
        }
      }

      if ( len + best < cost_limit ) {
        genotype[nodeprev] = nodesel;
        genotype[nodesel] = nodenext;
        selected[nodesel] = 1;
        *scount +=1;

        if (kt->root != (CCkdtree *) NULL)
          CCkdtree_undelete (kt, nodesel);

        for (i=0; i<ncount; i++)
        { if (!selected[i])
          { struct neighbour *nodeneigh = &(neighbour[i]);
            nodeneigh->node[3] = nodesel;
          }
        }

        len += best;
        first = 0;
        struct neighbour *nodeneigh = &(neighbour[nodesel]);
        xfree(nodeneigh->node);

      } else {
        completed = 1;
      }

    } while (completed == 0 && *scount < ncount);

  prev=0;
  for (i = 0; i < *scount; i++) {
    cycle[i] = prev;
    prev = genotype[prev];
  }
  xassert(prev == 0);
  for (i = *scount; i < ncount; i++)
    cycle[i] = -1;

  j=0;
  for (i = 0; i < ncount; i++) {
    if (selected[i]) {
      sposition[j] = i;
      j++;
    }
  }

  *val = len;

  if (kt->root != (CCkdtree *) NULL)
   CCkdtree_undelete_all (kt, ncount);

  for (i = 1; i < ncount; i++)
  { if (!selected[i]) {
      struct neighbour *nodeneigh = &(neighbour[i]);
      xfree(nodeneigh->node);
    }
  }
  tfree(neighbour);
  xfree(pos);
  return 0;
}

void OPadd_node (CCkdtree *kt, int ncount, compass_data *data, int node,
    int *scount, int *selected, int *sposition, int *cycle, int *genotype, double *len,
    CCrandstate *rstate)
{ int i, j, prev, next;
  double cost;
  struct neighbour *neighbour = (struct neighbour *) NULL;
  struct position *pos = (struct position *) NULL;

  neighbour = xcalloc(1, sizeof(struct neighbour));
  neighbour->this = node;
  neighbour->node = xcalloc(ncount, sizeof(int));

  if (ncount < 3) {
      fprintf (stderr, "Cannot find tour in an %d node graph\n", ncount);
  }

  if (kt->root != (CCkdtree *) NULL) {

      CCkdtree_undelete_all (kt, ncount);
    for (i = 1; i < ncount; i++) {
      if (!selected[i])
        CCkdtree_delete (kt, i);
    }

  }

  pos = xcalloc(1, sizeof(struct position));

  get_node_3_nearest (kt, ncount, data, *scount, node, selected, neighbour, 1, rstate);
  get_best_position (data, *scount, genotype, node, neighbour, pos);

  genotype[pos->prev] = node;
  genotype[node] = pos->next;
  *len += pos->cost;
  selected[node] = 1;
  *scount +=1;

  prev=0;
  for (i = 0; i < *scount; i++) {
    cycle[i] = prev;
    prev = genotype[prev];
  }
  xassert(prev == 0);
  for (i = *scount; i < ncount; i++)
    cycle[i] = -1;

  j=0;
  for (i = 0; i < ncount; i++) {
    if (selected[i]) {
      sposition[j] = i;
      j++;
    }
  }

CLEANUP:

  if (kt->root != (CCkdtree *) NULL) {
      CCkdtree_undelete_all (kt, ncount);
  }

  xfree(neighbour->node);
  xfree(neighbour);
  xfree(pos);

}

static void get_node_3_nearest (CCkdtree *kt, int ncount, compass_data *data, int scount, int node, int *selected,
    struct neighbour *neighbour, int first, CCrandstate *rstate)
{
  int i;

  if (first || scount < 3 || kt->root != (CCkdtree *) NULL)
  { if (kt->root != (CCkdtree *) NULL)
    { CCkdtree_node_k_nearest (kt, ncount, node, 3, data, (double *) NULL, neighbour->node, rstate);
    }
    else
    { size_t *indices = (size_t *) NULL;
      indices = xmalloc (3 * sizeof(size_t));
      gsl_vector *tempdist = gsl_vector_alloc(ncount);

      for (i=0; i<ncount;i++)
      { if (selected[i])
        { gsl_vector_set (tempdist, i, (double) Edgelen (data, node, i));
        }
        else
        { gsl_vector_set (tempdist, i, BIGDOUBLE);
        }
      }

      gsl_sort_vector_smallest_index (indices, 3, tempdist);

      neighbour->node[0] = (int) indices[0];
      neighbour->node[1] = (int) indices[1];
      neighbour->node[2] = (int) indices[2];

      xfree(indices);
      gsl_vector_free(tempdist);

    }
  }
  else
  { size_t *indices = (size_t *) NULL;
    indices = xmalloc (3 * sizeof(size_t));
    gsl_vector *tempdist = gsl_vector_alloc(ncount);

    for (i=0; i<ncount; i++)
    { if (i == neighbour->node[0] || i == neighbour->node[1] ||
          i == neighbour->node[2] || i == neighbour->node[3] )
      { gsl_vector_set (tempdist, i, (double) Edgelen (data, node, i));
      }
      else
      { gsl_vector_set (tempdist, i, BIGDOUBLE);
      }
    }

    gsl_sort_vector_smallest_index (indices, 3, tempdist);
    neighbour->node[0] = (int) indices[0];
    neighbour->node[1] = (int) indices[1];
    neighbour->node[2] = (int) indices[2];

    xfree(indices);
    gsl_vector_free(tempdist);
  }
}

static void get_best_position (compass_data *data, int scount, int *genotype, int node, struct neighbour *nodeneigh,
    struct position *pos)
{
  int prev, next;
  double cost, best = BIGDOUBLE;

  if (scount >2 )
  { if (genotype[nodeneigh->node[0]] == nodeneigh->node[1] ||
      genotype[nodeneigh->node[1]] == nodeneigh->node[0] ||
      genotype[nodeneigh->node[0]] == nodeneigh->node[2] ||
      genotype[nodeneigh->node[2]] == nodeneigh->node[0] ||
      genotype[nodeneigh->node[1]] == nodeneigh->node[2] ||
      genotype[nodeneigh->node[2]] == nodeneigh->node[1] )
  { if (genotype[nodeneigh->node[0]] == nodeneigh->node[1] )
    { best = (double) ( Edgelen (data, nodeneigh->node[0], node) +
          Edgelen (data, node, nodeneigh->node[1]) -
          Edgelen (data, nodeneigh->node[0], nodeneigh->node[1]));
      prev = nodeneigh->node[0];
      next = nodeneigh->node[1];
    }
    if (genotype[nodeneigh->node[1]] == nodeneigh->node[0] )
    { cost = (double) ( Edgelen (data, nodeneigh->node[1], node) +
          Edgelen (data, node, nodeneigh->node[0]) -
          Edgelen (data, nodeneigh->node[1], nodeneigh->node[0]));
      if (cost < best)
      { prev = nodeneigh->node[1];
        next = nodeneigh->node[0];
        best = cost;
      }
    }
    if (genotype[nodeneigh->node[0]] == nodeneigh->node[2] )
    { cost = (double) ( Edgelen (data, nodeneigh->node[0], node) +
          Edgelen (data, node, nodeneigh->node[2]) -
         Edgelen (data, nodeneigh->node[0], nodeneigh->node[2]));
      if (cost < best)
      { prev = nodeneigh->node[0];
        next = nodeneigh->node[2];
        best = cost;
      }
    }
    if (genotype[nodeneigh->node[2]] == nodeneigh->node[0] )
    { cost = (double) ( Edgelen (data, nodeneigh->node[2], node) +
          Edgelen (data, node, nodeneigh->node[0]) -
          Edgelen (data, nodeneigh->node[2], nodeneigh->node[0]));
      if (cost < best)
      { prev = nodeneigh->node[2];
        next = nodeneigh->node[0];
        best = cost;
      }
    }
    if (genotype[nodeneigh->node[1]] == nodeneigh->node[2] )
    { cost = (double) ( Edgelen (data, nodeneigh->node[1], node) +
        Edgelen (data, node, nodeneigh->node[2]) -
        Edgelen (data, nodeneigh->node[1], nodeneigh->node[2]));
      if (cost < best)
      { prev = nodeneigh->node[1];
        next = nodeneigh->node[2];
        best = cost;
      }
    }
    if (genotype[nodeneigh->node[2]] == nodeneigh->node[1] )
    { cost = (double) ( Edgelen (data, nodeneigh->node[2], node) +
        Edgelen (data, node, nodeneigh->node[1]) -
        Edgelen (data, nodeneigh->node[2], nodeneigh->node[1]));
      if (cost < best)
      { prev = nodeneigh->node[2];
        next = nodeneigh->node[1];
        best = cost;
      }
    }
  }
  else
  { best = (double) ( Edgelen (data, nodeneigh->node[0], node) +
        Edgelen (data, node, genotype[nodeneigh->node[0]]) -
        Edgelen (data, nodeneigh->node[0], genotype[nodeneigh->node[0]]));
    prev = nodeneigh->node[0];
    next = genotype[nodeneigh->node[0]];

    cost = (double) ( Edgelen (data, nodeneigh->node[1], node) +
        Edgelen (data, node,genotype[nodeneigh->node[1]]) -
        Edgelen (data, nodeneigh->node[1], genotype[nodeneigh->node[1]]));
    if (cost < best)
    { prev = nodeneigh->node[1];
      next = genotype[nodeneigh->node[1]];
      best = cost;
    }

    cost = (double) ( Edgelen (data, nodeneigh->node[2], node) +
        Edgelen (data, node,genotype[nodeneigh->node[2]]) -
        Edgelen (data, nodeneigh->node[2], genotype[nodeneigh->node[2]]));
    if (cost < best)
    { prev = nodeneigh->node[2];
      next = genotype[nodeneigh->node[2]];
      best = cost;
    }
  }
  pos->prev = prev;
  pos->next = next;
  pos->cost = best;
  }
  else if (scount ==1 )
  { pos->prev = 0;
    pos->next = 0;
    pos->cost = (double) ( 2*Edgelen (data, 0, node));
  }
  else if (scount == 2 )
  { pos->prev = 0;
    pos->next = genotype[0];
    pos->cost = (double) ( Edgelen (data, 0, node) + Edgelen(data, node, genotype[0])-
        Edgelen(data,0,genotype[0]));
  }
}
