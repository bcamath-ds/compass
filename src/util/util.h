/****************************************************************************/
/*                                                                          */
/*  This file is part of CONCORDE                                           */
/*                                                                          */
/*  (c) Copyright 1995--1999 by David Applegate, Robert Bixby,              */
/*  Vasek Chvatal, and William Cook                                         */
/*                                                                          */
/*  Permission is granted for academic research use.  For other uses,       */
/*  contact the authors for licensing options.                              */
/*                                                                          */
/*  Use at your own risk.  We make no guarantees about the                  */
/*  correctness or usefulness of this code.                                 */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                      PROTOTYPES FOR FILES IN UTIL                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*    EXPORTED FUNCTIONS:                                                   */
/*                                                                          */
/*  CC_SAFE_MALLOC(nnum,type)                                               */
/*    int nnum (the number of objects to be malloced)                       */
/*    data type (the sort of objects to be malloced)                        */
/*    RETURNS a pointer to the allocated space. If out of memory,           */
/*            it prints an error message and returns NULL.                  */
/*                                                                          */
/*  CC_FREE(object,type)                                                    */
/*    type *object (pointer to previously allocated space)                  */
/*    data type (the sort of object)                                        */
/*    ACTION: frees the memory and sets the object to NULL.                 */
/*                                                                          */
/*  CC_IFFREE(object,type)                                                  */
/*    type *object (pointer to previously allocated space)                  */
/*    data type (the sort of object)                                        */
/*    ACTION: if *object is not NULL, frees the memory and sets             */
/*            the object to NULL.                                           */
/*                                                                          */
/*  CC_PTR_ALLOC_ROUTINE (type, functionname, chunklist, freelist)          */
/*    data type (the sort of objects)                                       */
/*    string functionname (the generated function)                          */
/*    CCbigchunkptr *chunklist (used to accumulate bigchunks)               */
/*    type *freelist (used for the linked list of objects)                  */
/*    ACTION: Generates a function ("functionname") that returns            */
/*            (type *) objects, keeping the free ones on freelist           */
/*            and getting its space from calls to                           */
/*            CCutil_bigchunkalloc.                                         */
/*                                                                          */
/*  CC_PTR_FREE_ROUTINE (type, functionname, freelist)                      */
/*    Parameters as above.                                                  */
/*    ACTION: Generates a function that adds an object to the               */
/*            freelist.                                                     */
/*                                                                          */
/*  CC_PTR_FREE_LIST_ROUTINE (type, functionname, freefunction)             */
/*    Parameters defined as above, with freefunction the function           */
/*    generated by CC_PTR_FREE_ROUTINE.                                     */
/*    ACTION: Generates a function to free a linked list of                 */
/*            objects using calls to freefunction.                          */
/*                                                                          */
/*  CC_PTR_FREE_WORLD_ROUTINE (type, functionname, chunklist, freelist)     */
/*    Parameters defined as above.                                          */
/*    ACTION: Generates a function that returns all of the                  */
/*            memory used in the CC_PTR_ALLOC_ROUTINE allocations           */
/*            back to the global supply of CCbigchunkptrs.                  */
/*                                                                          */
/*  CC_PTR_LEAKS_ROUTINE (type, name, chunklist, freelist, field,           */
/*      fieldtype)                                                          */
/*    As above, with "field" the name of a "fieldtype" field in the         */
/*    object type that can be set to 0 or to 1.                             */
/*    ACTION: Generates a function that checks to see that we have          */
/*            not leaked any of the objects.                                */
/*                                                                          */
/*  CC_PTR_STATUS_ROUTINE (type, name, chunklist, freelist)                 */
/*       ACTION: Like LEAKS, but does not check for duplicates (and so      */
/*               does not corrupt the objects).                             */
/*                                                                          */
/*    NOTES:                                                                */
/*       These routines use the functions in allocrus.c.  The PTR macros    */
/*    generate the functions for allocating objects for linked lists. They  */
/*    get their raw memory from the bigchunk supply, so foo_free_world      */
/*    (generated by CC_PTR_FREE_WORLD_ROUTINE) should be called for each    */
/*    type of linked object "foo" when closing down the local memory.       */
/*       To use these functions, put the macros near the top of the file    */
/*    before any calls to the functions (since the macros also write the    */
/*    function prototypes). If you use CC_PTR_FREE_LIST_ROUTINE for foo,    */
/*    you must also use CC_PTR_FREE_ROUTINE, and                            */
/*    CC_PTR_FREE_LIST_ROUTINE must be listed after CC_PTR_FREE_ROUTINE     */
/*    (to get the prototype).                                               */
/*                                                                          */
/****************************************************************************/

#ifndef __UTIL_H
#define __UTIL_H

#include "machdefs.h"
//#include "compass.h"

#define CCutil_MAXDOUBLE (1e30)
#define CCutil_MAXINT    (2147483647)

#define CCcheck_rval(rval,msg) {                                          \
    if ((rval)) {                                                          \
        fprintf (stderr, "%s\n", (msg));                                   \
        goto CLEANUP;                                                      \
    }                                                                      \
}

#define CCcheck_NULL(item,msg) {                                           \
    if ((!item)) {                                                         \
        fprintf (stderr, "%s\n", (msg));                                   \
        rval = 1;                                                          \
        goto CLEANUP;                                                      \
    }                                                                      \
}


#define CC_SBUFFER_SIZE (4000)
#define CC_SFNAME_SIZE (32)

typedef struct CC_SFILE {
    int           status;
    int           desc;
    int           type;
    int           chars_in_buffer;
    int           current_buffer_char;     /* only used for reading */
    int           bits_in_last_char;       /* writing: number of empty bits in
                                            * buffer[chars_in_buffer];
                                            * reading: number of full bits in
                                            * buffer[?] */
    int           pos;
    char          fname[CC_SFNAME_SIZE];
    char          hname[CC_SFNAME_SIZE];
    unsigned char buffer[CC_SBUFFER_SIZE];
} CC_SFILE;

#ifdef CC_NETREADY
typedef struct CC_SPORT {
    unsigned short port;
    int t;
} CC_SPORT;
#endif /* CC_NETREADY */

typedef struct CCrandstate {
    int a;
    int b;
    int arr[55];
} CCrandstate;

/****************************************************************************/
/*                                                                          */
/*                             allocrus.c                                   */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                   MEMORY ALLOCATION MACROS                               */
/*                                                                          */
/*                           TSP CODE                                       */
/*                                                                          */
/*                                                                          */
/*  Written by:  Applegate, Bixby, Chvatal, and Cook                        */
/*  Date: February 24, 1995 (cofeb24)                                       */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#define CC_SAFE_MALLOC(nnum,type)                                          \
    (type *) CCutil_allocrus (((size_t) (nnum)) * sizeof (type))

#define CC_FREE(object,type) {                                             \
    CCutil_freerus ((void *) (object));                                    \
    object = (type *) NULL;                                                \
}

#define CC_IFFREE(object,type) {                                           \
    if ((object)) CC_FREE ((object),type);                                 \
}

#define CC_PTRWORLD_ALLOC_ROUTINE(type, ptr_alloc_r, ptr_bulkalloc_r)        \
                                                                             \
static int ptr_bulkalloc_r (CCptrworld *world, int nalloc)                   \
{                                                                            \
    CCbigchunkptr *bp;                                                       \
    int i;                                                                   \
    int count = CC_BIGCHUNK / sizeof ( type );                               \
    type *p;                                                                 \
                                                                             \
    while (nalloc > 0) {                                                     \
        bp = CCutil_bigchunkalloc ();                                        \
        if (bp == (CCbigchunkptr *) NULL) {                                  \
            fprintf (stderr, "ptr alloc failed\n");                          \
            return 1;                                                        \
        }                                                                    \
        bp->next = world->chunklist ;                                        \
        world->chunklist = bp;                                               \
                                                                             \
        p = ( type * ) bp->this_one;                                         \
        for (i=count-2; i>=0; i--) {                                         \
            p[i].next = &p[i+1];                                             \
        }                                                                    \
        p[count - 1].next = (type *) world->freelist;                        \
        world->freelist = (void *) p;                                        \
        nalloc -= count;                                                     \
    }                                                                        \
    return 0;                                                                \
}                                                                            \
                                                                             \
static type *ptr_alloc_r (CCptrworld *world)                                 \
{                                                                            \
    type *p;                                                                 \
                                                                             \
    if (world->freelist == (void *) NULL) {                                  \
        if (ptr_bulkalloc_r (world, 1)) {                                    \
            fprintf (stderr, "ptr alloc failed\n");                          \
            return ( type * ) NULL;                                          \
        }                                                                    \
    }                                                                        \
    p = (type *) world->freelist ;                                           \
    world->freelist = (void *) p->next;                                      \
                                                                             \
    return p;                                                                \
}

#define CC_PTRWORLD_FREE_ROUTINE(type, ptr_free_r)                           \
                                                                             \
static void ptr_free_r (CCptrworld *world, type *p)                          \
{                                                                            \
    p->next = (type *) world->freelist ;                                     \
    world->freelist = (void *) p;                                            \
}

#define CC_PTRWORLD_LISTADD_ROUTINE(type, entrytype, ptr_listadd_r, ptr_alloc_r) \
                                                                             \
static int ptr_listadd_r (type **list, entrytype x, CCptrworld *world)       \
{                                                                            \
    if (list != (type **) NULL) {                                            \
        type *p = ptr_alloc_r (world);                                       \
                                                                             \
        if (p == (type *) NULL) {                                            \
            fprintf (stderr, "ptr list add failed\n");                       \
            return 1;                                                        \
        }                                                                    \
        p->this = x;                                                         \
        p->next = *list;                                                     \
        *list = p;                                                           \
    }                                                                        \
    return 0;                                                                \
}

#define CC_PTRWORLD_LISTFREE_ROUTINE(type, ptr_listfree_r, ptr_free_r)       \
                                                                             \
static void ptr_listfree_r (CCptrworld *world, type *p)                      \
{                                                                            \
    type *next;                                                              \
                                                                             \
    while (p != (type *) NULL) {                                             \
        next = p->next;                                                      \
        ptr_free_r (world, p);                                               \
        p = next;                                                            \
    }                                                                        \
}

#define CC_PTRWORLD_LEAKS_ROUTINE(type, ptr_leaks_r, field, fieldtype)       \
                                                                             \
static int ptr_leaks_r (CCptrworld *world, int *total, int *onlist)          \
{                                                                            \
    int count = CC_BIGCHUNK / sizeof ( type );                               \
    int duplicates = 0;                                                      \
    type * p;                                                                \
    CCbigchunkptr *bp;                                                       \
                                                                             \
    *total = 0;                                                              \
    *onlist = 0;                                                             \
                                                                             \
    for (bp = world->chunklist ; bp; bp = bp->next)                          \
        (*total) += count;                                                   \
                                                                             \
    for (p = (type *) world->freelist ; p; p = p->next) {                    \
        (*onlist)++;                                                         \
        p-> field = ( fieldtype ) 0;                                         \
    }                                                                        \
    for (p = (type *) world->freelist ; p; p = p->next) {                    \
        if ((unsigned long) p-> field == (unsigned long) (size_t) 1)                           \
            duplicates++;                                                    \
        else                                                                 \
            p-> field = ( fieldtype ) (size_t) 1;                            \
    }                                                                        \
    if (duplicates) {                                                        \
        fprintf (stderr, "WARNING: %d duplicates on ptr free list \n",       \
                 duplicates);                                                \
    }                                                                        \
    return *total - *onlist;                                                 \
}

#define CC_PTRWORLD_ROUTINES(type, ptr_alloc_r, ptr_bulkalloc_r, ptr_free_r) \
CC_PTRWORLD_ALLOC_ROUTINE (type, ptr_alloc_r, ptr_bulkalloc_r)               \
CC_PTRWORLD_FREE_ROUTINE (type, ptr_free_r)

#define CC_PTRWORLD_LIST_ROUTINES(type, entrytype, ptr_alloc_r, ptr_bulkalloc_r, ptr_free_r, ptr_listadd_r, ptr_listfree_r) \
CC_PTRWORLD_ROUTINES (type, ptr_alloc_r, ptr_bulkalloc_r, ptr_free_r)        \
CC_PTRWORLD_LISTADD_ROUTINE (type, entrytype, ptr_listadd_r, ptr_alloc_r)    \
CC_PTRWORLD_LISTFREE_ROUTINE (type, ptr_listfree_r, ptr_free_r)

#define CC_BIGCHUNK ((int) ((1<<16) - sizeof (CCbigchunkptr) - 16))

struct CCbigchunk;

typedef struct CCbigchunkptr {
    void                 *this_one;
    struct CCbigchunk    *this_chunk;
    struct CCbigchunkptr *next;
} CCbigchunkptr;


typedef struct CCptrworld {
    int refcount;
    void *freelist;
    CCbigchunkptr *chunklist;
} CCptrworld;



void
   *CCutil_allocrus (size_t size),
   *CCutil_reallocrus (void *ptr, size_t size),
    CCutil_freerus (void *p),
    CCutil_bigchunkfree (CCbigchunkptr *bp),
    CCptrworld_init (CCptrworld *world),
    CCptrworld_add (CCptrworld *world),
    CCptrworld_delete (CCptrworld *world);

int
    CCutil_reallocrus_scale (void **pptr, int *pnnum, int count, double scale,
        size_t size),
    CCutil_reallocrus_count (void **pptr, int count, size_t size);

CCbigchunkptr
    *CCutil_bigchunkalloc (void);




/****************************************************************************/
/*                                                                          */
/*                             bgetopt.c                                    */
/*                                                                          */
/****************************************************************************/


int
    CCutil_bix_getopt (int argc, char **argv, const char *def, int *p_optind,
        char **p_optarg);


#define CC_BIX_GETOPT_UNKNOWN -3038



/****************************************************************************/
/*                                                                          */
/*                             dheaps_i.c                                   */
/*                                                                          */
/****************************************************************************/

typedef struct CCdheap {
    double  *key;
    int     *entry;
    int     *loc;
    int     total_space;
    int     size;
} CCdheap;


void
    CCutil_dheap_free (CCdheap *h),
    CCutil_dheap_delete (CCdheap *h, int i),
    CCutil_dheap_changekey (CCdheap *h, int i, double newkey);

int
    CCutil_dheap_init (CCdheap *h, int k),
    CCutil_dheap_resize (CCdheap *h, int newsize),
    CCutil_dheap_findmin (CCdheap *h),
    CCutil_dheap_deletemin (CCdheap *h),
    CCutil_dheap_insert (CCdheap *h, int i);



/****************************************************************************/
/*                                                                          */
/*                             edgeutil.c                                   */
/*                                                                          */
/****************************************************************************/

typedef struct CCelist {
    int  ecount;
    int *ends;
} CCelist;

typedef struct CCelistl {
    int  ecount;
    int *ends;
    int *len;
} CCelistl;

typedef struct CCelistw {
    int     ecount;
    int    *ends;
    double *weight;
} CCelistw;

typedef struct CCelistlw {
    int     ecount;
    int    *ends;
    int    *len;
    double *weight;
} CCelistlw;

void
    CCelist_init (CCelist *elist),
    CCelistl_init (CCelistl *elist),
    CCelistw_init (CCelistw *elist),
    CCelistlw_init (CCelistlw *elist),
    CCelist_free (CCelist *elist),
    CCelistl_free (CCelistl *elist),
    CCelistw_free (CCelistw *elist),
    CCelistlw_free (CCelistlw *elist);

int
    CCelist_alloc (CCelist *elist, int ecount),
    CCelistl_alloc (CCelistl *elist, int ecount),
    CCelistw_alloc (CCelistw *elist, int ecount),
    CCelistlw_alloc (CCelistlw *elist, int ecount),
    CCutil_edge_to_cycle (int ncount, int *elist, int *yesno, int *cyc);







/****************************************************************************/
/*                                                                          */
/*                             edgemap.c                                    */
/*                                                                          */
/****************************************************************************/

typedef struct CCutil_edgeinf {
    int                   ends[2];
    int                   val;
    struct CCutil_edgeinf *next;
} CCutil_edgeinf;

typedef struct CCutil_edgehash {
    CCutil_edgeinf **table;
    CCptrworld      edgeinf_world;
    unsigned int    size;
    unsigned int    mult;
} CCutil_edgehash;


int
    CCutil_edgehash_init (CCutil_edgehash *h, int size),
    CCutil_edgehash_add (CCutil_edgehash *h, int end1, int end2, int val),
    CCutil_edgehash_set (CCutil_edgehash *h, int end1, int end2, int val),
    CCutil_edgehash_del (CCutil_edgehash *h, int end1, int end2),
    CCutil_edgehash_find (CCutil_edgehash *h, int end1, int end2, int *val),
    CCutil_edgehash_getall (CCutil_edgehash *h, int *ecount, int **elist,
        int **elen);

void
    CCutil_edgehash_delall (CCutil_edgehash *h),
    CCutil_edgehash_free (CCutil_edgehash *h);


/****************************************************************************/
/*                                                                          */
/*                             eunion.c                                     */
/*                                                                          */
/****************************************************************************/

int
    CCutil_edge_file_union (int ncount, int nfiles, char **flist, int *ecount,
        int **elist, int **elen, int *foundtour, double *besttourlen);



/****************************************************************************/
/*                                                                          */
/*                             fastread.c                                   */
/*                                                                          */
/****************************************************************************/


int
    CCutil_readint (FILE *f);





/****************************************************************************/
/*                                                                          */
/*                             genhash.c                                    */
/*                                                                          */
/****************************************************************************/

struct CCgenhash_elem;

typedef struct CCgenhash {
    int                     nelem;
    int                     maxelem;
    int                     size;
    int                   (*hcmp) (void *key1, void *key2, void *u_data);
    unsigned int          (*hfunc) (void *key, void *u_data);
    void                   *u_data;
    double                  maxdensity;
    double                  lowdensity;
    CCptrworld              elem_world;
    struct CCgenhash_elem **table;
} CCgenhash;

typedef struct CCgenhash_iter {
    int                    i;
    struct CCgenhash_elem *next;
} CCgenhash_iter;


int
    CCutil_genhash_init (CCgenhash *h, int size, int (*hcmp) (void *key1,
        void *key2, void *u_data), unsigned int (*hfunc) (void *key,
        void *u_data), void *u_data, double maxdensity, double lowdensity),
    CCutil_genhash_insert (CCgenhash *h, void *key, void *data),
    CCutil_genhash_insert_h (CCgenhash *h, unsigned int hashval, void *key,
        void *data),
    CCutil_genhash_replace (CCgenhash *h, void *key, void *data),
    CCutil_genhash_replace_h (CCgenhash *h, unsigned int hashval, void *key,
        void *data),
    CCutil_genhash_delete (CCgenhash *h, void *key),
    CCutil_genhash_delete_h (CCgenhash *h, unsigned int hashval, void *key);

unsigned int
    CCutil_genhash_hash (CCgenhash *h, void *key);

void
   *CCutil_genhash_lookup (CCgenhash *h, void *key),
   *CCutil_genhash_lookup_h (CCgenhash *h, unsigned int hashval, void *key),
   *CCutil_genhash_next (CCgenhash *h, CCgenhash_iter *iter, void **key,
        int *keysize);

void
    CCutil_genhash_u_data (CCgenhash *h, void *u_data),
    CCutil_genhash_free (CCgenhash *h, void (*freefunc)(void *key, void *data,
        void *u_data)),
    CCutil_genhash_start (CCgenhash *h, CCgenhash_iter *iter);


/****************************************************************************/
/*                                                                          */
/*                             priority.c                                   */
/*                                                                          */
/****************************************************************************/

typedef struct CCpriority {
    CCdheap   heap;
    union CCpri_data {
        void *data;
        int  next;
    } *pri_info;
    int     space;
    int     freelist;
} CCpriority;


void
    CCutil_priority_free (CCpriority *pri),
    CCutil_priority_delete (CCpriority *pri, int handle),
    CCutil_priority_changekey (CCpriority *pri, int handle, double newkey),
   *CCutil_priority_findmin (CCpriority *pri, double *keyval),
   *CCutil_priority_deletemin (CCpriority *pri, double *keyval);

int
    CCutil_priority_init (CCpriority *pri, int k),
    CCutil_priority_insert (CCpriority *pri, void *data, double keyval);



/****************************************************************************/
/*                                                                          */
/*                             safe_io.c                                    */
/*                                                                          */
/****************************************************************************/


CC_SFILE
    *CCutil_sopen (const char *f, const char *s),
    *CCutil_sdopen (int d, const char *s);

int
    CCutil_swrite (CC_SFILE *f, char *buf, int size),
    CCutil_swrite_bits (CC_SFILE *f, int x, int xbits),
    CCutil_swrite_ubits (CC_SFILE *f, unsigned int x, int xbits),
    CCutil_swrite_char (CC_SFILE *f, char x),
    CCutil_swrite_string (CC_SFILE *f, const char *x),
    CCutil_swrite_short (CC_SFILE *f, short x),
    CCutil_swrite_ushort (CC_SFILE *f, unsigned short x),
    CCutil_swrite_int (CC_SFILE *f, int x),
    CCutil_swrite_uint (CC_SFILE *f, unsigned int x),
    CCutil_swrite_double (CC_SFILE *f, double x),
    CCutil_sread (CC_SFILE *f, char *buf, int size),
    CCutil_sread_bits (CC_SFILE *f, int *x, int xbits),
    CCutil_sread_ubits (CC_SFILE *f, unsigned int *x, int xbits),
    CCutil_sread_char (CC_SFILE *f, char *x),
    CCutil_sread_string (CC_SFILE *f, char *x, int maxlen),
    CCutil_sread_short (CC_SFILE *f, short *x),
    CCutil_sread_ushort (CC_SFILE *f, unsigned short *x),
    CCutil_sread_short_r (CC_SFILE *f, short *x),
    CCutil_sread_int (CC_SFILE *f, int *x),
    CCutil_sread_uint (CC_SFILE *f, unsigned int *x),
    CCutil_sread_int_r (CC_SFILE *f, int *x),
    CCutil_sread_double (CC_SFILE *f, double *x),
    CCutil_sread_double_r (CC_SFILE *f, double *x),
    CCutil_sflush (CC_SFILE *f),
    CCutil_stell (CC_SFILE *f),
    CCutil_sseek (CC_SFILE *f, int offset),
    CCutil_srewind (CC_SFILE *f),
    CCutil_sclose (CC_SFILE *f),
    CCutil_sbits (unsigned int x),
    CCutil_sdelete_file (const char *fname),
    CCutil_sdelete_file_backup (const char *fname);

#ifdef CC_NETREADY
CC_SFILE
   *CCutil_snet_open (const char *hname, unsigned short p),
   *CCutil_snet_receive (CC_SPORT *s);

CC_SPORT
   *CCutil_snet_listen (unsigned short p);

void
    CCutil_snet_unlisten (CC_SPORT *s);

#endif /* CC_NETREADY */





/****************************************************************************/
/*                                                                          */
/*                             signal.c                                     */
/*                                                                          */
/****************************************************************************/

#define CCutil_SIGHUP                1  /* HangUp */
#define CCutil_SIGINT                2  /* Interrupt */
#define CCutil_SIGQUIT               3  /* Quit */
#define CCutil_SIGILL                4  /* Illegal instruction */
#define CCutil_SIGTRAP               5  /* Trace trap */
#define CCutil_SIGABRT               6  /* Abort */
#define CCutil_SIGEMT                7  /* Emulator trap */
#define CCutil_SIGFPE                8  /* Floating point exception */
#define CCutil_SIGKILL               9  /* Kill process */
#define CCutil_SIGBUS               10  /* Bus error */
#define CCutil_SIGSEGV              11  /* Segmentation fault */
#define CCutil_SIGSYS               12  /* Illegal argument to system call */
#define CCutil_SIGPIPE              13  /* Pipe */
#define CCutil_SIGALRM              14  /* Alarm */
#define CCutil_SIGTERM              15  /* Terminate */
#define CCutil_SIGUSR1              16  /* User signal 1 */
#define CCutil_SIGUSR2              17  /* User signal 2 */
#define CCutil_SIGCHLD              18  /* Child condition change */
#define CCutil_SIGPWR               19  /* Power fail */
#define CCutil_SIGWINCH             20  /* Window size changes */
#define CCutil_SIGURG               21  /* Urgent condition on IO channel*/
#define CCutil_SIGIO                22  /* IO possible */
#define CCutil_SIGSTOP              23  /* Stop */
#define CCutil_SIGTSTP              24  /* Tty stop */
#define CCutil_SIGCONT              25  /* Continue */
#define CCutil_SIGTTIN              26  /* Tty background read */
#define CCutil_SIGTTOU              27  /* Tty background write */
#define CCutil_SIGVTALRM            28  /* Virtual timer alarm */
#define CCutil_SIGPROF              29  /* Profiling timer alarm */
#define CCutil_SIGXCPU              30  /* CPU limit exceeded */
#define CCutil_SIGXFSZ              31  /* File size limit exceeded */
#define CCutil_SIGSTKFLT            32  /* Stack fault */
#define CCutil_SIGIOT               33  /* IOT instruction */
#define CCutil_SIGPOLL              34  /* Pollable event */
#define CCutil_SIGMSG               35  /* Message available */
#define CCutil_SIGDANGER            36  /* System crash imminent */
#define CCutil_SIGMIGRATE           37  /* Migrate process */
#define CCutil_SIGPRE               38  /* Programming exception */
#define CCutil_SIGVIRT              39  /* Second virtual time alarm */
#define CCutil_MAXSIG               39


typedef void (*CCutil_handler)(int signum);

int
    CCutil_signal_handler (int ccsignum, CCutil_handler handler),
    CCutil_signal_default (int ccsignum),
    CCutil_signal_ignore (int ccsignum),
    CCutil_sig_to_ccsig (int signum);

void
    CCutil_signal_init (void),
    CCutil_handler_fatal (int signum),
    CCutil_handler_warn (int signum),
    CCutil_handler_exit (int signum);




/****************************************************************************/
/*                                                                          */
/*                             sortrus.c                                    */
/*                                                                          */
/****************************************************************************/


void
    CCutil_int_array_quicksort (int *len, int n),
    CCutil_int_perm_quicksort (int *perm, int *len, int n),
    CCutil_double_perm_quicksort (int *perm, double *len, int n),
    CCutil_rselect (int *arr, int l, int r, int m, double *coord,
        CCrandstate *rstate);

char
    *CCutil_linked_radixsort (char *data, char *datanext, char *dataval,
        int valsize);


/****************************************************************************/
/*                                                                          */
/*                             subdiv.c                                     */
/*                                                                          */
/****************************************************************************/

#define CC_SUBDIV_PORT  ((unsigned short) 32141)
#define CC_SUBGATE_PORT ((unsigned short) 32143)
#define CCutil_FILE_NAME_LEN  (128)

typedef struct CCsubdiv {
    double xrange[2];
    double yrange[2];
    int    cnt;
    int    id;
    double bound;
    int    status;
} CCsubdiv;

typedef struct CCsubdiv_lkh {
    int id;
    int cnt;
    int start;
    double origlen;
    double newlen;
    int    status;
} CCsubdiv_lkh;


int
   // CCutil_karp_partition (int ncount, compass_data *dat, int partsize,
   //     int *p_scount, CCsubdiv **p_slist, int ***partlist,
   //     CCrandstate *rstate),
    CCutil_write_subdivision_index (char *problabel, int ncount, int scount,
        CCsubdiv *slist),
    CCutil_read_subdivision_index (char *index_name, char **p_problabel,
        int *p_ncount, int *p_scount, CCsubdiv **p_slist),
    CCutil_write_subdivision_lkh_index (char *problabel, int ncount,
        int scount, CCsubdiv_lkh *slist, double tourlen),
    CCutil_read_subdivision_lkh_index (char *index_name, char **p_problabel,
        int *p_ncount, int *p_scount, CCsubdiv_lkh **p_slist,
        double *p_tourlen);


/****************************************************************************/
/*                                                                          */
/*                             urandom.c                                    */
/*                                                                          */
/****************************************************************************/

/* since urandom's generator does everything modulo CC_PRANDMAX, if two
 * seeds are congruent mod x and x|CC_PRANDMAX, then the resulting numbers
 * will be congruent mod x.  One example was if CC_PRANDMAX = 1000000000 and
 * urandom is used to generate a point set from a 1000x1000 grid, seeds
 * congruent mod 1000 generate the same point set.
 *
 * For this reason, we use 1000000007 (a prime)
 */
#define CC_PRANDMAX 1000000007

void
   CCutil_sprand (int seed, CCrandstate *r);

int
   CCutil_lprand (CCrandstate *r);

double
   CCutil_normrand (CCrandstate *r);





/****************************************************************************/
/*                                                                          */
/*                             util.c                                       */
/*                                                                          */
/****************************************************************************/


char
   *CCutil_strchr (char *s, int c),
   *CCutil_strrchr (char *s, int c),
   *CCutil_strdup (const char *s),
   *CCutil_strdup2 (const char *s);

const char
   *CCutil_strchr_c (const char *s, int c),
   *CCutil_strrchr_c (const char *s, int c);

unsigned int
    CCutil_nextprime (unsigned int x);

int
    CCutil_our_gcd (int a, int b),
    CCutil_our_lcm (int a, int b),
    CCutil_print_command (int ac, char **av);

void
    CCutil_readstr (FILE *f, char *s, int len),
    CCutil_printlabel (void);





/****************************************************************************/
/*                                                                          */
/*                             zeit.c                                       */
/*                                                                          */
/****************************************************************************/

typedef struct CCutil_timer {
    double  szeit;
    double  cum_zeit;
    char    name[40];
    int     count;
} CCutil_timer;


double
    CCutil_zeit (void),
    CCutil_real_zeit (void),
    CCutil_stop_timer (CCutil_timer *t, int printit),
    CCutil_total_timer (CCutil_timer *t, int printit);


void
    CCutil_init_timer (CCutil_timer *t, const char *name),
    CCutil_start_timer (CCutil_timer *t),
    CCutil_suspend_timer (CCutil_timer *t),
    CCutil_resume_timer (CCutil_timer *t);



#endif /* __UTIL_H */
