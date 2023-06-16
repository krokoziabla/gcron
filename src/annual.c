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


static time_t
cut (time_t S, time_t P, time_t whence, time_t wither, time_t X)
{
  time_t WH = (whence + S) % P;
  time_t WI = (wither + S) % P;

  time_t d = 0u;
  if (WH < X)
    {
      d += X - WH;
      if (WI < X)
        {
          time_t dd = X - WI;
          d = dd < d ? d - dd : dd - d;
        }
    }
  else if (WI < X)
    d += X - WI;

  return (whence - wither) * (WH - WI) > 0 ? X - d : 0u + d;
}

static void
cut_one_day (struct PulseTrain *train, gpointer userdata)
{
  time_t whence = (time_t) userdata;
  time_t wither = whence + 1u;
  time_t shift = wither - whence;

  g_return_if_fail (shift < train->period);

  time_t W = train->width;
  time_t P = train->period;

  train->period -= shift;
  train->width = W < P
    ? cut (P - train->shift % P, P, whence, wither, train->width) : W - shift;
  train->shift = cut (0, P, whence, wither, train->shift % P);
}

static GSList *
clone_trains (GSList *trains)
{
  GSList *clones = g_slist_copy (trains);
  for (GSList * t = clones; t != NULL; t = t->next)
    t->data = g_memdup2 (t->data, sizeof (struct PulseTrain));
  return clones;
}

GSList *
make (GSList *leap_trains)
{
  g_return_val_if_fail (leap_trains != NULL, NULL);

  GSList *non_leap_trains = clone_trains (leap_trains), *trains = NULL;
  g_slist_foreach (non_leap_trains, (GFunc) cut_one_day, (gpointer) 59u);

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
  trains = g_slist_prepend (trains, train);

  train->period = 146097u;
  train->width = 36525u;
  train->shift = 10957u;

  GSList **children = &train->children;
  {
    train = g_new0 (struct PulseTrain, 1);
    *children = g_slist_prepend (*children, train);

    train->period = 1461u;
    train->width = 366u;
    train->shift = 0u;
    train->children = leap_trains;

    train = g_new0 (struct PulseTrain, 1);
    *children = g_slist_prepend (*children, train);

    train->period = 1461u;
    train->width = 1095u;
    train->shift = 366u;
    train->children = non_leap_trains;
  }

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
    train->children = *children;
  }

  g_slist_foreach (trains, (GFunc) set_unit_day, NULL);

  return trains;
}

GSList *
make_from_monthly_pulses (GSList *d31_trains)
{
  GSList *d30_trains = clone_trains (d31_trains);
  g_slist_foreach (d30_trains, (GFunc) cut_one_day, (gpointer) 30u);
  GSList *d29_trains = clone_trains (d30_trains);
  g_slist_foreach (d29_trains, (GFunc) cut_one_day, (gpointer) 29u);

  GSList *leap_trains = NULL;

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 31u;
  train->shift = 0u;
  train->children = d31_trains;

  train = g_new0 (struct PulseTrain, 1);
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 29u;
  train->shift = 31u;
  train->children = d29_trains;

  train = g_new0 (struct PulseTrain, 1);
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 153u;
  train->shift = 60u;

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
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 153u;
  train->shift = 213u;
  train->children = *children;

  return make (leap_trains);
}
