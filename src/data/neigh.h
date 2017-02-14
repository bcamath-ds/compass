#ifndef NEIGH_H
#define NEIGH_H

typedef struct CCxnear {
    compass_data dat;
    double            *w;
    int               *nodenames;
    int               *invnames;
} CCxnear;

struct neigh_cp
{     /*Neighbor Graph control parameters */
  int msg_lev;              /* message level: */
#define COMPASS_MSG_OFF   0  /* no output */
#define COMPASS_MSG_ERR   1  /* warning and error messages only */
#define COMPASS_MSG_ON    2  /* normal output */
#define COMPASS_MSG_ALL   3  /* full output */
#define COMPASS_MSG_DBG   4  /* debug output */
  int tm_start;             /* starting timestamp */
  int tm_end;               /* ending timestamp */
  int neigh_graph;
#define NEIGH_NEAREST 0
#define NEIGH_QUADNEAREST 1
#define NEIGH_DELAUNAY 2
  /* Neighbor graph technique */
  int k;
  /* Number of k nearest */
};

#endif
