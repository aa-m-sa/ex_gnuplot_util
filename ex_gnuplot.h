/**
 * ex_gnuplot.h
 *
 * utility functions for plotting with gnuplot (v 4.6)
 *
 *  - minimizes boilerplate needed to use
 *  - popen -> process piping available, less temp files
 *  - idea based on myexamples/gnuplot (syntax) and gnuplot_i by N. Devillard (pipes), except a bit simpler
 *
 * Aaro Salosensaari 2016
 *
 * Original gnuplot_i was written by N. Devillard 1998 (version 2.10 2003) and is in public domain.
 */

#ifndef EX_GNUPLOT
#define EX_GNUPLOT

#include <stdarg.h>
#include <stdio.h>
#include <gsl/gsl_vector.h>
#include "ex_util.h"

#define GP_MAX_TMP_FILES 32

typedef struct {
    /* Pipe to gnuplot process */
    FILE *gnucmd ;

    /* Number of currently active plots */
    int nplots ;

    /* Pointer to table of names of temporary files */
    char *tmp_filename_tbl[GP_MAX_TMP_FILES] ;

    /* Number of temporary files */
    int ntmp ;
} gnuplot_ctrl ;


/**
 * Public interface function declarations
 */

gnuplot_ctrl *gnuplot_init(void);

void gnuplot_close(gnuplot_ctrl *handle);

/*
 * gnuplot_cmd
 *
 * This sends a string to an active gnuplot session, to be executed.
 * There is strictly no way to know if the command has been
 * successfully executed or not.
 * The command syntax is the same as printf.
 */
void gnuplot_cmd(gnuplot_ctrl *handle, char const *cmd, ...);

/*
 * ex_plot_xf can plot a variable number n of x, f(x) plots.
 *
 * n: number of plots
 * xlow:
 * xhigh:
 * xstep: (self-evident)
 * f1: function to plot
 * title1: title
 * style1: gnuplot style spef
 * (repeat fi, titlei, stylei until i=n)
 */
void ex_plot_xf(gnuplot_ctrl *handle, const int n, double xlow, double xhigh, double xstep, db_fx f1, const char *title1, const char *style1, ...);


/*
 * ex_plot_xys can plot a variable number n of x, y plots.
 *
 * n: number of plots
 * xdata: x points
 * ydata: y points
 * title1: title
 * style1: gnuplot style spef
 * (repeat xdatai, ydatai, titlei, stylei until i=n)
 */
void ex_plot_xys(gnuplot_ctrl *handle, const int n, gsl_vector *xdata1, gsl_vector *ydata1, const char *title1, const char *style1, ...);




void gnuplot_xy(gnuplot_ctrl *handle, gsl_vector *xdata, gsl_vector *ydata, const char *title1, const char *style1);


#endif /* ifndef EX_GNUPLOT */

