/* dmp.h (dynamic memory pool) */

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

#ifndef DMP_H
#define DMP_H

#include "stdc.h"

typedef struct DMP DMP;

struct DMP
{     /* dynamic memory pool */
      void *avail[32];
      /* avail[k], 0 <= k <= 31, is a pointer to first available (free)
       * atom of (k+1)*8 bytes long; at the beginning of each free atom
       * there is a pointer to another free atom of the same size */
      void *block;
      /* pointer to most recently allocated memory block; at the
       * beginning of each allocated memory block there is a pointer to
       * previously allocated memory block */
      int used;
      /* number of bytes used in most recently allocated memory block */
      size_t count;
      /* number of atoms which are currently in use */
};

#define DMP_BLK_SIZE 8000
/* size of memory blocks, in bytes, allocated for memory pools */

struct prefix
{     /* atom prefix (for debugging only) */
      DMP *pool;
      /* dynamic memory pool */
      int size;
      /* original atom size, in bytes */
};

#define prefix_size ((sizeof(struct prefix) + 7) & ~7)
/* size of atom prefix rounded up to multiple of 8 bytes */

int dmp_debug;
/* debug mode flag */

//#define dmp_debug _compass_dmp_debug
extern int dmp_debug;
/* debug mode flag */

//#define dmp_create_pool _compass_dmp_create_pool
void dmp_create_pool(DMP *pool);
/* create dynamic memory pool */

#define dmp_talloc(pool, type) \
      ((type *)dmp_get_atom(pool, sizeof(type)))

//#define dmp_get_atom _compass_dmp_get_atom
void *dmp_get_atom(DMP *pool, int size);
/* get free atom from dynamic memory pool */

#define dmp_tfree(pool, atom) \
      dmp_free_atom(pool, atom, sizeof(*(atom)))

//#define dmp_free_atom _compass_dmp_free_atom
void dmp_free_atom(DMP *pool, void *atom, int size);
/* return atom to dynamic memory pool */

//#define dmp_in_use _compass_dmp_in_use
size_t dmp_in_use(DMP *pool);
/* determine how many atoms are still in use */

//#define dmp_delete_pool _compass_dmp_delete_pool
void dmp_delete_pool(DMP *pool);
/* delete dynamic memory pool */

#endif

/* eof */
