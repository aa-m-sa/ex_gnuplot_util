// ex_gnuplot.c

#define _XOPEN_SOURCE 500

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "ex_util.h"
#include "ex_gnuplot.h"

/**
 * Function declarations. (''Private'')
 */

/**
 * Creates a temporary file name for writing
 *
 * @author Peter (12/9/2011)
 *
 * @param handle
 *
 * @return char const * Pointer to file name string.
 */
char const *gnuplot_tmpfile(gnuplot_ctrl *handle);


/**
 * Plot a temporary file.
 *
 * @author Peter (12/9/2011)
 *
 * @param handle
 * @param tmp_filename
 * @param title
 */
void gnuplot_plot_atmpfile(gnuplot_ctrl *handle, char const *tmp_filename, char const *title, char const *style);


/**
 * Function definitions.
 */


/**
 * gnuplot_init
 *
 * This opens up a new gnuplot session, ready for input. The struct
 * controlling a gnuplot session should remain opaque and only be
 * accessed through the provided functions.
 *
 * The session must be closed using gnuplot_close().
 */
gnuplot_ctrl *gnuplot_init(void)
{
    gnuplot_ctrl *handle;
    /*
     * Structure initialization:
     */
    handle = (gnuplot_ctrl*)malloc(sizeof(gnuplot_ctrl)) ;
    handle->nplots = 0 ;
    handle->ntmp = 0 ;

    handle->gnucmd = popen("gnuplot --persist", "w");
    if (handle->gnucmd == NULL) {
        fprintf(stderr, "error starting gnuplot, is gnuplot in your path?\n") ;
        free(handle) ;
        return NULL ;
    }

    for (int i=0; i<GP_MAX_TMP_FILES; i++)
    {
        handle->tmp_filename_tbl[i] = NULL;
    }
    return handle;
}

/*
 *  gnuplot_close
 *
 *  Kills the child PID and deletes all opened temporary files.
 *  It is mandatory to call this function to close the handle, child process
 *  might survive.
 *
 */
void gnuplot_close(gnuplot_ctrl *handle)
{

    if (pclose(handle->gnucmd) == -1) {
        fprintf(stderr, "problem closing communication to gnuplot\n") ;
        return;
    }
    if (handle->ntmp) {
        for (int i=0; i<handle->ntmp; i++) {
            remove(handle->tmp_filename_tbl[i]);
            free(handle->tmp_filename_tbl[i]);
            handle->tmp_filename_tbl[i] = NULL;

        }
    }
    free(handle) ;
    return;
}

/*
 * gnuplot_cmd
 *
 * This sends a string to an active gnuplot session, to be executed.
 * There is strictly no way to know if the command has been
 * successfully executed or not.
 * The command syntax is the same as printf.
 */
void gnuplot_cmd(gnuplot_ctrl *handle, char const *cmd, ...)
{
    va_list ap;

    va_start(ap, cmd);
    vfprintf(handle->gnucmd, cmd, ap);
    va_end(ap);

    fputs("\n", handle->gnucmd) ;
    fflush(handle->gnucmd) ;
    return;
}


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
void ex_plot_xf(gnuplot_ctrl *handle, const int n, double xlow, double xhigh, double xstep, db_fx f1, const char *title1, const char *style1, ...)
{
    va_list arg_list;
    va_start(arg_list, style1);

    db_fx flist[n];
    const char *titles[n];
    const char *styles[n];

    flist[0] = f1;
    titles[0] = title1;
    styles[0] = style1;

    for (int i = 1; i < n; ++i) {
        flist[i] = va_arg(arg_list, db_fx);
        titles[i] = va_arg(arg_list, char *);
        styles[i] = va_arg(arg_list, char *);
    }

    gsl_vector *xdata = vec_linspace(xlow, xhigh, xstep);

    for (int i = 0; i < n; ++i) {
        gsl_vector *ydata = vec_fx(flist[i], xdata);
        gnuplot_xy(handle, xdata, ydata, titles[i], styles[i]);
    }

    va_end(arg_list);
}


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
void ex_plot_xys(gnuplot_ctrl *handle, const int n, gsl_vector *xdata1, gsl_vector *ydata1, const char *title1, const char *style1, ...)
{
    va_list arg_list;
    va_start(arg_list, style1);

    const char *titles[n];
    const char *styles[n];
    gsl_vector *xdata[n];
    gsl_vector *ydata[n];

    xdata[0] = xdata1;
    ydata[0] = ydata1;
    titles[0] = title1;
    styles[0] = style1;

    for (int i = 1; i < n; ++i) {
        xdata[i] = va_arg(arg_list, gsl_vector *);
        ydata[i] = va_arg(arg_list, gsl_vector *);
        titles[i] = va_arg(arg_list, char *);
        styles[i] = va_arg(arg_list, char *);
    }

    for (int i = 0; i < n; ++i) {
        gnuplot_xy(handle, xdata[i], ydata[i], titles[i], styles[i]);
    }

    va_end(arg_list);
}

void ex_plot_histo(gnuplot_ctrl *handle, const int n_clusters, const char *histostyle,
        const char *style, gsl_vector *labels, gsl_vector *xdata1, const char *title1, ...)
{
    int npts = xdata1->size;

    FILE* tmpfd ;
    char const * tmpfname;

    if (handle==NULL || (npts<1)) {
        printf("error!!!"); 
        return;
    }

    /* Open temporary file for output   */
    tmpfname = gnuplot_tmpfile(handle);
    tmpfd = fopen(tmpfname, "w");

    if (tmpfd == NULL) {
        printf("cannot create temporary file: exiting plot") ;
        fprintf(stderr,"cannot create temporary file: exiting plot") ;
        return ;
    }

    va_list arg_list;
    gsl_vector *xdata[n_clusters];
    const char *titles[n_clusters];
    va_start(arg_list, title1);
    xdata[0] = xdata1;
    titles[0] = title1;
    for (int i = 1; i < n_clusters; ++i) {
        xdata[i] = va_arg(arg_list, gsl_vector *);
        titles[i] = va_arg(arg_list, char*);
    }

    fprintf(tmpfd, "n ");
    for (int i = 0; i < n_clusters; ++i) {
        fprintf(tmpfd, " %s", titles[i]);
    }
    fprintf(tmpfd, "\n");

    for (int i = 0; i < npts; ++i) {
        double li = gsl_vector_get(labels, i);
        fprintf(tmpfd, "%.3lf", li);

        for (int j = 0; j < n_clusters; ++j) {
            printf("%lf", gsl_vector_get(xdata[j], i));
            fprintf(tmpfd, " %.18e", gsl_vector_get(xdata[j], i));
        }
        fprintf(tmpfd, "\n");

    }

    fclose(tmpfd);


    gnuplot_cmd(handle, "set style data histogram");
    if (histostyle != NULL && strlen(histostyle) > 0)
        gnuplot_cmd(handle, "set style histogram %s", histostyle);
    else
        gnuplot_cmd(handle, "set style histogram cluster gap 1");

    if (style != NULL && strlen(style) > 0)
        gnuplot_cmd(handle, "set style %s", style); 
    else
        gnuplot_cmd(handle, "set style fill solid border -1");


    gnuplot_cmd(handle, "plot for [COL=2:%d] \"%s\" using COL:xticlabels(1)", n_clusters, tmpfname);
    handle->nplots++;
}

/*
 * gnuplot_xy
 * plots one xdata, ydata line plot
 */
void gnuplot_xy(gnuplot_ctrl *handle, gsl_vector *xdata, gsl_vector *ydata, const char *title, const char *style)
{
    // gnuplot -persist
    int npts = ydata->size;

    FILE* tmpfd ;
    char const * tmpfname;

    if (handle==NULL || (npts<1)) {
        printf("error!!!"); 
        return;
    }

    /* Open temporary file for output   */
    tmpfname = gnuplot_tmpfile(handle);
    tmpfd = fopen(tmpfname, "w");

    if (tmpfd == NULL) {
        printf("cannot create temporary file: exiting plot") ;
        fprintf(stderr,"cannot create temporary file: exiting plot") ;
        return ;
    }

    double xmin = gsl_vector_get(xdata, 0);
    double xmax = gsl_vector_get(xdata, 0);
    double ymin = gsl_vector_get(xdata, 0);
    double ymax = gsl_vector_get(xdata, 0);

    for (int i = 0; i < npts; ++i) {
        double xi = gsl_vector_get(xdata, i);
        double yi = gsl_vector_get(ydata, i);
        fprintf(tmpfd, "%.18e %.18e\n", xi, yi);

        xmin = (xmin > xi) ? xi : xmin;
        xmax = (xmax < xi) ? xi : xmax;
        ymin = (ymin > yi) ? yi : ymin;
        ymax = (ymax < yi) ? yi : ymax;
    }

    fclose(tmpfd);

    double lx = xmax - xmin;
    double ly = ymax - ymin;

    gnuplot_cmd(handle, "set xrange [%lf:%lf]", xmin-0.05*lx, xmax+0.05*lx);
    gnuplot_cmd(handle, "set yrange [%lf:%lf]", ymin-0.05*ly, ymax+0.05*ly);

    gnuplot_plot_atmpfile(handle,tmpfname,title,style);
}


/*
 * gnuplot_plot_atmpfile
 *
 * plots a tmpfile with given title and style
 */
void gnuplot_plot_atmpfile(gnuplot_ctrl *handle, char const *tmp_filename, char const *title, char const *style)
{
    char const *cmd = (handle->nplots > 0) ? "replot" : "plot";
    title = (title == NULL) ? "(none)" : title;
    gnuplot_cmd(handle, "%s \"%s\" title \"%s\" with %s",
            cmd, tmp_filename, title, style) ;
    handle->nplots++;
    return;
}


/* gnuplot_tmpfile
 *
 * create a tmpfile
 * UNIX only
 */
char const *gnuplot_tmpfile(gnuplot_ctrl *handle)
{
    static char const * tmp_filename_template = "gnuplot_tmpdatafile_XXXXXX";
    char *              tmp_filename = NULL;
    int                 tmp_filelen = strlen(tmp_filename_template);

    int                 unx_fd;

    assert(handle->tmp_filename_tbl[handle->ntmp] == NULL);

    /* Open one more temporary file? */
    if (handle->ntmp == GP_MAX_TMP_FILES - 1) {
        fprintf(stderr,
                "maximum # of temporary files reached (%d): cannot open more",
                GP_MAX_TMP_FILES) ;
        return NULL;
    }

    tmp_filename = (char*) malloc(tmp_filelen+1);
    if (tmp_filename == NULL)
    {
        return NULL;
    }
    strcpy(tmp_filename, tmp_filename_template);

    unx_fd = mkstemp(tmp_filename);
    if (unx_fd == -1)
    {
        return NULL;
    }
    close(unx_fd);

    handle->tmp_filename_tbl[handle->ntmp] = tmp_filename;
    handle->ntmp ++;
    return tmp_filename;
}
