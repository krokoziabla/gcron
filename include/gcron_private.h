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

struct GCronSource
{
  GSource base_source;
  time_t last;
  GSList *pulse_trains;
  GDestroyNotify pulse_train_destructor;
    time_t (*override_timefunc) (time_t *, gpointer);
  gpointer override_timefuncdata;

};

extern GSourceFuncs g_cron_source_funcs;

typedef GSList *(*ForEachFunc) (GSList *, guint *, struct PulseTrain *);
GSList *for_each (gchar *, ForEachFunc, struct PulseTrain *, guint, guint);

GSList *make (GSList * non_leap_trains);
GSList *make_from_monthly_pulses (GSList * pulses);
