#include "compass.h"
#include "env.h"
#include "machdefs.h"
#include "util.h"
#include "macrorus.h"

static double
    dtrunc (double);

static int
    edgelen_nonorm (int i, int j, compass_data *data),
    max_edgelen (int i, int j, compass_data *data),
    man_edgelen (int i, int j, compass_data *data),
    euclid_edgelen (int i, int j, compass_data *data),
    euclid_ceiling_edgelen (int i, int j, compass_data *data),
    euclid3d_edgelen (int i, int j, compass_data *data),
    geographic_edgelen (int i, int j, compass_data *data),
    geom_edgelen (int i, int j, compass_data *data),
    att_edgelen (int i, int j, compass_data *data),
//    dsjrand_edgelen (int i, int j, compass_data *data),
    crystal_edgelen (int i, int j, compass_data *data),
    matrix_edgelen (int i, int j, compass_data *data),
    sparse_edgelen (int i, int j, compass_data *data),
//    user_edgelen (int i, int j, compass_data *data),
//    rhmap1_edgelen (int i, int j, compass_data *data),
//    rhmap2_edgelen (int i, int j, compass_data *data),
//    rhmap3_edgelen (int i, int j, compass_data *data),
//    rhmap4_edgelen (int i, int j, compass_data *data),
//    rhmap5_edgelen (int i, int j, compass_data *data),
    toroidal_edgelen (int i, int j, compass_data *data);

typedef struct compass_data compass_data;

/***********************************************************************
*  NAME
*
*  compass_init_data - create problem object
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

static int edgelen_nonorm (int i, int j, compass_data *data)
{
    xprintf("erroor %d\n", data->adj[i][j]);
    put_err_msg( "OPutil_dat_edgelen has been called with no norm set\n");
    put_err_msg( "This is a FATAL ERROR\n");
    if (i != 0 || j != 0 || data != (compass_data *) NULL) {
        /* so the compiler won't complain about unused variables */
        put_err_msg( "This is a FATAL ERROR\n");
        exit (1);
    }
    return -1;
}

static void init_data ( compass_data *data)
{ data->n = 0;
  data->x = (double *) NULL;
  data->y = (double *) NULL;
  data->z = (double *) NULL;
  data->adj = (int **) NULL;
  data->adjspace = (int *) NULL;
  data->len      = (int **) NULL;
  data->lenspace = (int *) NULL;
  data->degree   = (int *) NULL;
  data->norm = 0;
  data->dsjrand_param = 1;
  data->dsjrand_factor = 1.0;
  data->default_len = 100000;
  data->sparse_ecount = 0;
  data->edgelen = edgelen_nonorm;
#if 0
  init_userdata (&data->userdat);
  init_rhdata (&data->rhdata);
#endif
  data->ndepot = 0;
  data->orig_ncount = 0;
  data->depotcost = (int *) NULL;
  data->orig_names = (int *) NULL;
  return;
}

void compass_init_data( compass_data *data)
{ init_data(data);
  return;
}

int compass_get_edge_len (int i, int j, compass_data *data)
{
  /*
    if (data->ndepot) {
        if (i >= data->orig_ncount) {
            return data->depotcost[j];
        } else if (j >= data->orig_ncount) {
            return data->depotcost[i];
        }
    }
    */
    return (data->edgelen)(i, j, data);
}

/***********************************************************************
*  NAME
*
*  compass_erase_data - erase data object content
*
*  SYNOPSIS
*
*  void compass_erase_data(compass_data *data);
*
*  DESCRIPTION
*
*  The routine compass_erase_data erases the content of the specified
*  data object. The effect of this operation is the same as if the
*  data object would be deleted with the routine compass_delete_data and
*  then created anew with the routine compass_init_data, with exception
*  that the handle (pointer) to the problem object remains valid. */

static void delete_data(compass_data *data);

void compass_erase_data(compass_data *data)
{     //compass_op_tree *tree = prob->tree;
      //if (tree != NULL && tree->reason != 0)
      //   xerror("compass_op_erase_prob: operation not allowed\n");
      //
      delete_data(data);
      init_data(data);
      return;
}


/***********************************************************************
*  NAME
*
*  compass_delete_data - delete compass_data object
*
*  SYNOPSIS
*
*  void compass_delete_data(compass_data *data);
*
*  DESCRIPTION
*
*  The routine compass_delete_data deletes the compass_data object and
*  frees all the memory allocated to it. */

static void delete_data(compass_data *data)
{ if (data->x != (int *) NULL) xfree (data->x);
  if (data->y != (int *) NULL) xfree (data->y);
  if (data->z != (int *) NULL) xfree (data->z);
#if 0
  xfree(data->tw_opening);
  xfree(data->tw_closing);
#endif
  if (data->adj != (int **) NULL) xfree (data->adj);
  if (data->adjspace != (int *) NULL) xfree (data->adjspace);
  if (data->len != (int **) NULL) xfree (data->len);
  if (data->lenspace != (int *) NULL) xfree (data->lenspace);
  if (data->degree != (int *) NULL) xfree (data->degree);
#if 0
  delete_userdata (&data->userdat);
  delete_rhdata (&data->rhdata);
#endif
  if (data->depotcost != (int *) NULL) xfree (data->depotcost);
  if (data->orig_names != (int *) NULL) xfree (data->orig_names);
  return;
}

void compass_delete_data(compass_data *data)
{ 
  delete_data(data);
  tfree(data);
  return;
}

int compass_data_set_norm (compass_data *data, int norm)
{
    switch (norm) {
    case CC_EUCLIDEAN_CEIL:
        data->edgelen = euclid_ceiling_edgelen;
        break;
    case CC_EUCLIDEAN:
        data->edgelen = euclid_edgelen;
        break;
    case CC_MAXNORM:
        data->edgelen = max_edgelen;
        break;
    case CC_MANNORM:
        data->edgelen = man_edgelen;
        break;
    case CC_EUCLIDEAN_3D:
        data->edgelen = euclid3d_edgelen;
        break;
//    case CC_USER:
//        data->edgelen = user_edgelen;
//        break;
    case CC_GEOGRAPHIC:
        data->edgelen = geographic_edgelen;
        break;
    case CC_GEOM:
        data->edgelen = geom_edgelen;
        break;
    case CC_ATT:
        data->edgelen = att_edgelen;
        break;
    case CC_MATRIXNORM:
        data->edgelen = matrix_edgelen;
        break;
//    case CC_DSJRANDNORM:
//        data->edgelen = dsjrand_edgelen;
//        break;
//    case CC_CRYSTAL:
//        data->edgelen = crystal_edgelen;
//        break;
    case CC_SPARSE:
        data->edgelen = sparse_edgelen;
        break;
//    case CC_RHMAP1:
//        data->edgelen = rhmap1_edgelen;
//        break;
//    case CC_RHMAP2:
//        data->edgelen = rhmap2_edgelen;
//        break;
//    case CC_RHMAP3:
//        data->edgelen = rhmap3_edgelen;
//        break;
//    case CC_RHMAP4:
//        data->edgelen = rhmap4_edgelen;
//        break;
//    case CC_RHMAP5:
//        data->edgelen = rhmap5_edgelen;
//        break;
    case CC_EUCTOROIDAL:
        data->edgelen = toroidal_edgelen;
        break;
    default:
        put_err_msg("ERROR:  Unknown NORM %d.\n", norm);
        return 1;
    }
    data->norm = norm;

#ifdef CCUTIL_EDGELEN_FUNCTIONPTR
    OPutil_dat_edgelen = data->edgelen;
#endif /* CCUTIL_EDGELEN_FUNCTIONPTR */

    return 0;
}

void compass_get_dat_norm (compass_data *data, int *norm)
{
    (*norm) = data->norm;
}


static int max_edgelen (int i, int j, compass_data *data)
{
    double t1 = data->x[i] - data->x[j], t2 = data->y[i] - data->y[j];

    if (t1 < 0)
        t1 *= -1;
    if (t2 < 0)
        t2 *= -1;
    t1 += 0.5;
    t2 += 0.5;

    return (int) (t1 < t2 ? t2 : t1);
}

static int man_edgelen (int i, int j, compass_data *data)
{
    double t1 = data->x[i] - data->x[j], t2 = data->y[i] - data->y[j];

    if (t1 < 0)
        t1 *= -1;
    if (t2 < 0)
        t2 *= -1;

    return (int) (t1 + t2 + 0.5);
}

static int euclid_edgelen (int i, int j, compass_data *data)
{
    double t1 = data->x[i] - data->x[j], t2 = data->y[i] - data->y[j];
    int temp;

    temp = (int) (sqrt (t1 * t1 + t2 * t2) + 0.5);
    return temp;
}

static int toroidal_edgelen (int i, int j, compass_data *data)
{
    double t1 = data->x[i] - data->x[j];
    double t2 = data->y[i] - data->y[j];
    int temp;

    if (t1 < 0) t1 = -t1;
    if (t2 < 0) t2 = -t2;
    if (data->gridsize - t1 < t1) t1 = data->gridsize - t1;
    if (data->gridsize - t2 < t2) t2 = data->gridsize - t2;
    temp = (int) (sqrt (t1 * t1 + t2 * t2) + 0.5);
    return temp;
}

static int euclid3d_edgelen (int i, int j, compass_data *data)
{
    double t1 = data->x[i] - data->x[j], t2 = data->y[i] - data->y[j];
    double t3 = data->z[i] - data->z[j];
    int temp;

    temp = (int) (sqrt (t1 * t1 + t2 * t2 + t3 * t3) + 0.5);
    return temp;
}

static int euclid_ceiling_edgelen (int i, int j, compass_data *data)
{
    double t1 = data->x[i] - data->x[j], t2 = data->y[i] - data->y[j];
/*
    int rd;
    double max;

    max = sqrt (t1 * t1 + t2 * t2);
    rd = (int) max;
    return (((max - rd) > .000000001) ? rd + 1 : rd);
*/
    return (int) (ceil (sqrt (t1 * t1 + t2 * t2)));
}

#define GH_PI (3.141592)


#define GH_PI (3.141592)

static int geographic_edgelen (int i, int j, compass_data *data)
{
    double deg, min;
    double lati, latj, longi, longj;
    double q1, q2, q3;
    int dd;
    double x1 = data->x[i], x2 = data->x[j], yy1 = data->y[i], yy2 = data->y[j];

    deg = dtrunc (x1);
    min = x1 - deg;
    lati = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = dtrunc (x2);
    min = x2 - deg;
    latj = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;

    deg = dtrunc (yy1);
    min = yy1 - deg;
    longi = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = dtrunc (yy2);
    min = yy2 - deg;
    longj = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;

    q1 = cos (longi - longj);
    q2 = cos (lati - latj);
    q3 = cos (lati + latj);
    dd = (int) (6378.388 * acos (0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3))
                + 1.0);
    return dd;
}

static int geom_edgelen (int i, int j, compass_data *data)
{
    double lati, latj, longi, longj;
    double q1, q2, q3, q4, q5;

    lati = M_PI * data->x[i] / 180.0;
    latj = M_PI * data->x[j] / 180.0;

    longi = M_PI * data->y[i] / 180.0;
    longj = M_PI * data->y[j] / 180.0;

    q1 = cos (latj) * sin(longi - longj);
    q3 = sin((longi - longj)/2.0);
    q4 = cos((longi - longj)/2.0);
    q2 = sin(lati + latj) * q3 * q3 - sin(lati - latj) * q4 * q4;
    q5 = cos(lati - latj) * q4 * q4 - cos(lati + latj) * q3 * q3;
    return (int) (6378388.0 * atan2(sqrt(q1*q1 + q2*q2), q5) + 1.0);
}

static int att_edgelen (int i, int j, compass_data *data)
{
    double xd = data->x[i] - data->x[j];
    double yd = data->y[i] - data->y[j];
    double rij = sqrt ((xd * xd + yd * yd) / 10.0);
    double tij = dtrunc (rij);
    int dij;

    if (tij < rij)
        dij = (int) tij + 1;
    else
        dij = (int) tij;
    return dij;
}

static double dtrunc (double x)
{
    int k;

    k = (int) x;
    x = (double) k;
    return x;
}

static int matrix_edgelen (int i, int j, compass_data *data)
{
    if (i > j)
        return (data->adj[i])[j];
    else
        return (data->adj[j])[i];
}

static int sparse_edgelen (int i, int j, compass_data *data)
{
    int *adj;
    int k, deg;

    if (i > j) {
        CC_SWAP (i, j, k);
    }
    adj = data->adj[i];
    deg = data->degree[i];

    for (k = 0; k < deg; k++) {
        if (adj[k] == j) {
            return data->len[i][k];
        }
    }
    return data->default_len;
}

