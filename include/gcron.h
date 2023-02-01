#pragma once

#include <glib.h>

GSource *g_cron_source_new (gchar * schedule);

guint g_cron_add (gchar * schedule, GSourceFunc function, gpointer data);
guint g_cron_add_full (gint priority, gchar * schedule, GSourceFunc function,
                       gpointer data, GDestroyNotify notify);
