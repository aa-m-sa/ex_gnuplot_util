#ifndef EX_UTIL
#define EX_UTIL

#include <gsl/gsl_vector.h>

typedef double (*db_fxy)(double x, double y);
typedef double (*db_fx) (double);

/* print_table_fxy
 * prints f(x,y) tables given range for x and y
 */
void print_table_fxy(db_fxy fun,
        double x_start, double x_end, double x_step, char xch,
        double y_start, double y_end, double y_step, char ych);

/*
 * vec_linspace
 * returns a pointer to a gsl_vector from xlow (inclusive) to xhigh (exclusive)
 * with steps xstep
 */
gsl_vector *vec_linspace(double xlow, double xhigh, double xstep);

/*
 * vec_fx
 * given a function db_fx and vector of xpoints xdata, returns a vector f(x)
 */
gsl_vector *vec_fx(db_fx f, gsl_vector *xdata);

/*
 * sign
 */
int sign(double x);
#endif
