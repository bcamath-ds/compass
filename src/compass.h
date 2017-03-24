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

#ifndef COMPASS_H
#define COMPASS_H

#include "data/data.h"
#include "gsl/gsl_rng.h"
#include <openssl/sha.h>

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* library version numbers: */
#define COMPASS_MAJOR_VERSION  0
#define COMPASS_MINOR_VERSION  1

#define COMPASS_PROB_MAGIC 0xD7D9D6C2

#define COMPASS_UNDEF   0   /* integer solution is undefined */
#define COMPASS_OPT     1   /* integer solution is optimal */
#define COMPASS_FEAS    2   /* integer solution is feasible */
#define COMPASS_NOFEAS  3   /* no integer solution exists */

#define COMPASS_SOLVE_TSP 0
#define COMPASS_SOLVE_OP  1

#define HASH_UPDATE_NAME      0
#define HASH_UPDATE_OPD0      1
#define HASH_UPDATE_OPSCORE   2

typedef struct compass_prob compass_prob;
typedef struct CCkdtree CCkdtree;
typedef struct CCrandstate CCrandstate;
typedef struct RNG RNG;

struct compass_prob
{ /* OP problem object */
  unsigned      magic;
  /* magic value used for debugging */
  void           *pool;
  /* memory pool to store problem object components */
  char          *name;
  unsigned char *hash;
  SHA256_CTX    *ctx;
  /* problem name (1 to 255 chars); NULL means no name is assigned
  to the problem */
  int           n_max;
  /* length of the array of nodes (enlarged automatically) */
  int           n;
  /* number of nodes */
  compass_data *data;
  /* compass data object */
  int           *cacheind;
  int           *cacheval;
  int           cacheM;
  /* Distance cache */
  CCkdtree  *kdtree;
  /* compass data object */
  int seed;
  /* seed value to be passed to the MathProg translator; initially
     set to 1; 0x80000000 means the value is omitted */
  CCrandstate       *rstate_cc;
  gsl_rng           *rstate_gsl;
  RNG               *rstate;
  struct op_prob      *op;
  struct tsp_prob     *tsp;
};

struct csa
{ /* common storage area */
  compass_prob *prob;
  /* OP problem object */
  double tm_start;
  /* starting timestamp */
  double tm_end;
  /* ending timestamp */
  double tm_lim;
  /* time limit (milliseconds) */
  struct neigh_cp *neighcp;
  /* TSP control parameters */
  struct tsp_cp *tspcp;
  /* TSP control parameters */
  struct op_cp *opcp;
  /* Orienteering Problem control parameters */
  /* Branch and Cut control parameters */
  int format;
  /* problem file format: */
#define FMT_MPS_DECK    1  /* fixed MPS */
#define FMT_MPS_FILE    2  /* free MPS */
#define FMT_LP          3  /* CPLEX LP */
#define FMT_GLP         4  /* GLPK LP/MIP */
#define FMT_MATHPROG    5  /* MathProg */
#define FMT_LIB_FILE    6  /* TSP/OP LIB */
  const char *in_file;
  /* name of input problem file */
#define DATA_MAX 10
  /* maximal number of input data files */
  int ndf;
  /* number of input data files specified */
  const char *in_data[1+DATA_MAX];
  /* name(s) of input data file(s) */
  const char *out_dpy;
  /* name of output file to send display output; NULL means the
     display output is sent to the terminal */
  int seed;
  /* seed value to be passed to the MathProg translator; initially
     set to 1; 0x80000000 means the value is omitted */
  int hash_tm;
  /* hash problem using time stamp */
  const char *in_res;
  /* name of input solution file in raw format */
  int scale;
  /* automatic problem scaling flag */
  const char *out_sol;
  /* name of output solution file in printable format */
  const char *out_res;
  /* name of output solution file in raw format */
  const char *out_ranges;
  /* name of output file to write sensitivity analysis report */
  int check;
  /* input data checking flag; no solution is performed */
  const char *new_name;
  /* new name to be assigned to the problem */
  const char *out_mps;
  /* name of output problem file in fixed MPS format */
  const char *out_lib;
  /* name of output problem file in LIB format */
  const char *out_freemps;
  /* name of output problem file in free MPS format */
  const char *out_cpxlp;
  /* name of output problem file in CPLEX LP format */
  const char *out_glp;
  /* name of output problem file in GLPK format */
#if 0
  const char *out_pb;
  /* name of output problem file in OPB format */
  const char *out_npb;
  /* name of output problem file in normalized OPB format */
#endif
#if 1 /* 06/VIII-2011 */
  const char *out_cnf;
  /* name of output problem file in DIMACS CNF-SAT format */
#endif
  const char *log_file;
  /* name of output file to hardcopy terminal output */
  const char *stats_file;
  /* name of output file to hardcopy terminal output */
  int crash;
  /* initial basis option: */
#define USE_STD_BASIS   1  /* use standard basis */
#define USE_ADV_BASIS   2  /* use advanced basis */
#define USE_CPX_BASIS   3  /* use Bixby's basis */
#define USE_INI_BASIS   4  /* use initial basis from ini_file */
  const char *ini_file;
  /* name of input file containing initial basis */
  int      norm;
  /*  */
  int solve_tsp;
  /*  */
  int solve_op;
  /* */
  int xcheck;
  /* flag to check final basis with glp_exact */
  int nomip;
  /* flag to consider MIP as pure LP */
};

#endif
