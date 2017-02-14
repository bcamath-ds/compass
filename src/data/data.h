#ifndef DATA_H
#define DATA_H


#undef  CCUTIL_EDGELEN_FUNCTIONPTR

typedef struct CCdata_user {
    double  *x;
    double  *y;
} CCdata_user;

typedef struct CCdata_rhvector {
    int dist_00;
    int dist_01;
    int dist_02;
    int dist_12;
    int dist_22;
    double p;
    int rhlength;
    char *space;
    char **vectors;
} CCdata_rhvector;

typedef struct compass_data compass_data;
typedef struct compass_file compass_file;
typedef struct CCdata_user CCdata_user;
typedef struct CCdata_rhvector CCdata_rhvector;

struct compass_data
{  int    (*edgelen) ( int i, int j, struct compass_data *data);
    int      n;
    double  *x;
    double  *y;
    double  *z;
    int    **adj;
    int     *adjspace;
    int    **len;
    int     *lenspace;
    int     *degree;
    int      norm;
    int      dsjrand_param;
    int      default_len;     /* for edges not in sparse graph   */
    int      sparse_ecount;   /* number of edges in sparse graph */
    double   gridsize;        /* for toroidal norm */
    double   dsjrand_factor;
    CCdata_rhvector rhdat;
    CCdata_user     userdat;
    int      ndepot;          /* used with the subdivision code   */
    int      orig_ncount;     /* just ncount-ndepot               */
    int     *depotcost;       /* cost from each node to the depot */
    int     *orig_names;      /* the nodes names from full problem */
};

#define CC_KD_NORM_TYPE    128            /* Kdtrees work      */
#define CC_X_NORM_TYPE     256            /* Old nearest works */
#define CC_JUNK_NORM_TYPE  512            /* Nothing works     */

#define CC_D2_NORM_SIZE      1024         /* x,y coordinates   */
#define CC_D3_NORM_SIZE      2048         /* x,y,z coordinates */
#define CC_MATRIX_NORM_SIZE  4096         /* adj matrix        */

#define CC_NORM_BITS      (CC_KD_NORM_TYPE | CC_X_NORM_TYPE | \
                           CC_JUNK_NORM_TYPE)
#define CC_NORM_SIZE_BITS (CC_D2_NORM_SIZE | CC_D3_NORM_SIZE | \
                           CC_MATRIX_NORM_SIZE)

#define CC_MAXNORM        (0 |   CC_KD_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_EUCLIDEAN_CEIL (1 |   CC_KD_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_EUCLIDEAN      (2 |   CC_KD_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_EUCLIDEAN_3D   (3 |    CC_X_NORM_TYPE |     CC_D3_NORM_SIZE)
#define CC_USER           (4 | CC_JUNK_NORM_TYPE |                   0)
#define CC_ATT            (5 |    CC_X_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_GEOGRAPHIC     (6 |    CC_X_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_MATRIXNORM     (7 | CC_JUNK_NORM_TYPE | CC_MATRIX_NORM_SIZE)
#define CC_DSJRANDNORM    (8 | CC_JUNK_NORM_TYPE |                   0)
#define CC_CRYSTAL        (9 |    CC_X_NORM_TYPE |     CC_D3_NORM_SIZE)
#define CC_SPARSE        (10 | CC_JUNK_NORM_TYPE |                   0)
#define CC_RHMAP1        (11 | CC_JUNK_NORM_TYPE |                   0)
#define CC_RHMAP2        (12 | CC_JUNK_NORM_TYPE |                   0)
#define CC_RHMAP3        (13 | CC_JUNK_NORM_TYPE |                   0)
#define CC_RHMAP4        (14 | CC_JUNK_NORM_TYPE |                   0)
#define CC_RHMAP5        (15 | CC_JUNK_NORM_TYPE |                   0)
#define CC_EUCTOROIDAL   (16 | CC_JUNK_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_GEOM          (17 |    CC_X_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_MANNORM       (18 |   CC_KD_NORM_TYPE |     CC_D2_NORM_SIZE)
#define CC_SUBDIVISION   (99 | CC_JUNK_NORM_TYPE |                   0)

#define CC_GEOGRAPHIC_SCALE (6378.388 * 3.14 / 180.0)    /*  see edgelen.c  */
#define CC_GEOM_SCALE (6378388.0 * 3.14 / 180.0)         /*  see edgelen.c  */
#define CC_ATT_SCALE (.31622)                            /*  sqrt(1/10)     */

/* Distances CC_RHMAP1 through CC_RHMAP5 are for an application to          */
/* radiation hybrid mapping in genetics, explained in: Agarwala R,          */
/* Applegate DL,  Maglott D, Schuler GD, Schaffer AA: A Fast and Scalable   */
/* Radiation Hybrid Map Construction and Integration Strategy. Genome       */
/* Research, 10:350-364, 2000.  The correspondence to the distance function */
/* terms used in that paper is: CC_RMAP1 (weighted_ocb), CC_RHMAP2          */
/* (normalized_mle), CC_RHMAP3 (base_mle), CC_RHMAP4 (extended_mle),        */
/* CC_RHMAP5 (normalized_ocb)                                               */

/* For X-NORMS, scales are such that |x[i] - x[j]| * scale <= edgelen(i,j). */
/* Geographic is slightly off, since the fractional part of x[i] is really  */
/* really minutes, not fractional degrees.                                  */

#endif

