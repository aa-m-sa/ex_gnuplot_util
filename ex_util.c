#include <stdio.h>
#include <math.h>
#include "ex_util.h"

/* end_cond
 * helper function for print_table_fxy:
 * returns 1 when iteration should stop
 */
int end_cond(double x, double x_step, double x_end) {
    if (x_step > 0) {
        if (x < x_end)
            return 0;
        else
            return 1;
    }
    if (x_step < 0) {
        if (x > x_end)
            return 0;
        else
            return 1;
    }
    return 1;
}

/* print_table_fxy
 * prints f(x,y) tables given range for x and y
 */
void print_table_fxy(db_fxy fun,
        double x_start, double x_end, double x_step, char xch,
        double y_start, double y_end, double y_step, char ych)
{
    printf("%c, %c\n", ych, xch);
    printf("         ");
    for (double x = x_start; end_cond(x, x_step, x_end) == 0; x += x_step) {
        printf("% 8.4lf ", x);
    }
    printf("\n");
    for (double y = y_start; end_cond(y, y_step, y_end) == 0; y += y_step) {
        printf("% 8.4lf ", y);
        for (double x = x_start; end_cond(x, x_step, x_end) == 0; x += x_step) {
            double f = fun(x,y);
            printf("% 8.4lf ", f);
        }
        printf("\n");
    }
}

/*
 * sign
 */
int sign(double x)
{
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 1;
}

/* vec_linspace
 * creates a linspace vector to gsl_vector
 *
 * returns size of a pointer to gsl_vector
 */
gsl_vector *vec_linspace(double xlow, double xhigh, double xstep)
{
    int n = floor((xhigh - xlow)/xstep);
    gsl_vector *v = gsl_vector_alloc(n);

    double xi = xlow;
    for (int i = 0; i < n; ++i) {
        gsl_vector_set(v, i, xi);
        xi += xstep;
    }

    return v;
}

/*
 * vec_fx
 * given a function db_fx and vector of xpoints xdata, returns a vector f(x)
 */

gsl_vector *vec_fx(db_fx f, gsl_vector *xdata)
{
    int n = xdata->size;
    gsl_vector *fx = gsl_vector_alloc(n);
    for (int i = 0; i < n; ++i) {
        double x = gsl_vector_get(xdata, i);
        gsl_vector_set(fx, i, f(x));
    }

    return fx;
}
