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
#include "util/misc.h"
#include "util/util.h"
#include "tsp/tsp.h"
#include "op/op.h"

int
   main (int, char **);
static int
   print_command (int ac, char **av),
parse_cmdline(struct csa *csa, int argc, char *argv[]);
static void
print_version(int briefly);


int main(int argc, char *argv[])
{ struct csa _csa, *csa = &_csa;
  int ret = 0;
  double time_elapsed;
  size_t tpeak;
  //csa->graph = NULL;
  csa->format = FMT_LIB_FILE;
  csa->tm_start = xtime();
  csa->in_file = NULL;
  csa->stats_file = NULL;
  csa->out_dpy = NULL;
  csa->seed = (int) CCutil_real_zeit ();
  csa->tspcp = compass_tsp_init_cp();
  csa->opcp = compass_op_init_cp();
  csa->neighcp = compass_neigh_init_cp();
  csa->in_res = NULL;
  csa->scale = 1;
  csa->out_sol = NULL;
  csa->out_res = NULL;
  csa->out_ranges = NULL;
  csa->check = 0;
  csa->new_name = NULL;
  csa->out_mps = NULL;
  csa->out_freemps = NULL;
  csa->out_cpxlp = NULL;
  csa->out_glp = NULL;
  csa->log_file = NULL;
  csa->crash = USE_ADV_BASIS;
  csa->ini_file = NULL;
  //csa->exact = 0;
  csa->solve_tsp = 0;
  csa->solve_op = 0;
  csa->xcheck = 0;
  csa->nomip = 0;
  //csa->use_bnd = 0;
  //csa->obj_bnd = 0;
  //csa->use_sol = NULL;
  CCrandstate rstate;
  gsl_rng *rstate_gsl;
  /*--------------------------------------------------------------------------*/
  /* parse command-line parameters */
  ret = parse_cmdline(csa, argc, argv);
  if (ret < 0)
  { ret = EXIT_SUCCESS;
    goto done;
  }
  if (ret > 0)
  { ret = EXIT_FAILURE;
    goto done;
  }
  /*--------------------------------------------------------------------------*/
  /* print version information */
  print_version(1);
  /*--------------------------------------------------------------------------*/
  /* open log file, if required */
  if (csa->log_file != NULL)
  { if (compass_open_tee(csa->log_file))
    { xprintf("Unable to create log file\n");
      ret = EXIT_FAILURE;
      goto done;
    }
  }
  /*--------------------------------------------------------------------------*/
  /* print parameters specified in the command line */
  if (argc > 1)
  { int k, len = INT_MAX;
    xprintf("Parameter(s) specified in the command line:");
    for (k = 1; k < argc; k++)
    { if (len > 72)
        xprintf("\n"), len = 0;
      xprintf(" %s", argv[k]);
      len += 1 + strlen(argv[k]);
    }
    xprintf("\n");
  }
  /*--------------------------------------------------------------------------*/
  /* Initialize problem */
  csa->prob = compass_init_prob();
  /*--------------------------------------------------------------------------*/
  /* read problem from file*/
  if (csa->in_file == NULL)
  { xprintf("No input problem file specified; try %s --help\n", argv[0]);
    ret = EXIT_FAILURE;
    goto done;
  }
  if (csa->format == FMT_LIB_FILE)
  { ret = compass_read_prob(csa->prob, FMT_LIB_FILE, csa->in_file);
    if (ret != 0)
err1: {  xprintf("LIB file processing error\n");
        ret = EXIT_FAILURE;
        goto done;
    }
    csa->opcp->pinit = csa->prob->op->d0 / csa->prob->tsp->sol->val;
  }
  else
  { if (csa->solve_tsp && csa->solve_op )
    { xprintf("You must specify the problem to solve: --tsp or --op\n");
      ret = EXIT_FAILURE;
      goto done;
    }
  }
  /*--------------------------------------------------------------------------*/
  /* change problem name, if required */
  if (csa->new_name != NULL)
    compass_set_prob_name(csa->prob, csa->new_name);
  /******************************************/
  compass_init_rng(csa->prob, csa->seed);
  /*--------------------------------------------------------------------------*/
  /* Build neigh graph */
  compass_data_k_nearest (csa->prob, csa->neighcp );
  /*--------------------------------------------------------------------------*/
  /* solve problems*/
  if (csa->solve_tsp == COMPASS_ON )
    main_tsp(csa, argc, argv);
  if (csa->solve_op == COMPASS_ON )
    main_op(csa, argc, argv);
  /*--------------------------------------------------------------------------*/
  /* Time and memory usage summary*/
  xprintf("\n");
  csa->tm_end = xtime();
  time_elapsed = xdifftime(csa->tm_end, csa->tm_start);
  xprintf("Time used:   %.2f secs\n", time_elapsed);
  compass_mem_usage(NULL, NULL, NULL, &tpeak);
  xprintf("Memory used: %.1f Mb (%.0f bytes)\n",
      (double)tpeak / 1048576.0, (double)tpeak);

#if 0
  if ( csa->stats_file == COMPASS_ON )
  { xprintf("Writing statistics to %s...\n", csa->stats_file);
    compass_write_tsp_stats(prob,
  }
#endif

  if (csa->prob->tsp != NULL)
    compass_tsp_delete_prob(csa->prob);
  if (csa->prob->op != NULL)
    compass_op_delete_prob(csa->prob);
  compass_free_rng(csa->prob);
  if (csa->prob != NULL)
    compass_delete_prob(csa->prob);
  /*--------------------------------------------------------------------------*/
  /* all seems to be ok */
  ret = EXIT_SUCCESS;
  /*--------------------------------------------------------------------------*/
done:
  xfree(csa->neighcp);
  compass_tsp_delete_cp(csa->tspcp);
  compass_op_delete_cp(csa->opcp);
  /* free the Compass environment */
  /* close log file, if necessary */
  if (csa->log_file != NULL) compass_close_tee();
  compass_free_env();

    return ret;
}

static void print_help(const char *my_name)
{     /* print help information */
  xprintf("Usage: %s [options...] filename\n", my_name);
  xprintf("\n");
  xprintf("General options:\n");
  //xprintf("   --mps             read problem in fixed MPS format\n");
  //xprintf("   --freemps         read problem in free MPS format (default)\n");
  //xprintf("   --lp              read problem in CPLEX LP format\n");
  //xprintf("   --glp             read problem in GLPK format\n");
  xprintf("  --problib            Read problem in LIB format (default)\n");
  xprintf("  -d filename, --data  filename\n");
  xprintf("                       Read data from filename;\n");
  xprintf("  -r filename, --read  filename\n");
  xprintf("                       Read solution from filename\n");
  xprintf("  -o filename, --output filename\n");
  xprintf("                       Write solution to filename in printable format"
      "\n");
  xprintf("  -w filename, --write filename\n");
  xprintf("                       Write solution to filename in plain text format"
      "\n");

  xprintf("  --stats filename     Write statistics to filename\n");
  //xprintf("   --ranges filename\n");
  //xprintf("                     write sensitivity analysis report to filename in"
  //    "\n printable format (simplex only)\n");
  xprintf("  --tmlim nnn          Limit solution time to nnn seconds\n");
  xprintf("  --memlim nnn         Limit available memory to nnn megabytes\n");
  xprintf("  --check              Do not solve problem, check input data only\n");
  xprintf("  --name probname      Change problem name to probname\n");
  xprintf("  --wmps filename      Write problem to filename in fixed MPS format\n");
  xprintf("  --wfreemps filename\n");
  xprintf("                       Write problem to filename in free MPS format\n");
  xprintf("  --wlp filename       Write problem to filename in CPLEX LP format\n");
  xprintf("  --wglp filename      Write problem to filename in GLPK format\n");
  xprintf("  --wop filename       Write OP to filename in TSPLIB format\n");
  xprintf("  --log filename       Write copy of terminal output to filename\n");
  xprintf("  -h, --help           Display this help information and exit\n");
  xprintf("  --version            Display program version and exit\n");
  xprintf("  -v, -vv(vv)          Display output. You can increase verbosity (i.e. -vv)\n");
  xprintf("  --nruns n            Number of runs\n");
  xprintf("\n");
  xprintf("  (GEO) General options:\n");
  xprintf("  --norm #            Norm (must specify if dat file is not a TSPLIB file)\n");
  xprintf("                       0=MAX, 1=L1, 2=L2, 3=3D, 4=USER, 5=ATT, 6=GEO, 7=MATRIX,\n");
  xprintf("                       8=DSJRAND, 9=CRYSTAL, 10=SPARSE, 11-15=RH-norm 1-5,\n");
  xprintf("                       16=TOROIDAL ,17=GEOM, 18=JOHNSON\n");
  xprintf("  --neigh-set #       Neighbor set technique\n"
          "                        %d-Nearest[default], %d-Quadnearest, %d-Delaunay\n",
                                    NEIGH_NEAREST, NEIGH_QUADNEAREST,
                                    NEIGH_DELAUNAY);
  xprintf("  --scale              Scale problem (default)\n");
  xprintf("  --noscale            Do not scale problem\n");
  xprintf("\n");
  xprintf("Traveller Salesman Problem options:\n");
  xprintf("\n");
  xprintf("  (TSP) General options:\n");
  xprintf("  --tsp                Solve the Traveller Salesman Problem\n");
  xprintf("  --tsp-start-tour     Generate starting cycle\n"
          "                        %d-Random, %d-NNeigh, %d-Greedy, %d-Boruvka,\n"
          "                        %d-QBoruvka[default]\n",
                                    TSP_RANDOM_TOUR, TSP_NEIGHBOR_TOUR,
                                    TSP_GREEDY_TOUR, TSP_BORUVKA_TOUR, TSP_QBORUVKA_TOUR);
  xprintf("  --tsp-local-search   Local search technique\n");
  xprintf("                        (%d) No local search\n", TSP_NO_LS);
  xprintf("                        (%d) 2-opt\n", TSP_TWOOPT_LS);
  xprintf("                        (%d) 2.5-opt\n", TSP_TWOOPT5_LS);
  xprintf("                        (%d) 3-opt\n", TSP_THREEOPT_LS);
  xprintf("                        (%d) Lin-Kernighan\n", TSP_LINKERN_LS);
  xprintf("\n");
  xprintf("  (TSP) Iterated Lin-Kernighan options:\n");
  xprintf("  --kick #             kick Type \n"
          "                       %d-Random, %d-Geometric, %d-Close, %d-Random_Walk [default]\n",
                                    CC_LK_RANDOM_KICK, CC_LK_GEOMETRIC_KICK,
                                    CC_LK_CLOSE_KICK, CC_LK_WALK_KICK);
  xprintf("  --nkicks n           number of kicks (default is #nodes)\n");
  xprintf("\n");
  xprintf("Orienteering Problem options:\n");
  xprintf("\n");
  xprintf("  (OP) General options:\n");
  xprintf("  --op                 Solve the Orienteering Problem\n");
  xprintf("  --op-d0              Distance limit\n");
  xprintf("  --op-from            Departure node\n");
  xprintf("  --op-to              Arraiving node (default from=to)\n");
  xprintf("  --op-add t           Add phase sorting criteria\n");
  xprintf("  --op-drop t          Drop phase sorting criteria\n");
  xprintf("  --op-pinit p         Proportion of selected nodes at initialization\n");
  xprintf("  --op-pgreedy p       Greediness parameter\n");
  xprintf("\n");
  xprintf("  (OP) Genetic Algorithm options:\n");
  xprintf("  --ea                 Use EA approach\n");
  //xprintf("   --beta b        Beta parameter\n");
  xprintf("  --ga-itlim n         Number of iterations\n");
  xprintf("  --ga-improve1 d      1 if on. 0 else.\n");
  xprintf("  --ga-improve2 d      1 if On. 0 else.\n");
  xprintf("  --ga-d2d it          Number of iterations between add/drop phase"
      "s\n");
  xprintf("  --pop-size p         Population size\n");
  xprintf("  --stop-pop p         Population based stopping criteria (perc)\n");
  xprintf("  --ga-pmut p          Use mutation p probability\n");
  xprintf("  --ga-nparsel n       Number of parents preselected\n");
  //xprintf("   --exact           use simplex method based on exact arithmetic\n");
  xprintf("\n");
#if 0
  xprintf("   (OP) 2-paramenter Iteractive Algorithm options:\n");
  xprintf("   --2pia              Use 2p-IA method\n");
  xprintf("\n");
  xprintf("MIP solver options:\n");
  xprintf("   --mip               Use MIP method\n");
  xprintf("   --nomip             consider all integer variables as c"
     "ontinuous (LP)\n");
  xprintf("                       (allows solving MIP as pure LP)\n");
  xprintf("   --first             branch on first integer variable\n")
     ;
  xprintf("   --last              branch on last integer variable\n");
  xprintf("   --mostf             branch on most fractional variable "
     "\n");
  xprintf("   --mipgap tol        set relative mip gap tolerance to t"
     "ol\n");
  xprintf("\n");
#endif
  xprintf("For description of the TSPLIB format see Reference Manual.\n"
    "<http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/DOC.PS>.\n");
  xprintf("\n");
  xprintf("Please report bugs to <gkobeaga@bcamath.org>.\n");
  return;
}

static void print_version(int briefly)
{ /* print version information */
  xprintf("Compass: Combinatorial Optimization Problem Solver, v%s\n",
      compass_version());
  if (briefly) goto done;
  xprintf("\n");
  xprintf("This program has ABSOLUTELY NO WARRANTY.\n");
  xprintf("\n");
  xprintf("This program is free software; you may re-distribute it under the"
     "terms\n");
  xprintf("of the GNU General Public License version 2 or later.\n");
done:
  xprintf("\n");
  return;
}

static int parse_cmdline(struct csa *csa, int argc, char *argv[])
{ /* parse command-line parameters */
  int k;
#define p(str) (strcmp(argv[k], str) == 0)
  for (k = 1; k < argc; k++)
    { if (p("--mps"))
      csa->format = FMT_MPS_DECK;
    else if (p("--lib"))
      csa->format = FMT_LIB_FILE;
    else if (p("--glp"))
      csa->format = FMT_GLP;
    else if (p("--math") || p("-m") || p("--model"))
      csa->format = FMT_MATHPROG;
    else if (p("-d") || p("--data"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No input data file specified\n");
        return 1;
      }
      if (csa->ndf == DATA_MAX)
      { xprintf("Too many input data files\n");
        return 1;
      }
      csa->in_data[++(csa->ndf)] = argv[k];
    }
    else if (p("--seed"))
    { k++;
      if (k == argc || argv[k][0] == '\0' ||
          argv[k][0] == '-' && !isdigit((unsigned char)argv[k][1]))
      { xprintf("No seed value specified\n");
        return 1;
      }
      if (strcmp(argv[k], "?") == 0)
        csa->seed = 0x80000000;
      else if (str2int(argv[k], &csa->seed))
      { xprintf("Invalid seed value '%s'\n", argv[k]);
        return 1;
      }
    }
    else if ( p("--stats"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No stats file specified\n");
        return 1;
      }
      if (csa->stats_file != NULL)
      { xprintf("Only one stats file allowed\n");
        return 1;
      }
      csa->stats_file = argv[k];
    }
#if 0
         else if (p("-r") || p("--read"))
         {  k++;
            if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
            {  xprintf("No input solution file specified\n");
               return 1;
            }
            if (csa->in_res != NULL)
            {  xprintf("Only one input solution file allowed\n");
               return 1;
            }
            csa->in_res = argv[k];
         }
#endif
    else if (p("-o") || p("--output"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No output solution file specified\n");
        return 1;
      }
      if (csa->out_sol != NULL)
      { xprintf("Only one output solution file allowed\n");
        return 1;
      }
      csa->out_sol = argv[k];
    }
    else if (p("-w") || p("--write"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No output solution file specified\n");
        return 1;
      }
      if (csa->out_res != NULL)
      { xprintf("Only one output solution file allowed\n");
        return 1;
      }
      csa->out_res = argv[k];
    }
    else if (p("--tmlim"))
    { int tm_lim;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No time limit specified\n");
        return 1;
      }
      if (str2int(argv[k], &tm_lim) || tm_lim < 0)
      { xprintf("Invalid time limit '%s'\n", argv[k]);
        return 1;
      }
      if (tm_lim <= INT_MAX / 1000)
        csa->tm_lim = 1000 * tm_lim;
      else
        csa->tm_lim = INT_MAX;
    }
    else if (p("--memlim"))
    { int mem_lim;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No memory limit specified\n");
        return 1;
      }
      if (str2int(argv[k], &mem_lim) || mem_lim < 1)
      { xprintf("Invalid memory limit '%s'\n", argv[k]);
        return 1;
      }
      compass_mem_limit(mem_lim);
    }
    else if (p("--tsp"))
      csa->solve_tsp = COMPASS_ON;
    else if (p("--op"))
      csa->solve_op = COMPASS_ON;
    else if (p("--check"))
      csa->check = 1;
    else if (p("--name"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No problem name specified\n");
        return 1;
      }
      if (csa->new_name != NULL)
      { xprintf("Only one problem name allowed\n");
        return 1;
      }
      csa->new_name = argv[k];
    }
    else if (p("--wmps"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No fixed MPS output file specified\n");
        return 1;
      }
      if (csa->out_mps != NULL)
      { xprintf("Only one fixed MPS output file allowed\n");
        return 1;
      }
      csa->out_mps = argv[k];
    }
    else if (p("--wfreemps"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No free MPS output file specified\n");
        return 1;
      }
      if (csa->out_freemps != NULL)
      { xprintf("Only one free MPS output file allowed\n");
        return 1;
      }
      csa->out_freemps = argv[k];
    }
    else if (p("--wlib"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("TSP/OPLIB output file specified\n");
        return 1;
      }
      if (csa->out_glp != NULL)
      { xprintf("Only one TSP/OPLIB output file allowed\n");
        return 1;
      }
      csa->out_lib = argv[k];
    }
    else if (p("--wglp"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No GLPK LP/MIP output file specified\n");
        return 1;
      }
      if (csa->out_glp != NULL)
      { xprintf("Only one GLPK LP/MIP output file allowed\n");
        return 1;
      }
      csa->out_glp = argv[k];
    }
    else if (p("--log"))
    { k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No log file specified\n");
        return 1;
      }
      if (csa->log_file != NULL)
      { xprintf("Only one log file allowed\n");
        return 1;
      }
      csa->log_file = argv[k];
    }
    else if (p("-h") || p("--help"))
    { print_help(argv[0]);
      return -1;
    }
    else if (p("--xcheck"))
      csa->xcheck = 1;
    /*------------------------------------------------------------------------*/
    /* Geometric parameters */
    else if (p("--norm"))
    { int inorm;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No norm specified\n");
        return 1;
      }
      if (str2int(argv[k], &inorm) || inorm < 0)
      { xprintf("Invalid norm '%s'\n", argv[k]);
        return 1;
      }
      switch (inorm) {
      case 0: csa->norm = CC_MAXNORM; break;
      case 1: csa->norm = CC_MANNORM; break;
      case 2: csa->norm = CC_EUCLIDEAN; break;
      case 3: csa->norm = CC_EUCLIDEAN_3D; break;
      case 4: csa->norm = CC_USER; break;
      case 5: csa->norm = CC_ATT; break;
      case 6: csa->norm = CC_GEOGRAPHIC; break;
      case 7: csa->norm = CC_MATRIXNORM; break;
      case 8: csa->norm = CC_DSJRANDNORM; break;
      case 9: csa->norm = CC_CRYSTAL; break;
      case 10: csa->norm = CC_SPARSE; break;
      case 11: csa->norm = CC_RHMAP1; break;
      case 12: csa->norm = CC_RHMAP2; break;
      case 13: csa->norm = CC_RHMAP3; break;
      case 14: csa->norm = CC_RHMAP4; break;
      case 15: csa->norm = CC_RHMAP5; break;
      case 16: csa->norm = CC_EUCTOROIDAL; break;
      case 17: csa->norm = CC_GEOM; break;
      case 18: csa->norm = CC_EUCLIDEAN_CEIL; break;
      default:
        xprintf("Invalid norm '%d'\n", inorm);
        print_help (argv[0]);
        return 1;
      }
    }
    else if (p("--neigh-set"))
    { int neigh_set;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No tour improvement for phase 1 specified\n");
        return 1;
      }
      if (str2int(argv[k], &neigh_set) || neigh_set < 0)
      { xprintf("Invalid tour  improvement for phase 1 '%s'\n", argv[k]);
        return 1;
      }
      switch (neigh_set) {
      case NEIGH_NEAREST:     csa->neighcp->neigh_graph = NEIGH_NEAREST; break;
      case NEIGH_QUADNEAREST: csa->neighcp->neigh_graph = NEIGH_QUADNEAREST; break;
      case NEIGH_DELAUNAY:    csa->neighcp->neigh_graph = NEIGH_DELAUNAY; break;
      default:
        xprintf("Invalid tour improvement for phase 1'%d'\n", neigh_set);
        print_help (argv[0]);
        return 1;
      }
      csa->tspcp->neighcp->neigh_graph = csa->neighcp->neigh_graph;
      csa->opcp->tspcp->neighcp->neigh_graph = csa->neighcp->neigh_graph;
    }
    else if (p("--neigh-k"))
    { int neigh_k;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No population size specified\n");
        return 1;
      }
      if (str2int(argv[k], &neigh_k) || neigh_k < 1)
      { xprintf("Invalid population size '%s'\n", argv[k]);
        return 1;
      }
      csa->neighcp->k = neigh_k;
      csa->tspcp->neighcp->k = neigh_k;
      csa->opcp->tspcp->neighcp->k = neigh_k;
    }
    else if (p("--scale"))
      csa->scale = 1;
    else if (p("--noscale"))
      csa->scale = 0;
    /*------------------------------------------------------------------------*/
    /* Population parameters*/
    else if (p("--pop-size"))
    { int size;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No population size specified\n");
        return 1;
      }
      if (str2int(argv[k], &size) || size < 1)
      { xprintf("Invalid population size '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->pop_size = csa->opcp->eacp->pop_size = size;
    }
    else if (p("--stop-pop"))
    { int per;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No stopping percentil specified\n");
        return 1;
      }
      if (str2int(argv[k], &per) || per < 0 || per > 100)
      { xprintf("Invalid stopping percentil '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->stop_pop = per;
    }
    /*------------------------------------------------------------------------*/
    /* TSP solver general parameters*/
    else if (p("--tsp-start-tour"))
    { int start_tour;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No parent preselect number specified\n");
        return 1;
      }
      if (str2int(argv[k], &start_tour) || start_tour < 0)
      { xprintf("Invalid tour initialization '%s'\n", argv[k]);
        return 1;
      }
      switch (start_tour) {
      case TSP_RANDOM_TOUR: csa->tspcp->start_tour = TSP_RANDOM_TOUR; break;
      case TSP_NEIGHBOR_TOUR: csa->tspcp->start_tour = TSP_NEIGHBOR_TOUR; break;
      case TSP_GREEDY_TOUR: csa->tspcp->start_tour = TSP_GREEDY_TOUR; break;
      case TSP_BORUVKA_TOUR: csa->tspcp->start_tour = TSP_BORUVKA_TOUR; break;
      case TSP_QBORUVKA_TOUR: csa->tspcp->start_tour = TSP_QBORUVKA_TOUR; break;
      default:
        xprintf("Invalid tour initialization '%d'\n", start_tour);
        print_help (argv[0]);
        return 1;
      }
      csa->opcp->tspcp->start_tour = csa->tspcp->start_tour;
    }
    else if (p("-ls") || p("--tsp-local-search"))
    { int local_search;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No tour improvement for phase 1 specified\n");
        return 1;
      }
      if (str2int(argv[k], &local_search) || local_search < 0)
      { xprintf("Invalid tour  improvement for phase 1 '%s'\n", argv[k]);
        return 1;
      }
      switch (local_search) {
      case TSP_NO_LS:       csa->tspcp->local_search = TSP_NO_LS; break;
      case TSP_TWOOPT_LS:   csa->tspcp->local_search = TSP_TWOOPT_LS; break;
      case TSP_TWOOPT5_LS:  csa->tspcp->local_search = TSP_TWOOPT5_LS; break;
      case TSP_THREEOPT_LS: csa->tspcp->local_search = TSP_THREEOPT_LS; break;
      case TSP_LINKERN_LS:  csa->tspcp->local_search = TSP_LINKERN_LS; break;
      default:
        xprintf("Invalid tour improvement for phase 1'%d'\n", local_search);
        print_help (argv[0]);
        return 1;
      }
      csa->opcp->tspcp->local_search = csa->tspcp->local_search;
    }
    /*--------------------------------------------------------------------------*/
    /* OP problem*/
    else if (p("--op-d0"))
    { double d0;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No budget limit specified\n");
        return 1;
      }
      if (str2num(argv[k], &d0) || d0 < 0)
      { xprintf("Invalid budget limit '%s'\n", argv[k]);
        return 1;
      }
      csa->prob->op->d0=d0;
    }
    else if (p("--op-from,"))
    { int from;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No departure node specified\n");
        return 1;
      }
      if (str2int(argv[k], &from) || from < 0 || from >= csa->prob->n)
      { xprintf("Invalid departure node '%s'\n", argv[k]);
        return 1;
      }
      csa->prob->op->from = from;
    }
    else if (p("--op-to,"))
    { int to;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No departure node specified\n");
        return 1;
      }
      if (str2int(argv[k], &to) || to < 0 || to >= csa->prob->n)
      { xprintf("Invalid departure node '%s'\n", argv[k]);
        return 1;
      }
      csa->prob->op->to = to;
    }
    else if (p("--op-pinit"))
    { double pinit;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No initialization probability specified\n");
        return 1;
      }
      if (str2num(argv[k], &pinit) || pinit < 0.0 || pinit > 1.0)
      { xprintf("Invalid initialization probability '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->pinit = pinit;
    }
    else if (p("--op-pgreedy"))
    { double pgreedy;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No greediness parameter specified\n");
        return 1;
      }
      if (str2num(argv[k], &pgreedy) || pgreedy < 0.0 || pgreedy > 1.0)
      { xprintf("Invalid greediness parameter '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->pgreedy = pgreedy;
    }
    else if (p("--op-add"))
    { int add;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No sorting rule specified for add phase\n");
        return 1;
      }
      if (str2int(argv[k], &add) || add < 0)
      { xprintf("Invalid sorting rule for the add phase '%s'\n", argv[k]);
        return 1;
      }
      switch (add) {
      case OP_ADD_D: csa->opcp->add = OP_ADD_D;
      case OP_ADD_SD: csa->opcp->add = OP_ADD_SD;
      case OP_ADD_S: csa->opcp->add = OP_ADD_S;
      default:
        xprintf("Invalid sorting rule for the add phase '%s'\n", argv[k]);
        print_help (argv[0]);
        return 1;
      }
    }
    else if (p("--op-drop"))
    { int drop;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No sorting rule specified for drop phase\n");
        return 1;
      }
      if (str2int(argv[k], &drop) || drop < 0)
      { xprintf("Invalid sorting rule for the drop phase '%s'\n", argv[k]);
        return 1;
      }
      switch (drop) {
      case OP_DROP_D: csa->opcp->drop = OP_DROP_D;
      case OP_DROP_SD: csa->opcp->drop = OP_DROP_SD;
      case OP_DROP_S: csa->opcp->drop = OP_DROP_S;
      default:
        xprintf("Invalid sorting rule for the drop phase '%s'\n", argv[k]);
        print_help (argv[0]);
        return 1;
      }
    }
    /*------------------------------------------------------------------------*/
    /* OP solver general parameters*/
    else if (p("--op-preproccess"))
      csa->opcp->pp_tech = OP_PP_ROOT;
    else if (p("--nruns"))
    { int nruns;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No number of runs specified\n");
        return 1;
      }
      if (str2int(argv[k], &nruns) || nruns < 0)
      { xprintf("Invalid number of runs '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->nruns = nruns;
    }
    /*------------------------------------------------------------------------*/
    /* Genetic Algorithm parameters */
    else if (p("--op-ga"))
      csa->opcp->heur_tech = OP_HEUR_EA;
    else if (p("--ga-itlim"))
    { int it_lim;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No iteration limit specified\n");
        return 1;
      }
      if (str2int(argv[k], &it_lim) || it_lim < 0)
      { xprintf("Invalid iteration limit '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->it_lim = it_lim;
    }
    else if (p("--ga-nparsel"))
    { int nparsel;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No parent preselect number specified\n");
        return 1;
      }
      if (str2int(argv[k], &nparsel) || nparsel < 2)
      { xprintf("Invalid parent preselect number '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->nparsel = nparsel;
    }
    else if (p("--ga-pmut"))
    { double pmut;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No mutation probability specified\n");
        return 1;
      }
      if (str2num(argv[k], &pmut) || pmut < 0.0 || pmut > 1.0)
      { xprintf("Invalid mutation probability '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->pmut = pmut;
    }
    else if (p("--ga-improve1"))
    { int status;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No improvent status specified\n");
        return 1;
      }
      if (str2int(argv[k], &status) || status < 0 || status > 1)
      { xprintf("Invalid improvement status '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->len_improve1 = status;
    }
    else if (p("--ga-improve2"))
    { int status;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No improvent status specified\n");
        return 1;
      }
      if (str2int(argv[k], &status) || status < 0 || status > 1)
      { xprintf("Invalid improvement status '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->len_improve2 = status;
    }
    else if (p("--ga-d2d"))
    { int d2d;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No number of iterations for d2d specified\n");
        return 1;
      }
      if (str2int(argv[k], &d2d) || d2d < 0)
      { xprintf("Invalid number of iterations '%s' for d2d\n", argv[k]);
        return 1;
      }
      csa->opcp->eacp->d2d = d2d;
    }
    /*------------------------------------------------------------------------*/
    /* 2 Parameter Iteractive Algorithm parameters */
#if 0
    else if (p("--op-2pia"))
      csa->opcp->heur_tech = OP_HEUR_2PIA;
    else if (p("--2pia-numsel"))
    { int num_sel;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No parameter specified for 2PIA\n");
        return 1;
      }
      if (str2int(argv[k], &num_sel) || num_sel < 1)
      { xprintf("Invalid parameter for IA '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->iacp->num_sel = num_sel;
    }
    else if (p("--2pia-itlim"))
    { int it_lim;
      k++;
      if (k == argc || argv[k][0] == '\0' || argv[k][0] == '-')
      { xprintf("No iteration limit specified\n");
        return 1;
      }
      if (str2int(argv[k], &it_lim) || it_lim < 0)
      { xprintf("Invalid iteration limit '%s'\n", argv[k]);
        return 1;
      }
      csa->opcp->iacp->it_lim = it_lim;
    }
#endif
    /*------------------------------------------------------------------------*/
    else if (argv[k][0] == '-' || (argv[k][0] == '-' && argv[k][1] == '-'))
    { xprintf("Invalid option '%s'; try %s --help\n", argv[k], argv[0]);
      return 1;
    }
    else
    { if (csa->in_file != NULL)
      { xprintf("Only one input problem file allowed\n");
        return 1;
        }
      csa->in_file = argv[k];
    }
  }
#undef p
      return 0;
}
