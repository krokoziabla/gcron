#pragma once

#include <glib.h>

GSource *g_cron_source_new (gchar * schedule);

/**
 * g_cron_add: (skip)
 */
guint g_cron_add (gchar * schedule, GSourceFunc function, gpointer data);

/**
 * g_cron_add_full: (rename-to g_cron_add)
 */
guint g_cron_add_full (gint priority, gchar * schedule, GSourceFunc function,
                       gpointer data, GDestroyNotify notify);
