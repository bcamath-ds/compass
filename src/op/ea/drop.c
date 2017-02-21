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

#define BIGDOUBLE (1e30)

int OPdrop_operator ( int ncount, compass_data *data,
    int *scount, int *selected, int *sposition, int *cycle, int *genotype, double *val,
    double *scores, double cost_limit, CCrandstate *rstate)
{
    double len, worst, tcost, cost, greedyeval;
    int i, j, prev, next;
    int nodesel, nodeprev, nodenext;

    if (ncount < 4) {
        fprintf (stderr, "Cannot drop nodes in an %d node tour\n", ncount);
        return 1;
    }

    len = *val;

    while ( len > cost_limit) {
      worst = BIGDOUBLE;
      for (i = 1; i < ncount; i++)
      { if (selected[i])
        { for (j = 0; j < ncount; j++)
          { if (genotype[j] == i)
              prev = j;
          }
          next = genotype[i];

          tcost = (double) (CCutil_dat_edgelen(prev, i, data) +
              CCutil_dat_edgelen(i,next, data) -
              CCutil_dat_edgelen (prev, next, data));

          if (scores[i]!=0)
            greedyeval =  (double) scores[i] / tcost;
          else
            greedyeval = -BIGDOUBLE;

          if (greedyeval < worst) {
            nodesel   = i;
            nodeprev  = prev;
            nodenext  = next;
            worst     = greedyeval;
            cost     =  tcost;
          }
        }
      }

      genotype[nodeprev] = nodenext;
      genotype[nodesel] = nodesel;

      selected[nodesel] = 0;
      *scount -=1;

      len -= cost;

    }

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
}

void OPdrop_node (int ncount, compass_data *data, int node,
    int *scount, int *selected, int *sposition, int *cycle, int *genotype, double *len,
    CCrandstate *rstate)
{

  int i, j, prev, next;
  double cost;

  if (ncount < 4) {
    fprintf (stderr, "Cannot drop nodes in an %d node tour\n", ncount);
  }

  for (j = 0; j < ncount; j++) {
    if (genotype[j] == node)
      prev = j;
  }
  next = genotype[node];

  cost = (double) (compass_get_edge_len(prev, node, data) +  compass_get_edge_len(node,next, data) -  compass_get_edge_len (prev, next, data));

  genotype[prev] = next;
  genotype[node] = node;

  selected[node] = 0;
  *scount -=1;

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

  *len -= cost;

}
