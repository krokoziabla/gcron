#include <glib.h>

#include "annual.h"
#include "cron_source.h"

static void
set_unit_day (gpointer data, gpointer user_data)
{
  g_return_if_fail (data != NULL);

  struct PulseTrain *train = (struct PulseTrain *) data;

  train->unit = 60u * 60u * 24u;
}


static GSList *
make (GSList * leap_trains, GSList * non_leap_trains)
{
  g_return_val_if_fail (leap_trains != NULL, NULL);

  GSList *trains = NULL;

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

    if (non_leap_trains != NULL)
      {
        train = g_new0 (struct PulseTrain, 1);
        *trains = g_slist_prepend (*trains, train);

        train->period = 1461u;
        train->width = 1095u;
        train->shift = 366u;
        train->children = non_leap_trains;
      }
  }

  GSList *children = train->children;

  train = g_new0 (struct PulseTrain, 1);
  trains = g_slist_prepend (trains, train);

  train->period = 146097u;
  train->width = 109572u;
  train->shift = 10957u + 36525u;

  {
    GSList **trains = &train->children;

    if (non_leap_trains != NULL)
      {
        train = g_new0 (struct PulseTrain, 1);
        *trains = g_slist_prepend (*trains, train);

        train->period = 36524u;
        train->width = 1460u;
        train->shift = 0u;
        train->children = non_leap_trains;
      }

    train = g_new0 (struct PulseTrain, 1);
    *trains = g_slist_prepend (*trains, train);

    train->period = 36524u;
    train->width = 35064u;
    train->shift = 1460u;
    train->children = children;
  }

  pulse_train_for_each (trains, set_unit_day, NULL);

  return trains;
}

GSList *
make_from_annual_pulses (GSList * pulses)
{
  GSList *leap_trains = NULL, *non_leap_trains = NULL;

  while (pulses != NULL)
    {
      struct AnnualPulse *pulse = (struct AnnualPulse *) pulses->data;

      struct PulseTrain *train = g_new0 (struct PulseTrain, 1);

      train->period = 366u;
      train->width = pulse->width;
      train->shift = pulse->leap_shift;

      leap_trains = g_slist_prepend (leap_trains, train);

      train = g_new0 (struct PulseTrain, 1);

      train->period = 365u;
      train->width = pulse->width;
      train->shift = pulse->non_leap_shift;

      non_leap_trains = g_slist_prepend (non_leap_trains, train);

      pulses = pulses->next;
    }

  return make (leap_trains, non_leap_trains);
}

GSList *
make_from_pulse (struct AnnualPulse *pulse)
{
  GSList list = {.data = (gpointer) pulse,.next = NULL };
  return make_from_annual_pulses (&list);
}

static struct PulseTrain *
make_from_monthly_pulse (struct MonthlyPulse *pulse, time_t period)
{
  g_return_val_if_fail (pulse != NULL, NULL);
  g_return_val_if_fail (period != 0u, NULL);

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);

  train->period = period;
  train->width = pulse->width;
  train->shift = pulse->shift;

  return train;
}

GSList *
make_from_monthly_pulses (GSList * pulses)
{
  GSList *d28_trains = NULL,
    *d29_trains = NULL, *d30_trains = NULL, *d31_trains = NULL;

  while (pulses != NULL)
    {
      d28_trains =
        g_slist_prepend (d28_trains,
                         make_from_monthly_pulse ((struct MonthlyPulse
                                                   *) pulses->data, 28u));
      d29_trains =
        g_slist_prepend (d29_trains,
                         make_from_monthly_pulse ((struct MonthlyPulse
                                                   *) pulses->data, 29u));
      d30_trains =
        g_slist_prepend (d30_trains,
                         make_from_monthly_pulse ((struct MonthlyPulse
                                                   *) pulses->data, 30u));
      d31_trains =
        g_slist_prepend (d31_trains,
                         make_from_monthly_pulse ((struct MonthlyPulse
                                                   *) pulses->data, 31u));
      pulses = pulses->next;
    }

  GSList *leap_trains = NULL;

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 60u;
  train->shift = 0u;

  {
    GSList **leap_trains = &train->children;

    train = g_new0 (struct PulseTrain, 1);
    *leap_trains = g_slist_prepend (*leap_trains, train);

    train->period = 60u;
    train->width = 31u;
    train->shift = 0u;
    train->children = d31_trains;

    train = g_new0 (struct PulseTrain, 1);
    *leap_trains = g_slist_prepend (*leap_trains, train);

    train->period = 60u;
    train->width = 29u;
    train->shift = 31u;
    train->children = d29_trains;
  }

  train = g_new0 (struct PulseTrain, 1);
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 153u;
  train->shift = 60u;

  {
    GSList **leap_trains = &train->children;

    train = g_new0 (struct PulseTrain, 1);
    *leap_trains = g_slist_prepend (*leap_trains, train);

    train->period = 61u;
    train->width = 31u;
    train->shift = 0u;
    train->children = d31_trains;

    train = g_new0 (struct PulseTrain, 1);
    *leap_trains = g_slist_prepend (*leap_trains, train);

    train->period = 61u;
    train->width = 30u;
    train->shift = 31u;
    train->children = d30_trains;
  }

  GSList *children = train->children;

  train = g_new0 (struct PulseTrain, 1);
  leap_trains = g_slist_prepend (leap_trains, train);

  train->period = 366u;
  train->width = 153u;
  train->shift = 213u;
  train->children = children;

  GSList *non_leap_trains = NULL;

  train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 59u;
  train->shift = 0u;

  {
    GSList **non_leap_trains = &train->children;

    train = g_new0 (struct PulseTrain, 1);
    *non_leap_trains = g_slist_prepend (*non_leap_trains, train);

    train->period = 60u;
    train->width = 31u;
    train->shift = 0u;
    train->children = d31_trains;

    train = g_new0 (struct PulseTrain, 1);
    *non_leap_trains = g_slist_prepend (*non_leap_trains, train);

    train->period = 60u;
    train->width = 28u;
    train->shift = 31u;
    train->children = d28_trains;
  }

  train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 153u;
  train->shift = 59u;
  train->children = children;

  train = g_new0 (struct PulseTrain, 1);
  non_leap_trains = g_slist_prepend (non_leap_trains, train);

  train->period = 365u;
  train->width = 153u;
  train->shift = 212u;
  train->children = children;

  return make (leap_trains, non_leap_trains);
}

GSList *
make_from_leap_pulse (struct LeapPulse *pulse)
{
  GSList *leap_trains = NULL;
  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);

  train->period = 366u;
  train->width = pulse->width;
  train->shift = pulse->shift;

  leap_trains = g_slist_prepend (leap_trains, train);

  return make (leap_trains, NULL);
}
