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

#include "env.h"

#define ALIGN 16
/* some processors need data to be properly aligned, so this macro
 * defines the alignment boundary, in bytes, provided by cmpsk memory
 * allocation routines; looks like 16-byte alignment boundary is
 * sufficient for all 32- and 64-bit platforms (8-byte boundary is not
 * sufficient for some 64-bit platforms because of jmp_buf) */

#define MBD_SIZE (((sizeof(MBD) + (ALIGN - 1)) / ALIGN) * ALIGN)
/* size of memory block descriptor, in bytes, rounded up to multiple
 * of the alignment boundary */

/***********************************************************************
*  dma - dynamic memory allocation (basic routine)
*
*  This routine performs dynamic memory allocation. It is similar to
*  the standard realloc function, however, it provides every allocated
*  memory block with a descriptor, which is used for sanity checks on
*  reallocating/freeing previously allocated memory blocks as well as
*  for book-keeping the memory usage statistics. */

static void *dma(const char *func, void *ptr, size_t size)
{     ENV *env = get_env_ptr();
      MBD *mbd;
      if (ptr == NULL)
      {  /* new memory block will be allocated */
         mbd = NULL;
      }
      else
      {  /* allocated memory block will be reallocated or freed */
         /* get pointer to the block descriptor */
         mbd = (MBD *)((char *)ptr - MBD_SIZE);
         /* make sure that the block descriptor is valid */
         if (mbd->self != mbd)
            xerror("%s: ptr = %p; invalid pointer\n", func, ptr);
         /* remove the block from the linked list */
         mbd->self = NULL;
         if (mbd->prev == NULL)
            env->mem_ptr = mbd->next;
         else
            mbd->prev->next = mbd->next;
         if (mbd->next == NULL)
            ;
         else
            mbd->next->prev = mbd->prev;
         /* decrease usage counts */
         if (!(env->mem_count >= 1 && env->mem_total >= mbd->size))
            xerror("%s: memory allocation error\n", func);
         env->mem_count--;
         env->mem_total -= mbd->size;
         if (size == 0)
         {  /* free the memory block */
            free(mbd);
            return NULL;
         }
      }
      /* allocate/reallocate memory block */
      if (size > SIZE_T_MAX - MBD_SIZE)
         xerror("%s: block too large\n", func);
      size += MBD_SIZE;
      if (size > env->mem_limit - env->mem_total)
         xerror("%s: memory allocation limit exceeded\n", func);
      if (env->mem_count == INT_MAX)
         xerror("%s: too many memory blocks allocated\n", func);
      mbd = (mbd == NULL ? malloc(size) : realloc(mbd, size));
      if (mbd == NULL)
         xerror("%s: no memory available\n", func);
      /* setup the block descriptor */
      mbd->size = size;
      mbd->self = mbd;
      mbd->prev = NULL;
      mbd->next = env->mem_ptr;
      /* add the block to the beginning of the linked list */
      if (mbd->next != NULL)
         mbd->next->prev = mbd;
      env->mem_ptr = mbd;
      /* increase usage counts */
      env->mem_count++;
      if (env->mem_cpeak < env->mem_count)
         env->mem_cpeak = env->mem_count;
      env->mem_total += size;
      if (env->mem_tpeak < env->mem_total)
         env->mem_tpeak = env->mem_total;
      return (char *)mbd + MBD_SIZE;
}

/***********************************************************************
*  NAME
*
*  compass_alloc - allocate memory block
*
*  SYNOPSIS
*
*  void *compass_alloc(int n, int size);
*
*  DESCRIPTION
*
*  The routine compass_alloc allocates a memory block of n * size bytes
*  long.
*
*  Note that being allocated the memory block contains arbitrary data
*  (not binary zeros!).
*
*  RETURNS
*
*  The routine compass_alloc returns a pointer to the block allocated.
*  To free this block the routine compass_free (not free!) must be used. */

void *compass_alloc(int n, int size)
{     if (n < 1)
         xerror("compass_alloc: n = %d; invalid parameter\n", n);
      if (size < 1)
         xerror("compass_alloc: size = %d; invalid parameter\n", size);
      if ((size_t)n > SIZE_T_MAX / (size_t)size)
         xerror("compass_alloc: n = %d, size = %d; block too large\n",
            n, size);
      return dma("compass_alloc", NULL, (size_t)n * (size_t)size);
}

/**********************************************************************/

void *compass_realloc(void *ptr, int n, int size)
{     /* reallocate memory block */
      if (ptr == NULL)
         xerror("compass_realloc: ptr = %p; invalid pointer\n", ptr);
      if (n < 1)
         xerror("compass_realloc: n = %d; invalid parameter\n", n);
      if (size < 1)
         xerror("compass_realloc: size = %d; invalid parameter\n", size);
      if ((size_t)n > SIZE_T_MAX / (size_t)size)
         xerror("compass_realloc: n = %d, size = %d; block too large\n",
            n, size);
      return dma("compass_realloc", ptr, (size_t)n * (size_t)size);
}

/***********************************************************************
*  NAME
*
*  compass_free - free (deallocate) memory block
*
*  SYNOPSIS
*
*  void compass_free(void *ptr);
*
*  DESCRIPTION
*
*  The routine compass_free frees (deallocates) a memory block pointed to
*  by ptr, which was previuosly allocated by the routine compass_alloc or
*  reallocated by the routine compass_realloc. */

void compass_free(void *ptr)
{     if (ptr == NULL)
         xerror("compass_free: ptr = %p; invalid pointer\n", ptr);
      dma("compass_free", ptr, 0);
      return;
}

/***********************************************************************
*  NAME
*
*  compass_mem_limit - set memory usage limit
*
*  SYNOPSIS
*
*  void compass_mem_limit(int limit);
*
*  DESCRIPTION
*
*  The routine compass_mem_limit limits the amount of memory available for
*  dynamic allocation (in CMPSK routines) to limit megabytes. */

void compass_mem_limit(int limit)
{     ENV *env = get_env_ptr();
      if (limit < 1)
         xerror("compass_mem_limit: limit = %d; invalid parameter\n",
            limit);
      if ((size_t)limit <= (SIZE_T_MAX >> 20))
         env->mem_limit = (size_t)limit << 20;
      else
         env->mem_limit = SIZE_T_MAX;
      return;
}

/***********************************************************************
*  NAME
*
*  compass_mem_usage - get memory usage information
*
*  SYNOPSIS
*
*  void compass_mem_usage(int *count, int *cpeak, size_t *total,
*     size_t *tpeak);
*
*  DESCRIPTION
*
*  The routine compass_mem_usage reports some information about utilization
*  of the memory by CMPSK routines. Information is stored to locations
*  specified by corresponding parameters (see below). Any parameter can
*  be specified as NULL, in which case its value is not stored.
*
*  *count is the number of the memory blocks currently allocated by the
*  routines compass_malloc and compass_calloc (one call to compass_malloc or
*  compass_calloc results in allocating one memory block).
*
*  *cpeak is the peak value of *count reached since the initialization
*  of the CMPSK library environment.
*
*  *total is the total amount, in bytes, of the memory blocks currently
*  allocated by the routines compass_malloc and compass_calloc.
*
*  *tpeak is the peak value of *total reached since the initialization
*  of the CMPSK library envirionment. */

void compass_mem_usage(int *count, int *cpeak, size_t *total,
      size_t *tpeak)
{     ENV *env = get_env_ptr();
      if (count != NULL)
         *count = env->mem_count;
      if (cpeak != NULL)
         *cpeak = env->mem_cpeak;
      if (total != NULL)
         *total = env->mem_total;
      if (tpeak != NULL)
         *tpeak = env->mem_tpeak;
      return;
}

/* eof */
