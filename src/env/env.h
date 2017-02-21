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

#ifndef ENV_H
#define ENV_H

#include "stdc.h"

typedef struct ENV ENV;
typedef struct MBD MBD;

#define SIZE_T_MAX (~(size_t)0)
/* largest value of size_t type */

#define TBUF_SIZE 4096
/* terminal output buffer size, in bytes */

#define EBUF_SIZE 1024
/* error message buffer size, in bytes */

/* enable/disable flag: */
#define compass_ON  1
#define compass_OFF 0

struct ENV
{     /* cmps environment block */
      char version[7+1];
      /* version string returned by the routine compass_version */
      ENV *self;
      /* pointer to this block to check its validity */
      /*--------------------------------------------------------------*/
      /* terminal output */
      char *term_buf; /* char term_buf[TBUF_SIZE]; */
      /* terminal output buffer */
      int term_out;
      /* flag to enable/disable terminal output */
      int (*term_hook)(void *info, const char *s);
      /* user-defined routine to intercept terminal output */
      void *term_info;
      /* transit pointer (cookie) passed to the routine term_hook */
      FILE *tee_file;
      /* output stream used to copy terminal output */
      /*--------------------------------------------------------------*/
      /* error handling */
#if 1 /* 07/XI-2015 */
      int err_st;
      /* error state flag; set on entry to compass_error */
#endif
      const char *err_file;
      /* value of the __FILE__ macro passed to compass_error */
      int err_line;
      /* value of the __LINE__ macro passed to compass_error */
      void (*err_hook)(void *info);
      /* user-defined routine to intercept abnormal termination */
      void *err_info;
      /* transit pointer (cookie) passed to the routine err_hook */
      char *err_buf; /* char err_buf[EBUF_SIZE]; */
      /* buffer to store error messages (used by I/O routines) */
      /*--------------------------------------------------------------*/
      /* dynamic memory allocation */
      size_t mem_limit;
      /* maximal amount of memory, in bytes, available for dynamic
       * allocation */
      MBD *mem_ptr;
      /* pointer to the linked list of allocated memory blocks */
      int mem_count;
      /* total number of currently allocated memory blocks */
      int mem_cpeak;
      /* peak value of mem_count */
      size_t mem_total;
      /* total amount of currently allocated memory, in bytes; it is
       * the sum of the size field over all memory block descriptors */
      size_t mem_tpeak;
      /* peak value of mem_total */
      /*--------------------------------------------------------------*/
      /* dynamic linking support (optional) */
      void *h_odbc;
      /* handle to ODBC shared library */
      void *h_mysql;
      /* handle to MySQL shared library */
};

struct MBD
{     /* memory block descriptor */
      size_t size;
      /* size of block, in bytes, including descriptor */
      MBD *self;
      /* pointer to this descriptor to check its validity */
      MBD *prev;
      /* pointer to previous memory block descriptor */
      MBD *next;
      /* pointer to next memory block descriptor */
};

#define get_env_ptr _compass_get_env_ptr
ENV *get_env_ptr(void);
/* retrieve pointer to environment block */

#define tls_set_ptr _compass_tls_set_ptr
void tls_set_ptr(void *ptr);
/* store global pointer in TLS */

#define tls_get_ptr _compass_tls_get_ptr
void *tls_get_ptr(void);
/* retrieve global pointer from TLS */

#define xputs compass_puts
void compass_puts(const char *s);
/* write string on terminal */

#define xprintf compass_printf
void compass_printf(const char *fmt, ...);
/* write formatted output on terminal */

#define xvprintf compass_vprintf
void compass_vprintf(const char *fmt, va_list arg);
/* write formatted output on terminal */

int compass_term_out(int flag);
/* enable/disable terminal output */

void compass_term_hook(int (*func)(void *info, const char *s), void *info);
/* install hook to intercept terminal output */

int compass_open_tee(const char *fname);
/* start copying terminal output to text file */

int compass_close_tee(void);
/* stop copying terminal output to text file */

#ifndef compass_ERRFUNC_DEFINED
#define compass_ERRFUNC_DEFINED
typedef void (*compass_errfunc)(const char *fmt, ...);
#endif

#define xerror compass_error_(__FILE__, __LINE__)
compass_errfunc compass_error_(const char *file, int line);
/* display fatal error message and terminate execution */

#define xassert(expr) \
      ((void)((expr) || (compass_assert_(#expr, __FILE__, __LINE__), 1)))
void compass_assert_(const char *expr, const char *file, int line);
/* check for logical condition */

void compass_error_hook(void (*func)(void *info), void *info);
/* install hook to intercept abnormal termination */

#define put_err_msg _compass_put_err_msg
void compass_err_msg(const char *msg);
/* provide error message string */

#define get_err_msg _compass_get_err_msg
const char *get_err_msg(void);
/* obtain error message string */

#define xmalloc(size) compass_alloc(1, size)
/* allocate memory block (obsolete) */

#define xcalloc(n, size) compass_alloc(n, size)
/* allocate memory block (obsolete) */

#define xalloc(n, size) compass_alloc(n, size)
#define talloc(n, type) ((type *)compass_alloc(n, sizeof(type)))
void *compass_alloc(int n, int size);
/* allocate memory block */

#define xrealloc(ptr, n, size) compass_realloc(ptr, n, size)
#define trealloc(ptr, n, type) ((type *)compass_realloc(ptr, n, \
      sizeof(type)))
void *compass_realloc(void *ptr, int n, int size);
/* reallocate memory block */

#define xfree(ptr) compass_free(ptr)
#define tfree(ptr) compass_free(ptr)
void compass_free(void *ptr);
/* free memory block */

void compass_mem_limit(int limit);
/* set memory usage limit */

void compass_mem_usage(int *count, int *cpeak, size_t *total,
      size_t *tpeak);
/* get memory usage information */

typedef struct compass_file compass_file;
/* sequential stream descriptor */

#define compass_open _compass_open
compass_file *compass_open(const char *name, const char *mode);
/* open stream */

#define compass_eof _compass_eof
int compass_eof(compass_file *f);
/* test end-of-file indicator */

#define compass_ioerr _compass_ioerr
int compass_ioerr(compass_file *f);
/* test I/O error indicator */

#define compass_read _compass_read
int compass_read(compass_file *f, void *buf, int nnn);
/* read data from stream */

#define compass_getc _compass_getc
int compass_getc(compass_file *f);
/* read character from stream */

#define compass_write _compass_write
int compass_write(compass_file *f, const void *buf, int nnn);
/* write data to stream */

#define compass_format _compass_format
int compass_format(compass_file *f, const char *fmt, ...);
/* write formatted data to stream */

#define compass_close _compass_close
int compass_close(compass_file *f);
/* close stream */

#define xtime compass_time
double compass_time(void);
/* determine current universal time */

#define xdifftime compass_difftime
double compass_difftime(double t1, double t0);
/* compute difference between two time values */

#define xdlopen _compass_dlopen
void *xdlopen(const char *module);
/* open dynamically linked library */

#define xdlsym _compass_dlsym
void *xdlsym(void *h, const char *symbol);
/* obtain address of symbol from dynamically linked library */

#define xdlclose _compass_dlclose
void xdlclose(void *h);
/* close dynamically linked library */

#endif

/* eof */
