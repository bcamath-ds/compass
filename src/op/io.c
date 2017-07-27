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
#include "tsp.h"
#include "op.h"
#include "util.h"
#include "env.h"

struct compass_file
{     /* sequential stream descriptor */
      char *base;
      /* pointer to buffer */
      int size;
      /* size of buffer, in bytes */
      char *ptr;
      /* pointer to next byte in buffer */
      int cnt;
      /* count of bytes in buffer */
      int flag;
      /* stream flags: */
#define IONULL 0x01 /* null file */
#define IOSTD  0x02 /* standard stream */
#define IOGZIP 0x04 /* gzipped file */
#define IOWRT  0x08 /* output stream */
#define IOEOF  0x10 /* end of file */
#define IOERR  0x20 /* input/output error */
      void *file;
      /* pointer to underlying control object */
};



#define MATRIX_LOWER_DIAG_ROW  0
#define MATRIX_UPPER_ROW       1
#define MATRIX_UPPER_DIAG_ROW  2
#define MATRIX_FULL_MATRIX     3

#define SCORES_FIXED     0
#define SCORES_FSGRANDOM     1

#define xfprintf compass_format

typedef struct compass_file compass_file;

static void error(const char *fname, const char *fmt, const int count, ...)
{ /* print error message and terminate processing */
  va_list arg;
  xprintf("%s:%d: error: ", fname, count);
  va_start(arg, fmt);
  xvprintf(fmt, arg);
  va_end(arg);
  xprintf("\n");
  return;
}

static void warning( const char *fname, const char *fmt, const int count, ...)
{ /* print warning message and continue processing */
 va_list arg;
 xprintf("%s:%d: warning: ", fname, count);
 va_start(arg, fmt);
 xvprintf(fmt, arg);
 va_end(arg);
 xprintf("\n");
 return;
}

/***********************************************************************
*  NAME
*
*  compass_read_prob - read Orienteering Problem data in LIB format
*
*  SYNOPSIS
*
*  int compass_read_prob(compass_prob *prob, int fmt, const op_opcp *parm,
*     const char *fname);
*
*  DESCRIPTION
*
*  The routine compass_read_op_prob reads Orienteering Problem data in LIB
*  format from a text file.
*
*  The parameter fmt specifies the file format:
*
*  FMT_LIB_FILE - LIB format;
*  FMT_MPS_FILE - free (modern) MPS format.
*
*  The parameter parm is a pointer to the structure op_opcp, which specifies
*  control parameters used by the routine. If parm is NULL, the routine uses
*  default settings.
*
*  The character string fname specifies a name of the text file to be read.
*
*  Note that before reading data the current content of the problem object is
*  completely erased with the routine compass_erase_op_prob.
*
*  RETURNS
*
*  If the operation was successful, the routine compass_read_prob returns zero.
*  Otherwise, it prints an error message and returns non-zero. */

static int read_lib( compass_prob *prob, compass_file *fp)
{ compass_data *data = prob->data;
  int matrixform = MATRIX_LOWER_DIAG_ROW;
  int scoresform = SCORES_FIXED;
  char buf[256], key[256], field[256];
  char *p,*q;
  xprintf("\n");
  while (fgets (buf, 254,  (FILE *) fp->file) != (char *) NULL)
  { p = buf;
    while (*p != '\0')
    { if (*p == ':')
        *p = ' ';
      p++;
    }
    p = buf;
    if (sscanf (p, "%s", key) != EOF)
    { p += strlen (key);
      while (*p == ' ')
        p++;
      if (!strcmp (key, "NAME"))
      { //if (p[strlen(p)-2]=='\r')
        //  strncpy(prob->name , p,strlen(p)-2);
        //else
       if (p[strlen(p)-2]=='\r')
       { p[strlen(p)-2] = '\0';
         memcpy(prob->name , p,strlen(p)-2);
       }
       if (p[strlen(p)-1]=='\n')
       { p[strlen(p)-1] = '\0';
         memcpy(prob->name , p,strlen(p));
                }
       else {
         p[strlen(p)-2] = '\0';
        memcpy(prob->name , p,strlen(p)-2);
       }
        for (q = prob->name; q != '\0'; q++)
          *q = (char) tolower(*q);
        xprintf ("  Problem Name: %s\n", p);
      }
/*--------------------------------------------------------------------------*/
/* Type */
      else if (!strcmp (key, "TYPE"))
      { xprintf ("  Problem Type: %s", p);
        if (sscanf (p, "%s", field) == EOF )
        { put_err_msg ( "Missing problem type\n");
          return 1;
        }
      }
/*--------------------------------------------------------------------------*/
/* Comment */
      else if (!strcmp (key, "COMMENT"))
        xprintf ("  Comment: %s", p);
/*--------------------------------------------------------------------------*/
/* Dimension */
      else if (!strcmp (key, "DIMENSION"))
      { int n;
        if (sscanf (p, "%s", field) == EOF) {
          put_err_msg ( "ERROR in NUMBER_OF_NODES line\n");
          return 1;
        }
        str2int(field, &n);
        prob->n = n;
        data->n = n;
        xprintf ("  Number of Nodes: %d\n", prob->n);
        compass_tsp_init_prob(prob);
        compass_op_init_prob(prob);
      }
    }
  }

  struct tsp_prob *tsp = prob->tsp;
  struct op_prob *op = prob->op;
  rewind(fp->file);
  while (fgets (buf, 254,  (FILE *) fp->file) != (char *) NULL)
  { p = buf;
    while (*p != '\0')
    { if (*p == ':')
        *p = ' ';
      p++;
    }
    p = buf;
    if (sscanf (p, "%s", key) != EOF)
    { p += strlen (key);
      while (*p == ' ')
        p++;
#if 0
/*--------------------------------------------------------------------------*/
/* TSP Solution */
      if (!strcmp (key, "TSP_SOLUTION"))
      { double tspsol;
        if (sscanf (p, "%s", field) == EOF)
          xprintf ("Not explicit TSP_SOLUTION\n");
        str2num (field, &tspsol );
        prob->tsp->sol->val = tspsol;
        prob->tsp->sol_stat = COMPASS_OPT;
        xprintf ("  TSP solution: %.2f\n", prob->tsp->sol->val);
      }
#endif
/*--------------------------------------------------------------------------*/
/* D0 */
      if (!strcmp (key, "COST_LIMIT"))
      { double d0;
        if (sscanf (p, "%s", field) == EOF)
          xprintf ("Not explicit COST_LIMIT\n");
        str2num (field, &d0);
        prob->op->d0 = d0;
        xprintf ("  Cost limit: %.2f\n", op->d0);
      }
/*--------------------------------------------------------------------------*/
/* Edge Weight Type */
      else if (!strcmp (key, "EDGE_WEIGHT_TYPE"))
      { if (sscanf (p, "%s", field) == EOF) {
          put_err_msg ( "ERROR in EDGE_WEIGHT_TYPE line\n");
          return 1;
        }
        if (!strcmp (field, "EXPLICIT"))
        { data->norm = CC_MATRIXNORM;
          xprintf ("  Explicit Lengths (CC_MATRIXNORM)\n");
        }
        else if (!strcmp (field, "EUC_2D"))
        { data->norm = CC_EUCLIDEAN;
          xprintf ("  Rounded Euclidean Norm (CC_EUCLIDEAN)\n");
        }
        else if (!strcmp (field, "EUC_3D"))
        { data->norm = CC_EUCLIDEAN_3D;
          xprintf ("  Rounded Euclidean 3D Norm (CC_EUCLIDEAN_3D)\n");
        }
        else if (!strcmp (field, "MAX_2D"))
        { data->norm = CC_MAXNORM;
          xprintf ("  Max Norm (CC_MAXNORM)\n");
        }
        else if (!strcmp (field, "MAN_2D"))
        { data->norm = CC_MANNORM;
          xprintf ("  Max Norm (CC_MAXNORM)\n");
        }
        else if (!strcmp (field, "GEO"))
        { data->norm = CC_GEOGRAPHIC;
          xprintf ("  Geographical Norm (CC_GEOGRAPHIC)\n");
        }
        else if (!strcmp (field, "GEOM"))
        { data->norm = CC_GEOM;
          xprintf ("  Geographical Norm in Meters (CC_GEOM)\n");
        }
        else if (!strcmp (field, "ATT"))
        { data->norm = CC_ATT;
          xprintf ("  ATT Norm (CC_ATT)\n");
        }
        else if (!strcmp (field, "CEIL_2D"))
        { data->norm = CC_EUCLIDEAN_CEIL;
          xprintf ("  Rounded Up Euclidean Norm (CC_EUCLIDEAN_CEIL)\n");
        }
        else if (!strcmp (field, "DSJRAND"))
        { data->norm = CC_DSJRANDNORM;
          xprintf ("  David Johnson Random Norm (CC_DSJRANDNORM)\n");
        }
        else
        { put_err_msg ( "ERROR: Not set up for norm %s\n", field);
          return 1;
        }
        compass_data_set_norm(data, data->norm);
      }
/*--------------------------------------------------------------------------*/
/* Edge Weight Format */
      else if (!strcmp (key, "EDGE_WEIGHT_FORMAT"))
      { if (sscanf (p, "%s", field) == EOF) {
          put_err_msg ( "ERROR in EDGE_WEIGHT_FORMAT line\n");
          return 1;
        }
        if (!strcmp (field, "LOWER_DIAG_ROW"))
          matrixform = MATRIX_LOWER_DIAG_ROW;
        else if (!strcmp (field, "UPPER_ROW"))
          matrixform = MATRIX_UPPER_ROW;
        else if (!strcmp (field, "UPPER_DIAG_ROW"))
          matrixform = MATRIX_UPPER_DIAG_ROW;
        else if (!strcmp (field, "FULL_MATRIX"))
           matrixform = MATRIX_FULL_MATRIX;
        else if (strcmp (field, "FUNCTION")) {
           put_err_msg ( "Cannot handle format: %s\n", field);
           return 1;
        }
      }
/*--------------------------------------------------------------------------*/
/* Node score format */
      else if (!strcmp (key, "NODE_SCORE_FORMAT"))
      { if (sscanf (p, "%s", field) == EOF)
        { put_err_msg ( "ERROR in NODE_SCORE_FORMAT line\n");
          return 1;
        }
        if (!strcmp (field, "FIXED_NODE_SCORES"))
        { scoresform = SCORES_FIXED;
          xprintf ("Fixed Node scores\n");
        }
        else if (!strcmp (field, "FST_RAND"))
        { scoresform = SCORES_FSGRANDOM;
          xprintf ("FST, Random Scores (CC_FSGRANDSCORES)\n");
        }
        else if (strcmp (field, "FUNCTION"))
        { put_err_msg ( "Cannot handle format: %s\n", field);
          return 1;
        }
      }
/*--------------------------------------------------------------------------*/
/* Node coordinates section */
      else if (!strcmp (key, "NODE_COORD_SECTION"))
      { int i;
        if (prob->n <= 0)
        { put_err_msg ( "ERROR: Dimension not specified\n");
          return 1;
        }
        if (data->x != (double *) NULL)
        { put_err_msg ( "ERROR: A second NODE_COORD_SECTION?\n");
          compass_erase_prob(prob);
          return 1;
        }
        if ((data->norm & CC_NORM_SIZE_BITS) == CC_D2_NORM_SIZE)
        { data->x = xcalloc (prob->n, sizeof(double));
          if (!data->x)
          { compass_erase_prob(prob);
            return 1;
          }
          data->y = xcalloc (prob->n, sizeof(double));
          if (!data->y)
          { compass_erase_prob(prob);
            return 1;
          }
          for (i = 0; i < prob->n; i++)
           fscanf ((FILE *)fp->file, "%*d %lf %lf", &(data->x[i]), &(data->y[i]));
        } else if ((data->norm & CC_NORM_SIZE_BITS) == CC_D3_NORM_SIZE)
        { data->x = xcalloc (prob->n, sizeof(double));
          if (!data->x)
          { compass_erase_prob(prob);
            return 1;
          }
          data->y = xcalloc(prob->n, sizeof(double));
          if (!data->y)
          { compass_erase_prob(prob);
            return 1;
          }
          data->z = xcalloc(prob->n, sizeof(double));
          if (!data->z)
          { compass_erase_prob(prob);
            return 1;
          }
          for (i = 0; i < prob->n; i++)
           fscanf ((FILE *)fp->file, "%*d %lf %lf %lf", &(data->x[i]),
               &(data->y[i]), &(data->z[i]));
        }
      }
/*--------------------------------------------------------------------------*/
/* Node scores section */
      else if (!strcmp (key, "NODE_SCORE_SECTION"))
      { int i;
        if (prob->n <= 0)
        { put_err_msg ( "ERROR: Dimension not specified\n");
          return 1;
        }
#if 0
        if (op->s != (double *) NULL)
        { put_err_msg ( "ERROR: A second NODE_SCORES_SECTION?\n");
          compass_erase_prob(prob);
          return 1;
        }
#endif
        if (scoresform == SCORES_FIXED)
        { op->s = xcalloc (prob->n, sizeof(double));
          if (!op->s)
          { compass_erase_prob(prob);
            return 1;
          }
          for (i = 0; i < prob->n; i++)
            fscanf ((FILE *)fp->file, "%*d %lf", &(op->s[i]));
        }
        if (scoresform == SCORES_FSGRANDOM)
        { put_err_msg ( "FSG_RAND not implemented yet!\n");
          return 1;
        }
      }
/*--------------------------------------------------------------------------*/
/* Edge weight section */
      else if (!strcmp (key, "EDGE_WEIGHT_SECTION"))
      { int i, j;
        if (prob->n <= 0) {
          put_err_msg ( "ERROR: Dimension not specified\n");
          return 1;
        }
        if (data->adj != (int **) NULL)
        { put_err_msg ( "ERROR: A second NODE_COORD_SECTION?\n");
          compass_erase_prob(prob);
          return 1;
        }
        if ((data->norm & CC_NORM_SIZE_BITS) == CC_MATRIX_NORM_SIZE)
        { data->adj = xcalloc (prob->n, sizeof(int *));
          data->adjspace = xcalloc ((prob->n)*(prob->n+1)/2, sizeof(int));
          if (data->adj == (int **) NULL || data->adjspace == (int *) NULL)
          { compass_erase_prob(prob);
            return 1;
          }
          for (i = 0, j = 0; i < prob->n; i++)
          { data->adj[i] = data->adjspace + j;
            j += (i+1);
          }
          if (matrixform == MATRIX_LOWER_DIAG_ROW)
          { for (i = 0; i < prob->n; i++)
            { for (j = 0; j <= i; j++)
                fscanf ((FILE *)fp->file, "%d", &(data->adj[i][j]));
            }
          } else if (matrixform == MATRIX_UPPER_ROW ||
              matrixform == MATRIX_UPPER_DIAG_ROW ||
              matrixform == MATRIX_FULL_MATRIX)
          {
            int **tempadj = (int **) NULL;
            int *tempadjspace = (int *) NULL;
            tempadj = xcalloc (prob->n, sizeof(int *) );
            tempadjspace = xcalloc ((prob->n) * (prob->n), sizeof(int));
            if (tempadj == (int **) NULL || tempadjspace == (int *) NULL)
            {
              xfree (tempadj);
              xfree (tempadjspace);
              compass_erase_prob(prob);
              return 1;
            }
            for (i = 0; i < prob->n; i++)
            { tempadj[i] = tempadjspace + i * (prob->n);
              if (matrixform == MATRIX_UPPER_ROW)
              { tempadj[i][i] = 0;
                for (j = i + 1; j < prob->n; j++)
                  fscanf ((FILE *)fp->file, "%d", &(tempadj[i][j]));
              }
              else if (matrixform == MATRIX_UPPER_DIAG_ROW)
              { for (j = i; j < prob->n; j++)
                  fscanf ((FILE *)fp->file, "%d", &(tempadj[i][j]));
              }
              else
              {
                for (j = 0; j < prob->n; j++)
                  fscanf ((FILE *)fp->file, "%d", &(tempadj[i][j]));
              }
            }
            for (i = 0; i < prob->n; i++)
            { for (j = 0; j <= i; j++)
                data->adj[i][j] = tempadj[j][i];
            }
            xfree (tempadjspace);
            xfree (tempadj);
          }
        }
        else
        { put_err_msg ( "ERROR: Matrix with norm %d?\n", data->norm);
          return 1;
        }
      }
/*--------------------------------------------------------------------------*/
/* Fixed edges section */
      else if (!strcmp (key, "FIXED_EDGES_SECTION"))
      { put_err_msg ( "ERROR: Not set up for fixed edges\n");
        return 1;
      }
    }
  }

  if (data->x == (double *) NULL && data->adj == (int **) NULL)
  { put_err_msg ( "ERROR: Didn't find the data\n");
    return 1;
  }
  else
    return 0;
}


int compass_read_prob(compass_prob *prob, int flags, const char *fname)
{ compass_data *data = prob->data;
  compass_file *fp;
  if (prob == NULL || prob->magic != COMPASS_PROB_MAGIC)
  { put_err_msg("glp_read_prob: prob = %p; invalid problem object\n", prob);
    goto done;
  }
  if (fname == NULL)
  { put_err_msg("glp_read_prob: fname = %d; invalid parameter\n", fname);
    goto done;
  }
  xprintf("\n");
  xprintf("Reading problem data from '%s'...\n", fname);
  fp = compass_open(fname, "r");
  if (fp == NULL)
  { xprintf("Unable to open '%s' - %s\n", fname, get_err_msg());
    goto done;
  }
  /* read problem line */
  if (flags == FMT_LIB_FILE)
    read_lib(prob, fp);


  compass_close(fp);
done:
   return 0;
}

/*******************************************************************************
*  NAME
*
*  compass_write_op_sol - write Orienteering Problem solution
*
*  SYNOPSIS
*
*  int compass_write_op_sol (op_prob *prob, const char *fname);
*
*  DESCRIPTION
*
*  The routine compass_write_op_sol writes the solution to a text file.
*
*  The character string fname specifies a name of the text file to be
*  written.
*
*  RETURNS
*
*  If the operation was successful, the routine compass_write_op_sol returns
*  zero. Otherwise, it prints an error message and returns non-zero. */

int compass_write_op_sol(compass_prob *prob, const char *fname)
{ int ret, i;
  compass_file *fp;

  struct op_solution *sol = prob->op->sol;

  if (prob->op->sol == NULL) {
    xprintf("No OP solution to write.\n");
  }

  xprintf("\n");
  xprintf("Writing OP solution to '%s'...\n", fname);

  fp = compass_open(fname, "w");
  if (fp == NULL)
  {  xprintf("Unable to create '%s' - %s\n", fname, get_err_msg());
     ret = 1;
     goto done;
  }

  xfprintf (fp, "NAME : %s\n", prob->name );
  xfprintf (fp, "TYPE : OP\n");
  xfprintf (fp, "DIMENSION : %d\n", prob->n );
  xfprintf (fp, "COST_LIMIT : %.2f\n", prob->op->d0 );
  xfprintf (fp, "ROUTE_NODES : %d\n", sol->ns );
  xfprintf (fp, "ROUTE_SCORE : %.2f\n", sol->val );
  xfprintf (fp, "ROUTE_COST : %.2f\n", sol->length );
  xfprintf (fp, "NODE_SEQUENCE_SECTION\n");
  for ( i=0; i< sol->ns; i++)
    xfprintf (fp, "%d\n", sol->cycle[i] +1 );
  xfprintf (fp, "-1\n");
  xfprintf (fp, "DEPOT_SECTION\n");
  xfprintf (fp, "%d\n",prob->op->from+1);
  xfprintf (fp, "-1\n");
  xfprintf (fp, "EOF\n");

  ret = 0;
done:

  if (fp != NULL)
    compass_close(fp);

  return ret;
}

/*******************************************************************************
*  NAME
*
*  compass_write_op_stats - write Orienteering Problem execution stats
*
*  SYNOPSIS
*
*  int compass_write_op_stats (op_prob *prob, const op_opcp *parm,
*     const char *fname);
*
*  DESCRIPTION
*
*  The routine compass_write_op_stats writes problem execution stats to a
*  text file.
*
*  The parameter parm is a pointer to the structure op_opcp, which
*  specifies control parameters used by the Orienteering problem solver routine.
*
*  The character string fname specifies a name of the text file to be
*  written.
*
*  RETURNS
*
*  If the operation was successful, the routine compass_write_op_stats returns
*  zero. Otherwise, it prints an error message and returns non-zero. */


int compass_write_op_stats(compass_prob *prob, struct op_cp *opcp,
    const char *fname)
{ int ret, recno, i;
  compass_file *fp;
  struct op_prob *op = prob->op;
  xprintf("\n");
  xprintf("Writing excution stats to '%s'...\n", fname);
  if (prob->op == NULL) {
    xprintf("No OP problem to write.\n");
  }

  fp = compass_open(fname, "a");
  if (fp == NULL)
  {  xprintf("Unable to create '%s' - %s\n", fname, get_err_msg());
     ret = 1;
     goto done;
  }

  unsigned long flen = (unsigned long)ftell( (FILE *)(fp->file));
  if (flen == 0 )
    xfprintf (fp, "name,\tn,\ttsp,\tnorm,\td0,\tobj,\tlen,\tvis,\ttime,"
        "\tneighg,\tneighk,\ttour,\ttspls,\tprepr,\tadd,\tdrop,\tnpop,\tINITtech,\tINITsel,\tINITpgreedy,\tINITpinit,\tEAnpar,\tEAimpr1,\tEAimpr2,\tEAd2d,\tEAstoppop,\tEApmut,\tINITobj,\tINITvis,\tINITtime,\tEAit,\tEAobj,\tEAtime,\thash\n");
  /* write problem records */
  xfprintf (fp, "%s,\t", prob->name );
  xfprintf (fp, "%d,\t", prob->n );
  xfprintf (fp, "%.0f,\t", prob->tsp->sol->val );
  xfprintf (fp, "%d,\t", prob->data->norm );
  /* write op solution info */
  xfprintf (fp, "%.0f,\t", prob->op->d0 );
  xfprintf (fp, "%.0f,\t", prob->op->sol->val );
  //xfprintf (fp, "%.0f,\t", prob->op->population->mean_val );
  xfprintf (fp, "%.0f,\t", prob->op->sol->length );
  xfprintf (fp, "%d,\t", prob->op->sol->ns );
  xfprintf (fp, "%.2f,\t", xdifftime( opcp->tm_end, opcp->tm_start ));
  /* write ea execution info */
  xfprintf (fp, "%d,\t", opcp->tspcp->neighcp->neigh_graph);
  xfprintf (fp, "%d,\t", opcp->tspcp->neighcp->k);
  xfprintf (fp, "%d,\t", opcp->tspcp->start_tour );
  xfprintf (fp, "%d,\t", opcp->tspcp->local_search );
  xfprintf (fp, "%d,\t", opcp->pp_tech );
  xfprintf (fp, "%d,\t", opcp->add );
  xfprintf (fp, "%d,\t", opcp->drop );
  xfprintf (fp, "%d,\t", opcp->pop_size );
  xfprintf (fp, "%d,\t", opcp->initcp->init_tech );
  xfprintf (fp, "%d,\t", opcp->initcp->sel_tech );
  xfprintf (fp, "%.2f,\t", opcp->initcp->pgreedy );
  xfprintf (fp, "%.2f,\t", opcp->initcp->pinit );
  xfprintf (fp, "%d,\t", opcp->eacp->nparsel );
  xfprintf (fp, "%d,\t", opcp->eacp->len_improve1 );
  xfprintf (fp, "%d,\t", opcp->eacp->len_improve2 );
  xfprintf (fp, "%d,\t", opcp->eacp->d2d );
  xfprintf (fp, "%d,\t", opcp->stop_pop );
  xfprintf (fp, "%.2f,\t", opcp->eacp->pmut );
  xfprintf (fp, "%.0f,\t", opcp->initcp->best->val );
  xfprintf (fp, "%d,\t", opcp->initcp->best->ns );
  xfprintf (fp, "%.2f,\t", xdifftime(opcp->initcp->tm_end, opcp->initcp->tm_start ));
  xfprintf (fp, "%d,\t", opcp->eacp->it );
  xfprintf (fp, "%.0f,\t", opcp->eacp->best->val );
  xfprintf (fp, "%.2f,\t", xdifftime(opcp->eacp->tm_end, opcp->eacp->tm_start ));
  for (i=0; i < 32; i++)
    xfprintf(fp, "%02x",prob->hash[i]);
  xfprintf (fp, "\n");
  if (compass_ioerr(fp))
  { xprintf("Write error on '%s' - %s\n", fname, get_err_msg());
    ret = 1;
    goto done;
  }
  ret = 0;
done:
  if (fp != NULL)
    compass_close(fp);
  return ret;
}
