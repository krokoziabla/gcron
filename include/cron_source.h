#pragma once

#include <time.h>
#include <glib.h>

struct PulseTrain
{
  time_t period;
  time_t width;
  time_t shift;
  time_t unit;
  GSList *children;
};

extern void pulse_train_for_each (GSList * trains, GFunc func,
                                  gpointer user_data);

struct GCronSource
{
  GSource base_source;
  time_t last;
  GSList *pulse_trains;
  GDestroyNotify pulse_train_destructor;
    time_t (*override_timefunc) (time_t * tloc, gpointer data);
  gpointer override_timefuncdata;

};

extern GSourceFuncs g_cron_source_funcs;

GSource *g_cron_source_new (gchar * schedule);
