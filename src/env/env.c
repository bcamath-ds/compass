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

/***********************************************************************
*  NAME
*
*  compass_init_env - initialize CMPSK environment
*
*  SYNOPSIS
*
*  int compass_init_env(void);
*
*  DESCRIPTION
*
*  The routine compass_init_env initializes the CMPSK environment. Normally
*  the application program does not need to call this routine, because
*  it is called automatically on the first call to any API routine.
*
*  RETURNS
*
*  The routine compass_init_env returns one of the following codes:
*
*  0 - initialization successful;
*  1 - environment has been already initialized;
*  2 - initialization failed (insufficient memory);
*  3 - initialization failed (unsupported programming model). */

int compass_init_env(void)
{     ENV *env;
      int ok;
      /* check if the programming model is supported */
      ok = (CHAR_BIT == 8 && sizeof(char) == 1 &&
         sizeof(short) == 2 && sizeof(int) == 4 &&
         (sizeof(void *) == 4 || sizeof(void *) == 8));
      if (!ok)
         return 3;
      /* check if the environment is already initialized */
      if (tls_get_ptr() != NULL)
         return 1;
      /* allocate and initialize the environment block */
      env = malloc(sizeof(ENV));
      if (env == NULL)
         return 2;
      memset(env, 0, sizeof(ENV));
      sprintf(env->version, "%d.%d",
         COMPASS_MAJOR_VERSION, COMPASS_MINOR_VERSION);
      env->self = env;
      env->term_buf = malloc(TBUF_SIZE);
      if (env->term_buf == NULL)
      {  free(env);
         return 2;
      }
      env->term_out = compass_ON;
      env->term_hook = NULL;
      env->term_info = NULL;
      env->tee_file = NULL;
      env->err_file = NULL;
      env->err_line = 0;
      env->err_hook = NULL;
      env->err_info = NULL;
      env->err_buf = malloc(EBUF_SIZE);
      if (env->err_buf == NULL)
      {  free(env->term_buf);
         free(env);
         return 2;
      }
      env->err_buf[0] = '\0';
      env->mem_limit = SIZE_T_MAX;
      env->mem_ptr = NULL;
      env->mem_count = env->mem_cpeak = 0;
      env->mem_total = env->mem_tpeak = 0;
      env->h_odbc = env->h_mysql = NULL;
      /* save pointer to the environment block */
      tls_set_ptr(env);
      /* initialization successful */
      return 0;
}

/***********************************************************************
*  NAME
*
*  get_env_ptr - retrieve pointer to environment block
*
*  SYNOPSIS
*
*  #include "env.h"
*  ENV *get_env_ptr(void);
*
*  DESCRIPTION
*
*  The routine get_env_ptr retrieves and returns a pointer to the CMPSK
*  environment block.
*
*  If the CMPSK environment has not been initialized yet, the routine
*  performs initialization. If initialization fails, the routine prints
*  an error message to stderr and terminates the program.
*
*  RETURNS
*
*  The routine returns a pointer to the environment block. */

ENV *get_env_ptr(void)
{     ENV *env = tls_get_ptr();
      /* check if the environment has been initialized */
      if (env == NULL)
      {  /* not initialized yet; perform initialization */
         if (compass_init_env() != 0)
         {  /* initialization failed; display an error message */
            fprintf(stderr, "Compass initialization failed\n");
            fflush(stderr);
            /* and abnormally terminate the program */
            abort();
         }
         /* initialization successful; retrieve the pointer */
         env = tls_get_ptr();
      }
      /* check if the environment block is valid */
      if (env->self != env)
      {  fprintf(stderr, "Invalid Compass environment\n");
         fflush(stderr);
         abort();
      }
      return env;
}

/***********************************************************************
*  NAME
*
*  compass_version - determine library version
*
*  SYNOPSIS
*
*  const char *compass_version(void);
*
*  RETURNS
*
*  The routine compass_version returns a pointer to a null-terminated
*  character string, which specifies the version of the CMPSK library in
*  the form "X.Y", where X is the major version number, and Y is the
*  minor version number, for example, "4.16". */

const char *compass_version(void)
{     ENV *env = get_env_ptr();
      return env->version;
}

/***********************************************************************
*  NAME
*
*  compass_free_env - free Compass environment
*
*  SYNOPSIS
*
*  int compass_free_env(void);
*
*  DESCRIPTION
*
*  The routine compass_free_env frees all resources used by Compass routines
*  (memory blocks, etc.) which are currently still in use.
*
*  Normally the application program does not need to call this routine,
*  because CMPSK routines always free all unused resources. However, if
*  the application program even has deleted all problem objects, there
*  will be several memory blocks still allocated for the library needs.
*  For some reasons the application program may want CMPSK to free this
*  memory, in which case it should call compass_free_env.
*
*  Note that a call to compass_free_env invalidates all problem objects as
*  if no Compass routine were called.
*
*  RETURNS
*
*  0 - termination successful;
*  1 - environment is inactive (was not initialized). */

int compass_free_env(void)
{     ENV *env = tls_get_ptr();
      MBD *desc;
      /* check if the environment is active */
      if (env == NULL)
         return 1;
      /* check if the environment block is valid */
      if (env->self != env)
      {  fprintf(stderr, "Invalid Compass environment\n");
         fflush(stderr);
         abort();
      }
      /* close handles to shared libraries */
      if (env->h_odbc != NULL)
         xdlclose(env->h_odbc);
      if (env->h_mysql != NULL)
         xdlclose(env->h_mysql);
      /* free memory blocks which are still allocated */
      while (env->mem_ptr != NULL)
      {  desc = env->mem_ptr;
         env->mem_ptr = desc->next;
         free(desc);
      }
      /* close text file used for copying terminal output */
      if (env->tee_file != NULL)
         fclose(env->tee_file);
      /* invalidate the environment block */
      env->self = NULL;
      /* free memory allocated to the environment block */
      free(env->term_buf);
      free(env->err_buf);
      free(env);
      /* reset a pointer to the environment block */
      tls_set_ptr(NULL);
      /* termination successful */
      return 0;
}

/* eof */
