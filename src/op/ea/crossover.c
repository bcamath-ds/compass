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
#include "machdefs.h"
#include "util.h"
#include "macrorus.h"
#include "env.h"
#include "rng.h"
#include "op.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_vector.h>

#define BIGINT 2000000000

typedef struct selnode {
    int nextsel1;
    int nextsel2;
    int prevsel1;
    int prevsel2;
    int *next_internodes1;
    int *next_internodes2;
    int *prev_internodes1;
    int *prev_internodes2;
    int next_ninter1;
    int next_ninter2;
    int prev_ninter1;
    int prev_ninter2;
} selnode;

static void
  initselnodes (selnode *selnode),
  freeselnodes (selnode *selnode),
  reset_relative_degree (gsl_vector *relative_degree, gsl_vector *unvisited, gsl_vector *degree, selnode *selnodes, int current),
  update_neigh_degrees (gsl_vector *degree, selnode *selnodes, int current),
  insert_inter_nodes (selnode *selnodes, int current, int next,
                      int *scount, int *selected, int *sposition, int *cycle, int *genotype,
                      gsl_rng *rstate_gsl
                      );

static int 
  select_connected_next(int ncount, selnode *selnodes, int current, gsl_vector *unvisited, gsl_vector *relative_degree, gsl_rng *rstate_gsl),
  select_disconnected_next(int ncount, int current, gsl_vector *unvisited, gsl_vector *degree, gsl_rng *rstate_gsl),
  select_connected_visited (int ncount, selnode *selnodes, int current, gsl_vector *degree, gsl_rng *rstate_gsl);

int op_ga_crossover (int ncount, int scount1, int scount2, int *selected1, int *selected2,
    int *sposition1, int *sposition2, int *cycle1, int *cycle2, int *genotype1, int *genotype2,
    int *scount, int *selected, int *sposition, int *cycle, int *genotype,
    gsl_rng *rstate_gsl)
{
  int rval=0;
  int i, j, k;
  int next1, next2;
  double len, worst, cost;
  gsl_vector *relative_degree = gsl_vector_alloc(ncount);
  int node;
  int bscount, nunvisited;
  int *tinternodes;
  int ninter, ninter1, ninter2;
  int current, prev, next;
  int connected;
  selnode *selnodes;


  // We select the nodes that are in the two paths
  bscount=0; //count nodes visited in both paths
  for (i=0; i<ncount; i++){
    selected[i] = selected1[i]*selected2[i];
    if (selected[i]) bscount++;
  }

  selnodes = xcalloc (ncount, sizeof(selnode));

  for (i=0;i<ncount;i++) initselnodes(&selnodes[i]);

  // Temp array to save intermediate nodes
  tinternodes =  xcalloc (ncount, sizeof(int));
  if (!tinternodes) {
    rval = 1;
    goto cleanup;
  }

  for (i=0; i<ncount; i++){
    if (selected[i]) {

      // For parent 1
      ninter1=0;
      next1 = genotype1[i];
      while (!selected[next1]) {
        tinternodes[ninter1] = next1;
        ninter1+=1;

        next1 = genotype1[next1];
      }

      selnodes[i].nextsel1 = next1;
      selnodes[i].next_ninter1 = ninter1;
      selnodes[next1].prevsel1 = i;
      selnodes[next1].prev_ninter1 = ninter1;

      if (ninter1 != 0) {
        selnodes[i].next_internodes1 = xcalloc (ninter1, sizeof(int));
        selnodes[next1].prev_internodes1 = xcalloc (ninter1, sizeof(int));
        for (j=0; j<ninter1; j++){
          selnodes[i].next_internodes1[j] = tinternodes[j];
          selnodes[next1].prev_internodes1[ninter1 - j - 1] = tinternodes[j];
        }
      }


      // For parent 2
      ninter2=0;
      next2 = genotype2[i];
      while (!selected[next2]) {
        tinternodes[ninter2] = next2;
        ninter2+=1;

        next2 = genotype2[next2];
      }

      selnodes[i].nextsel2 = next2;
      selnodes[i].next_ninter2 = ninter2;
      selnodes[next2].prevsel2 = i;
      selnodes[next2].prev_ninter2 = ninter2;

      if (ninter2 != 0) {
        selnodes[i].next_internodes2 = xcalloc (ninter2, sizeof(int));
        selnodes[next2].prev_internodes2 = xcalloc (ninter2, sizeof(int));
        for (j=0; j<ninter2; j++){
          selnodes[i].next_internodes2[j] = tinternodes[j];
          selnodes[next2].prev_internodes2[ninter2 - j - 1] = tinternodes[j];
        }
      }


    }
  }

  //Save the vertex degree to use in the crossover. How many diferent neighbours 
  //have in the the reduced parents.

  int *degree_init = xcalloc (ncount, sizeof(int));

  for (i=0; i<ncount; i++){
    if ( selected[i] ) {
      gsl_vector *neightnodes = gsl_vector_alloc(4);

      gsl_vector_set (neightnodes, 0, selnodes[i].nextsel1);
      gsl_vector_set (neightnodes, 1, selnodes[i].prevsel1);
      gsl_vector_set (neightnodes, 2, selnodes[i].nextsel2);
      gsl_vector_set (neightnodes, 3, selnodes[i].prevsel2);

      gsl_sort_vector(neightnodes);

      degree_init[i] = 1;
      node = gsl_vector_get (neightnodes,0);
      for (j=1; j<4; j++){
        if ( gsl_vector_get (neightnodes, j) != node) {
          degree_init[i] += 1;
          node = gsl_vector_get (neightnodes,j);
        }
      }

      gsl_vector_free(neightnodes);

    } else {
      degree_init[i] = BIGINT;
    }
  }

  gsl_vector *degree = gsl_vector_alloc(ncount);
  gsl_vector_set_zero (degree);
  for (i = 0; i < ncount; i++)
  {
    if ( selected[i] ) {
    gsl_vector_set(degree, i, degree_init[i]);
    }
  }


  nunvisited = bscount;
  gsl_vector *unvisited = gsl_vector_alloc(ncount);
  gsl_vector_set_zero (unvisited);
  for (i=1; i<ncount; i++) {
    if (selected[i])
      gsl_vector_set (unvisited, i, 1);
  }

  *scount = 0;
  current = 0;
  cycle[*scount] = current;
  *scount += 1;
  gsl_vector_set (unvisited, current, 0);
  nunvisited --;
  update_neigh_degrees (degree, selnodes, current);


  while ( nunvisited > 0 ) {

    reset_relative_degree (relative_degree, unvisited, degree, selnodes, current);

    if ( !gsl_vector_isnull (relative_degree) ) {

      next = select_connected_next(ncount, selnodes, current, unvisited, relative_degree, rstate_gsl);

      insert_inter_nodes (selnodes, current, next, &(*scount), selected, sposition, cycle, genotype, rstate_gsl);

      current = next;
      cycle[*scount] = current;
      *scount +=1;
      gsl_vector_set (unvisited, current, 0);
      nunvisited --;
      update_neigh_degrees (degree, selnodes, current);

    // If hasn't got unvisited connected nodes. Select one unvisited node randomly as next, and another connected (visited) node for inserting intermediates nodes.
    } else {

      next = select_disconnected_next (ncount, current, unvisited, degree, rstate_gsl);
      //connected = select_connected_visited (ncount, selnodes, current, degree, rstate_gsl);
      //insert_inter_nodes (selnodes, current, connected, &(*scount), selected, sposition, cycle, genotype, rstate_gsl);

      genotype[current] = next;

      current = next;
      cycle[*scount] = current;
      *scount +=1;
      gsl_vector_set (unvisited, current, 0);
      nunvisited --;
      update_neigh_degrees (degree, selnodes, current);

    }
  }

  //if current and 0 connected
  if ( (selnodes[current].nextsel1 == 0 || selnodes[current].prevsel1 == 0 ) &&
       (selnodes[current].nextsel2 == 0 || selnodes[current].prevsel2 == 0 ) ){
    insert_inter_nodes (selnodes, current, 0, &(*scount), selected, sposition, cycle, genotype, rstate_gsl);
  } else {
    genotype[current] = 0;
      //connected = select_connected_visited (ncount, selnodes, current, degree, rstate_gsl);
      //insert_inter_nodes (selnodes, current, connected, &(*scount), selected, sposition, cycle, genotype, rstate_gsl);
  }

  j=0;
  for (i = 0; i < ncount; i++) {
    if (selected[i]) {
      sposition[j] = i;
      j++;
    } else {
      genotype[i] = i;
    }
  }

  for (i = *scount; i < ncount; i++)
    cycle[i] = -1;


cleanup:
  gsl_vector_free(degree);
  gsl_vector_free(relative_degree);
  gsl_vector_free(unvisited);
  xfree(degree_init);
  xfree(tinternodes);
  for (i=0;i<ncount;i++) freeselnodes(&selnodes[i]);
  xfree(selnodes);
  //xfree(selnodes);
  return rval;
}

static void reset_relative_degree (gsl_vector *relative_degree, gsl_vector *unvisited, gsl_vector *degree, selnode *selnodes, int current)
{

  // Initialize the relative degree vector
  gsl_vector_set_zero (relative_degree);
  gsl_vector_set (relative_degree, selnodes[current].nextsel1, 1);
  gsl_vector_set (relative_degree, selnodes[current].prevsel1, 1);
  gsl_vector_set (relative_degree, selnodes[current].nextsel2, 1);
  gsl_vector_set (relative_degree, selnodes[current].prevsel2, 1);

  // Get the relative degree respective the neighbours of the current node
  gsl_vector_mul (relative_degree, unvisited); //we remove the visited nodes
  gsl_vector_mul (relative_degree, degree);

}

static void update_neigh_degrees (gsl_vector *degree, selnode *selnodes, int current)
{
  int i, node;
  gsl_vector *neightnodes = gsl_vector_alloc(4);

  gsl_vector_set (neightnodes, 0, selnodes[current].nextsel1);
  gsl_vector_set (neightnodes, 1, selnodes[current].prevsel1);
  gsl_vector_set (neightnodes, 2, selnodes[current].nextsel2);
  gsl_vector_set (neightnodes, 3, selnodes[current].prevsel2);

  gsl_sort_vector(neightnodes);

  node = gsl_vector_get (neightnodes,0);
  gsl_vector_set(degree, node, gsl_vector_get(degree,node)-1);
  for (i=1; i<4; i++){
    if ( gsl_vector_get (neightnodes, i) != node) {
      node = gsl_vector_get (neightnodes, i);
      gsl_vector_set(degree, node, gsl_vector_get(degree,node)-1);
    }
  }

  gsl_vector_free(neightnodes);
}

static int select_connected_next(int ncount, selnode *selnodes, int current, gsl_vector *unvisited, gsl_vector *relative_degree, gsl_rng *rstate_gsl) {
  int i, next;
  double min, val;

  min = BIGINT;

  for (i=0; i< ncount; i++){
    val = (int) gsl_vector_get(relative_degree, i);
    if (val != 0 & val < min) {
      min = val;
    }
  }

  // Step 3 of the generalized edge recombination algorithm
  if (min != BIGINT) {
    gsl_vector *candidates = gsl_vector_alloc(ncount);
    int *selected = xcalloc (ncount, sizeof(int));

    for (i=0; i<ncount; i++) {
      if (gsl_vector_get(relative_degree, i) == min ) {
        gsl_vector_set(candidates, i, 1);
      } else {
        gsl_vector_set(candidates, i, 0);
      }
    }

    gsl_ran_multinomial(rstate_gsl, ncount, 1, candidates->data, selected);

    for (i=0; i<ncount; i++) {
      if (selected[i]){
        next = i;
        break;
      }
    }

    gsl_vector_free(candidates);
    xfree(selected);

    return next;
  } else {
    // If current hasn't got a unvisited connected neighbour. We return 0. 
    // Notice that 0 never could be the next node, because we start the algorithm in node 0.
    return 0;
  }
}

static int select_disconnected_next(int ncount, int current, gsl_vector *unvisited, gsl_vector *degree, gsl_rng *rstate_gsl)
{
  int i,next;
  gsl_vector *candidates = gsl_vector_alloc(ncount);
  int *selected = xcalloc (ncount, sizeof(int));

  gsl_ran_multinomial(rstate_gsl, ncount, 1, unvisited->data, selected);

  for (i=0; i<ncount; i++) {
    if (selected[i]){
      next = i;
      break;
    }
  }

  gsl_vector_free(candidates);
  xfree(selected);
  return next;
}

static int select_connected_visited (int ncount, selnode *selnodes, int current, gsl_vector *degree, gsl_rng *rstate_gsl) {
  int i, next, min;
  int connected;
  gsl_vector *candidates = gsl_vector_alloc(ncount);
  int *selected = xcalloc (ncount, sizeof(int));

  gsl_vector_set_zero (candidates);
  gsl_vector_set (candidates, selnodes[current].nextsel1, 1);
  gsl_vector_set (candidates, selnodes[current].prevsel1, 1);
  gsl_vector_set (candidates, selnodes[current].nextsel2, 1);
  gsl_vector_set (candidates, selnodes[current].prevsel2, 1);

  gsl_ran_multinomial(rstate_gsl, ncount, 1, candidates->data, selected);

  for (i=0; i<ncount; i++) {
    if (selected[i]){
      connected = i;
      break;
    }
  }
  
  gsl_vector_free(candidates);
  xfree(selected);
  return connected;
}

// This function could be written in another way using the information in cycle1 and cycle2.
static void insert_inter_nodes (selnode *selnodes, int current, int next,
                      int *scount, int *selected, int *sposition, int *cycle, int *genotype,
                      gsl_rng *rstate_gsl)
{
  int i;
  int j;
  int parent, node;

  // If there are multiple connection between current and next nodes.
  if ( (selnodes[current].nextsel1 == next || selnodes[current].prevsel1 == next ) &&
       (selnodes[current].nextsel2 == next || selnodes[current].prevsel2 == next ) ){

    node = current;
    parent = (int) gsl_ran_binomial(rstate_gsl, 0.5, 1);
    if ( parent==0 ) {
      if (selnodes[current].nextsel1 == next && selnodes[current].next_ninter1 != 0 ) {
        for (i=0; i< selnodes[current].next_ninter1; i++) {
          genotype[node] = selnodes[current].next_internodes1[i];
          node = selnodes[current].next_internodes1[i];
          cycle[*scount] = node;
          selected[node] = 1;
          *scount +=1;
        }
      } else if (selnodes[current].prevsel1 == next && selnodes[current].prev_ninter1 != 0 ){
        for (i=0; i< selnodes[current].prev_ninter1; i++) {
          genotype[node] = selnodes[current].prev_internodes1[i];
          node = selnodes[current].prev_internodes1[i];
          cycle[*scount] = node;
          selected[node] = 1;
          *scount +=1;
        }
      }
    } else {
      node = current;
      if (selnodes[current].nextsel2 == next  && selnodes[current].next_ninter2 != 0 ) {
        for (i=0; i< selnodes[current].next_ninter2; i++) {
          genotype[node] = selnodes[current].next_internodes2[i];
          node = selnodes[current].next_internodes2[i];
          cycle[*scount] = node;
          selected[node] = 1;
          *scount +=1;
        }
      } else if (selnodes[current].prevsel2 == next && selnodes[current].prev_ninter2 != 0 ){
        for (i=0; i< selnodes[current].prev_ninter2; i++) {
          genotype[node] = selnodes[current].prev_internodes2[i];
          node = selnodes[current].prev_internodes2[i];
          cycle[*scount] = node;
          selected[node] = 1;
          *scount +=1;
        }
      }
    }
    genotype[node] = next;
    // If there is a unique connection between current and next nodes.
  } else {

    node = current;
    if (selnodes[current].nextsel1 == next  && selnodes[current].next_ninter1 != 0) {
      for (i=0; i< selnodes[current].next_ninter1; i++) {
        genotype[node] = selnodes[current].next_internodes1[i];
        node = selnodes[current].next_internodes1[i];
        cycle[*scount] = node;
        selected[node] = 1;
        *scount+=1;
      }
    } else if (selnodes[current].prevsel1 == next  && selnodes[current].prev_ninter1 != 0) {
      for (i=0; i< selnodes[current].prev_ninter1; i++) {
        genotype[node] = selnodes[current].prev_internodes1[i];
        node = selnodes[current].prev_internodes1[i];
        cycle[*scount] = node;
        selected[node] = 1;
        *scount+=1;
      }
    } else if (selnodes[current].nextsel2 == next  && selnodes[current].next_ninter2 != 0) {
      for (i=0; i< selnodes[current].next_ninter2; i++) {
        genotype[node] = selnodes[current].next_internodes2[i];
        node = selnodes[current].next_internodes2[i];
        cycle[*scount] = node;
        selected[node] = 1;
        *scount+=1;
      }
    } else if (selnodes[current].prevsel2 == next  && selnodes[current].prev_ninter2 != 0) {
      for (i=0; i< selnodes[current].prev_ninter2; i++) {
        genotype[node] = selnodes[current].prev_internodes2[i];
        node = selnodes[current].prev_internodes2[i];
        cycle[*scount] = node;
        selected[node] = 1;
        *scount+=1;
      }
    }
    genotype[node] = next;
  }
}

static void initselnodes (selnode *selnode)
{
  selnode->nextsel1 = 0;
  selnode->nextsel2 = 0;
  selnode->prevsel1 = 0;
  selnode->prevsel2 = 0;
  selnode->next_internodes1   = (int *) NULL;
  selnode->next_internodes2   = (int *) NULL;
  selnode->prev_internodes1   = (int *) NULL;
  selnode->prev_internodes2   = (int *) NULL;
  selnode->next_ninter1 = 0;
  selnode->next_ninter2 = 0;
  selnode->prev_ninter1 = 0;
  selnode->prev_ninter2 = 0;
}

static void freeselnodes (selnode *selnode)
{
  if (selnode) {
    selnode->nextsel1 = 0;
    selnode->nextsel2 = 0;
    selnode->prevsel1 = 0;
    selnode->prevsel2 = 0;
    //TODO: Free this data!!!!!!!!
    if(selnode->next_internodes1 != (int *) NULL)
      xfree (selnode->next_internodes1);
    if(selnode->next_internodes2 != (int *) NULL)
      xfree (selnode->next_internodes2);
    if(selnode->prev_internodes1 != (int *) NULL)
      xfree (selnode->prev_internodes1);
    if(selnode->prev_internodes2 != (int *) NULL)
      xfree (selnode->prev_internodes2);
    selnode->next_ninter1 = 0;
    selnode->next_ninter2 = 0;
    selnode->prev_ninter1 = 0;
    selnode->prev_ninter2 = 0;
  }
}


/**********************************************************************/
void compass_op_crossover( compass_prob *prob, op_population *pop,
    op_solution *child, int *parent, struct op_cp *opcp)
//void compass_op_ga_crossover ( op_prob *prob, op_gacp gacp, int par1, int par2)
/**********************************************************************/
{ int rval;
  int i, worst, ncommon;
  int *scount;
  int sum=0;
  int *tprob;
  int *selected, *sposition, *cycle, *genotype;

  op_solution *par0 = &pop->solution[parent[0]];
  op_solution *par1 = &pop->solution[parent[1]];

  ncommon=0;
  for (i=0; i<prob->n; i++)
    if ((par0->genotype[i]==par1->genotype[i])) ncommon++;

  if ( parent[0] != parent[1] &&
     ( ncommon!=par0->ns ||
       ncommon!=par1->ns ))
  { selected = xcalloc (prob->n, sizeof(int));
    if (selected == (int *) NULL) {
      fprintf (stderr, "out of memory in cer\n");
      rval = 1; goto cleanup;
    }
    sposition = xcalloc (prob->n, sizeof(int));
    if (sposition == (int *) NULL) {
      fprintf (stderr, "out of memory in cer\n");
      rval = 1; goto cleanup;
    }
    cycle = xcalloc (prob->n, sizeof(int));
    if (cycle == (int *) NULL) {
      fprintf (stderr, "out of memory in cer\n");
      rval = 1; goto cleanup;
    }
    genotype = xcalloc (prob->n, sizeof(int));
    if (genotype == (int *) NULL) {
      fprintf (stderr, "out of memory in cer\n");
      rval = 1; goto cleanup;
    }

    scount = xcalloc (1, sizeof(int));

    rval = op_ga_crossover (prob->n, par0->ns, par1->ns, par0->selected,
      par1->selected, par0->sposition, par1->sposition, par0->cycle,
      par1->cycle, par0->genotype,  par1->genotype,
      &(scount[0]), selected, sposition, cycle, genotype, prob->rstate_gsl);
    if (rval != 0) {
      fprintf (stderr, "crossover failed\n");
      rval = 1; goto cleanup;
    }
    for (i = 0; i < prob->n; i++)
    {
      child->selected[i]  = selected[i];
      child->sposition[i] = sposition[i];
      child->genotype[i]  = genotype[i];
      child->cycle[i]     = cycle[i];
    }
    child->ns    = scount[0];
    child->val = 0.0;
    for (i = 0; i < prob->n; i++)
    { if (child->selected[i])
      child->val += prob->op->s[i];
    }
    child->length  = (double) CCutil_dat_edgelen (child->cycle[child->ns - 1], child->cycle[0], prob->data);
    for (i = 1; i < child->ns; i++)
      child->length += (double) CCutil_dat_edgelen (child->cycle[i - 1], child->cycle[i], prob->data);

cleanup:
    xfree(selected);
    xfree(sposition);
    xfree(cycle);
    xfree(genotype);
    xfree(scount);
  }
  else
  {
    if (rng_unif_01(prob->rstate) < 0.5) {
      op_solution *par = &pop->solution[parent[0]];
      compass_op_copy_sol(prob, par, child);
    } else {
      op_solution *par = &pop->solution[parent[1]];
      compass_op_copy_sol(prob, par, child);
    }

  }
}
