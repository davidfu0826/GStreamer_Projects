#include <string.h>

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

/* Handle low-level stuff which GTK does not need to handle */
#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif

/* Structure to contain all our information, so we can pass it around */
typedef struct _CustomData {
    GstElement *playbin;              /* Our pipeline */

    GtkWidget *slider;                /* Slider widget to keep track of current position */
    GtkWidget *streams_list;          /* Text widget to display info about the streams */
    gulong slider_update_signal_id;   /* Signal ID for slider update signal */

    GstState state;                   /* Current state of pipeline */
    gint64 duration;                  /* Duration of clip in nanoseconds */
} CustomData;


/* This function is called when the GUI toolkit creates the physical window that will hold the video.
 * At this point we can retrieve its handler (which has a different meaning depending on the window system)
 * and pass it to GStreamer through the VideoOverlay interface. */
static void realize_cb (GtkWidget *widget, CustomData *data) {
    GdkWindow *window = gtk_widget_get_window (widget);
    guintptr window_handle;

    if (!gdk_window_ensure_native (window))
        g_error ("Couldn't create native window needed for GstVideoOverlay!");

#if defined (GDK_WINDOWING_X11)
    window_handle = GDK_WINDOW_XID (window);
#elif defined (GDK_WINDOWING_WIN32)
    window_handle = (guintptr)GDKWINDOW_HWND (window);
#elif defined (GDK_WINDOWING_QUARTZ)
    window_handle = gdk_quartz_window_get_nsview (window);
#endif

    gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (data->playbin), window_handle);
}

/* This function is called when the PLAY button is clicked */
static void play_cb (GtkButton *button, CustomData *data) {
    gst_element_set_state (data->playbin, GST_STATE_PLAYING);
}

/* This function is called when the PAUSE button is clicked */
static void pause_cb (GtkButton *button, CustomData *data) {
    gst_element_set_state (data->playbin, GST_STATE_PAUSED);    
}
/* This function is called when the STOP button is clicked */
static void stop_cb (GtkButton *button, CustomData *data) {
    gst_element_set_state (data->playbin, GST_STATE_READY);    
}
/* This function is called when the main window is closed */
static void delete_event_cb (GtkWidget *widget, GdkEvent *event, CustomData *data) {
    stop_cb (NULL, data);
    gtk_main_quit ();
}

/* Function called when window need to be redrawn. GStreamer takes care of this in PLAYING and PAUSED state, otherwise, we
 * simply draw a black rectangle to avoid garbage showing up */
static gboolean draw_cb (GtkWidget *widget, cairo_t *cr, CustoData *data) {
    if (data->state < GST_STATE_PAUSED) {
        GstAllocation allocation;

        /* Cairo is a 2D graphics library which we use to clean the video window.
         * It is used by GStreamer for other reasons, so it will always be available to us. */
        gtk_widget_get_allocation (widget, &allocation);
        cairo_set_source_rgb (cr, 0, 0, 0);
        cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
        cairo_fill (cr);
    }

    return FALSE;
}

/* This function is called when slider changes its position. We perform seek to the new position here. */
static void slider_cv (GtkRange *range, CustomData *data) {
    gdouble value = gtk_range_get_value (GTK_RANGE (data->slider));
    gst_element_seek_simple (data->playbin, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, (gint64)(value * GST_SECOND));
}

/* This creates all the GTK+ widgets that compose our application, and registers and callbacks */
static void create_ui (CustomData *data) {
    
}

