#ifndef Py_UFUNCOBJECT_H
#define Py_UFUNCOBJECT_H

#include <numpy/npy_math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PyUFuncGenericFunction) (char **, npy_intp *, npy_intp *, void *);

typedef struct {
        PyObject_HEAD
        int nin, nout, nargs;
        int identity;
        PyUFuncGenericFunction *functions;
        void **data;
        int ntypes;
        int check_return;
        char *name, *types;
        char *doc;
        void *ptr;
        PyObject *obj;
        PyObject *userloops;
    
        /* generalized ufunc */
        int core_enabled;      /* 0 for scalar ufunc; 1 for generalized ufunc */
        int core_num_dim_ix;   /* number of distinct dimension names in
                                  signature */
 
        /* dimension indices of input/output argument k are stored in
           core_dim_ixs[core_offsets[k]..core_offsets[k]+core_num_dims[k]-1] */
        int *core_num_dims;    /* numbers of core dimensions of each argument */
        int *core_dim_ixs;     /* dimension indices in a flatted form; indices
                                  are in the range of [0,core_num_dim_ix) */
        int *core_offsets;     /* positions of 1st core dimensions of each
                                  argument in core_dim_ixs */
        char *core_signature;  /* signature string for printing purpose */
} PyUFuncObject;

#include "arrayobject.h"

#define UFUNC_ERR_IGNORE 0
#define UFUNC_ERR_WARN   1
#define UFUNC_ERR_RAISE  2
#define UFUNC_ERR_CALL   3
#define UFUNC_ERR_PRINT  4
#define UFUNC_ERR_LOG    5

        /* Python side integer mask */

#define UFUNC_MASK_DIVIDEBYZERO 0x07
#define UFUNC_MASK_OVERFLOW 0x3f
#define UFUNC_MASK_UNDERFLOW 0x1ff
#define UFUNC_MASK_INVALID 0xfff

#define UFUNC_SHIFT_DIVIDEBYZERO 0
#define UFUNC_SHIFT_OVERFLOW     3
#define UFUNC_SHIFT_UNDERFLOW    6
#define UFUNC_SHIFT_INVALID      9


/* platform-dependent code translates floating point
   status to an integer sum of these values
*/
#define UFUNC_FPE_DIVIDEBYZERO  1
#define UFUNC_FPE_OVERFLOW      2
#define UFUNC_FPE_UNDERFLOW     4
#define UFUNC_FPE_INVALID       8

#define UFUNC_ERR_DEFAULT  0      /* Error mode that avoids look-up (no checking) */

#define UFUNC_OBJ_ISOBJECT      1
#define UFUNC_OBJ_NEEDS_API     2

   /* Default user error mode */
#define UFUNC_ERR_DEFAULT2                               \
        (UFUNC_ERR_PRINT << UFUNC_SHIFT_DIVIDEBYZERO) +  \
        (UFUNC_ERR_PRINT << UFUNC_SHIFT_OVERFLOW) +      \
        (UFUNC_ERR_PRINT << UFUNC_SHIFT_INVALID)

        /* Only internal -- not exported, yet*/
typedef struct {
        /* Multi-iterator portion --- needs to be present in this order
           to work with PyArray_Broadcast */
        PyObject_HEAD
        int  numiter;
        npy_intp size;
        npy_intp index;
        int nd;
        npy_intp dimensions[NPY_MAXDIMS];
        PyArrayIterObject *iters[NPY_MAXARGS];
        /*  End of Multi-iterator portion */

        /* The ufunc */
        PyUFuncObject *ufunc;

        /* The error handling */
        int errormask;         /* Integer showing desired error handling */
        PyObject *errobj;      /* currently a tuple with
                                  (string, func or obj with write method or None)
                               */
        int first;

        /* Specific function and data to use */
        PyUFuncGenericFunction function;
        void *funcdata;

        /* Loop method */
        int meth;

        /* Whether we need to copy to a buffer or not.*/
        int needbuffer[NPY_MAXARGS];
        int leftover;
        int ninnerloops;
        int lastdim;

        /* Whether or not to swap */
        int swap[NPY_MAXARGS];

        /* Buffers for the loop */
        char *buffer[NPY_MAXARGS];
        int bufsize;
        npy_intp bufcnt;
        char *dptr[NPY_MAXARGS];

        /* For casting */
        char *castbuf[NPY_MAXARGS];
        PyArray_VectorUnaryFunc *cast[NPY_MAXARGS];

        /* usually points to buffer but when a cast is to be
           done it switches for that argument to castbuf.
        */
        char *bufptr[NPY_MAXARGS];

        /* Steps filled in from iters or sizeof(item)
           depending on loop method.
        */
        npy_intp steps[NPY_MAXARGS];

        int obj;  /* This loop uses object arrays or needs the Python API */
                  /* Flags: UFUNC_OBJ_ISOBJECT, UFUNC_OBJ_NEEDS_API */
        int notimplemented; /* The loop caused notimplemented */
        int objfunc; /* This loop calls object functions
                        (an inner-loop function with argument types */
    
        /* generalized ufunc */
        npy_intp *core_dim_sizes;   /* stores sizes of core dimensions;
                                       contains 1 + core_num_dim_ix elements */
        npy_intp *core_strides;     /* strides of loop and core dimensions */
} PyUFuncLoopObject;

/* Could make this more clever someday */
#define UFUNC_MAXIDENTITY 32

typedef struct {
        PyObject_HEAD
        PyArrayIterObject *it;
        PyArrayObject *ret;
        PyArrayIterObject *rit;   /* Needed for Accumulate */
        int  outsize;
        npy_intp  index;
        npy_intp  size;
        char idptr[UFUNC_MAXIDENTITY];

        /* The ufunc */
        PyUFuncObject *ufunc;

        /* The error handling */
        int errormask;
        PyObject *errobj;
        int first;

        PyUFuncGenericFunction function;
        void *funcdata;
        int meth;
        int swap;

        char *buffer;
        int bufsize;

        char *castbuf;
        PyArray_VectorUnaryFunc *cast;

        char *bufptr[3];
        npy_intp steps[3];

        npy_intp N;
        int  instrides;
        int  insize;
        char *inptr;

        /* For copying small arrays */
        PyObject *decref;

        int obj;
        int retbase;

} PyUFuncReduceObject;


#if NPY_ALLOW_THREADS
#define NPY_LOOP_BEGIN_THREADS do {if (!(loop->obj & UFUNC_OBJ_NEEDS_API)) _save = PyEval_SaveThread();} while (0)
#define NPY_LOOP_END_THREADS   do {if (!(loop->obj & UFUNC_OBJ_NEEDS_API)) PyEval_RestoreThread(_save);} while (0)
#else
#define NPY_LOOP_BEGIN_THREADS
#define NPY_LOOP_END_THREADS
#endif

#define PyUFunc_One 1
#define PyUFunc_Zero 0
#define PyUFunc_None -1

#define UFUNC_REDUCE 0
#define UFUNC_ACCUMULATE 1
#define UFUNC_REDUCEAT 2
#define UFUNC_OUTER 3


typedef struct {
        int nin;
        int nout;
        PyObject *callable;
} PyUFunc_PyFuncData;

/* A linked-list of function information for
   user-defined 1-d loops.
 */
typedef struct _loop1d_info {
        PyUFuncGenericFunction func;
        void *data;
        int *arg_types;
        struct _loop1d_info *next;
} PyUFunc_Loop1d;


#include "__ufunc_api.h"

#define UFUNC_PYVALS_NAME "UFUNC_PYVALS"

#define UFUNC_CHECK_ERROR(arg)                                          \
        do {if ((((arg)->obj & UFUNC_OBJ_NEEDS_API) && PyErr_Occurred()) ||                         \
            ((arg)->errormask &&                                        \
             PyUFunc_checkfperr((arg)->errormask,                       \
                                (arg)->errobj,                          \
                                &(arg)->first)))                        \
                goto fail;} while (0)

/* This code checks the IEEE status flags in a platform-dependent way */
/* Adapted from Numarray  */

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

/*  OSF/Alpha (Tru64)  ---------------------------------------------*/
#if defined(__osf__) && defined(__alpha)

#include <machine/fpu.h>

#define UFUNC_CHECK_STATUS(ret) {               \
        unsigned long fpstatus;                 \
                                                \
        fpstatus = ieee_get_fp_control();                               \
        /* clear status bits as well as disable exception mode if on */ \
        ieee_set_fp_control( 0 );                                       \
        ret = ((IEEE_STATUS_DZE & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((IEEE_STATUS_OVF & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((IEEE_STATUS_UNF & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((IEEE_STATUS_INV & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        }

/* MS Windows -----------------------------------------------------*/
#elif defined(_MSC_VER)

#include <float.h>

  /* Clear the floating point exception default of Borland C++ */
#if defined(__BORLANDC__)
#define UFUNC_NOFPE _control87(MCW_EM, MCW_EM);
#endif

#define UFUNC_CHECK_STATUS(ret) {                \
        int fpstatus = (int) _clearfp();                        \
                                                                        \
        ret = ((SW_ZERODIVIDE & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((SW_OVERFLOW & fpstatus) ? UFUNC_FPE_OVERFLOW : 0)   \
                | ((SW_UNDERFLOW & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((SW_INVALID & fpstatus) ? UFUNC_FPE_INVALID : 0);    \
        }

/* Solaris --------------------------------------------------------*/
/* --------ignoring SunOS ieee_flags approach, someone else can
**         deal with that! */
#elif defined(sun) || defined(__BSD__) || defined(__OpenBSD__) || \
      (defined(__FreeBSD__) && (__FreeBSD_version < 502114)) || \
      defined(__NetBSD__)
#include <ieeefp.h>

#define UFUNC_CHECK_STATUS(ret) {                               \
        int fpstatus;                                           \
                                                                \
        fpstatus = (int) fpgetsticky();                                 \
        ret = ((FP_X_DZ  & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0)      \
                | ((FP_X_OFL & fpstatus) ? UFUNC_FPE_OVERFLOW : 0)      \
                | ((FP_X_UFL & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0)     \
                | ((FP_X_INV & fpstatus) ? UFUNC_FPE_INVALID : 0);      \
        (void) fpsetsticky(0);                                          \
        }

#elif defined(__GLIBC__) || defined(__APPLE__) || \
      defined(__CYGWIN__) || defined(__MINGW32__) || \
      (defined(__FreeBSD__) && (__FreeBSD_version >= 502114))

#if defined(__GLIBC__) || defined(__APPLE__) || \
    defined(__MINGW32__) || defined(__FreeBSD__)
#include <fenv.h>
#elif defined(__CYGWIN__)
#include "fenv/fenv.c"
#endif

#define UFUNC_CHECK_STATUS(ret) {                                       \
        int fpstatus = (int) fetestexcept(FE_DIVBYZERO | FE_OVERFLOW |  \
                                          FE_UNDERFLOW | FE_INVALID);   \
        ret = ((FE_DIVBYZERO  & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((FE_OVERFLOW   & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((FE_UNDERFLOW  & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((FE_INVALID    & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        (void) feclearexcept(FE_DIVBYZERO | FE_OVERFLOW |               \
                             FE_UNDERFLOW | FE_INVALID);                \
}

#elif defined(_AIX)

#include <float.h>
#include <fpxcp.h>

#define UFUNC_CHECK_STATUS(ret) { \
        fpflag_t fpstatus; \
 \
        fpstatus = fp_read_flag(); \
        ret = ((FP_DIV_BY_ZERO & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((FP_OVERFLOW & fpstatus) ? UFUNC_FPE_OVERFLOW : 0)   \
                | ((FP_UNDERFLOW & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((FP_INVALID & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        fp_swap_flag(0); \
}

#else

#define NO_FLOATING_POINT_SUPPORT
#define UFUNC_CHECK_STATUS(ret) { \
    ret = 0; \
  }

#endif

/*
 * THESE MACROS ARE DEPRECATED.
 * Use npy_set_floatstatus_* in the npymath library.
 */
#define generate_divbyzero_error() npy_set_floatstatus_divbyzero()
#define generate_overflow_error() npy_set_floatstatus_overflow()

  /* Make sure it gets defined if it isn't already */
#ifndef UFUNC_NOFPE
#define UFUNC_NOFPE
#endif


#ifdef __cplusplus
}
#endif
#endif /* !Py_UFUNCOBJECT_H */
