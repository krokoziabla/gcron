#include <glib.h>

#include "gcron_private.h"

static void
set_unit_day (struct PulseTrain *train, gpointer userdata)
{
  g_return_if_fail (train != NULL);

  (void) userdata;

  g_slist_foreach (train->children, (GFunc) set_unit_day, NULL);
  train->unit = 60u * 60u * 24u;
}

static void
leapscale (struct PulseTrain *train, gpointer userdata)
{
  if (train->shift >= 59u)
    ++train->shift;
  else if (train->shift + train->width >= 59u)
    {
      g_slist_foreach (train->children, (GFunc) leapscale, userdata);
      ++train->width;
    }
  ++train->period;
}

static GSList *
make_leap_twin (GSList * trains)
{
  GSList *leap_trains = g_slist_copy (trains);
  for (GSList * t = leap_trains; t != NULL; t = t->next)
    t->data = g_memdup2 (t->data, sizeof (struct PulseTrain));
  g_slist_foreach (leap_trains, (GFunc) leapscale, NULL);
  return leap_trains;
}

GSList *
make (GSList * non_leap_trains)
{
  g_return_val_if_fail (non_leap_trains != NULL, NULL);

  GSList *leap_trains = make_leap_twin (non_leap_trains), *trains = NULL;

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
  trains = g_slist_prepend (trains, train);

  train->period = 146097u;
  train->width = 36525u;
  train->shift = 10957u;

  {
    GSList **trains = &train->children;

    train = g_new0 (struct PulseTrain, 1);
    *trains = g_slist_prepend (*trains, train);

    train->period = 1461u;
    train->width = 366u;
    train->shift = 0u;
    train->children = leap_trains;

    train = g_new0 (struct PulseTrain, 1);
    *trains = g_slist_prepend (*trains, train);

    train->period = 1461u;
    train->width = 1095u;
    train->shift = 366u;
    train->children = non_leap_trains;
  }

  GSList *children = train->children;

  train = g_new0 (struct PulseTrain, 1);
  trains = g_slist_prepend (trains, train);

  train->period = 146097u;
  train->width = 109572u;
  train->shift = 10957u + 36525u;

  {
    GSList **trains = &train->children;

    train = g_new0 (struct PulseTrain, 1);
    *trains = g_slist_prepend (*trains, train);

    train->period = 36524u;
    train->width = 1460u;
    train->shift = 0u;
    train->children = non_leap_trains;

    train = g_new0 (struct PulseTrain, 1);
    *trains = g_slist_prepend (*trains, train);

    train->period = 36524u;
    train->width = 35064u;
    train->shift = 1460u;
    train->children = children;
  }

  g_slist_foreach (trains, (GFunc) set_unit_day, NULL);

  return trains;
}

GSList *
make_from_monthly_pulses (GSList * d28_trains)
{
  GSList *d30_trains = make_leap_twin (d28_trains);
  g_slist_foreach (d30_trains, (GFunc) leapscale, NULL);
  GSList *d31_trains = make_leap_twin (d30_trains);

  GSList *non_leap_trains = NULL;

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 31u;
  train->shift = 0u;
  train->children = d31_trains;

  train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 28u;
  train->shift = 31u;
  train->children = d28_trains;

  train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 153u;
  train->shift = 59u;

  GSList **children = &train->children;

  {
    train = g_new0 (struct PulseTrain, 1);
    *children = g_slist_prepend (*children, train);

    train->period = 61u;
    train->width = 31u;
    train->shift = 0u;
    train->children = d31_trains;

    train = g_new0 (struct PulseTrain, 1);
    *children = g_slist_prepend (*children, train);

    train->period = 61u;
    train->width = 30u;
    train->shift = 31u;
    train->children = d30_trains;
  }

  train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 153u;
  train->shift = 212u;
  train->children = *children;

  return make (non_leap_trains);
}
