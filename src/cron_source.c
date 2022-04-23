#include <glib.h>
#include <math.h>
#include <time.h>

#include "cron_source.h"

static gboolean prepare (GSource * source, gint * timeout);
static gboolean
dispatch (GSource * source, GSourceFunc callback, gpointer user_data);
static void finalize (GSource * source);

GSourceFuncs g_cron_source_funcs = {
  .prepare = prepare,
  .check = NULL,
  .dispatch = dispatch,
  .finalize = finalize,
};

static gboolean
in (time_t t, const struct PulseTrain *train, time_t * pulse)
{
  g_return_val_if_fail (train != NULL, FALSE);
  g_return_val_if_fail (train->period != 0u, FALSE);

  t -= train->shift;

  if (t % train->period == 0u && train->width > 0u)
    {
      if (pulse != NULL)
        *pulse = t + train->shift;
      return TRUE;
    }

  time_t e = t - train->width;
  if (e % train->period == 0u && train->width < train->period)
    {
      if (pulse != NULL)
        *pulse = t + train->shift + train->period - train->width;
      return FALSE;
    }
  time_t a = ceil (e / (double) train->period);
  time_t b = floor (t / (double) train->period);
  if (pulse != NULL)
    *pulse = a * train->period + train->shift;
  return a <= b;
}

static gboolean
prepare (GSource * source, gint * timeout)
{
  g_return_val_if_fail (timeout != NULL, FALSE);
  g_return_val_if_fail (source != NULL, FALSE);

  *timeout = 1000;

  struct GCronSource *cron_source = (struct GCronSource *) source;
  time_t now = cron_source->override_timefunc == NULL
    ? time (NULL) : cron_source->override_timefunc (NULL,
                                                    cron_source->
                                                    override_timefuncdata);

  if (cron_source->pulse_trains == NULL)
    return FALSE;

  time_t outer_window = 0u;
  for (GSList * g = cron_source->pulse_trains; g != NULL; g = g->next)
    {
      time_t inner_window = now + 1;

      for (GSList * t = (GSList *) g->data; t != NULL; t = t->next)
        {
          time_t pulse;
          if (!in (now, (const struct PulseTrain *) t->data, &pulse))
            continue;
          if (pulse < inner_window)
            inner_window = pulse;
        }

      if (inner_window == now + 1)
        return FALSE;
      if (inner_window > outer_window)
        outer_window = inner_window;
    }
  if (outer_window == cron_source->last)
    return FALSE;
  cron_source->last = outer_window;
  return TRUE;
}

static gboolean
dispatch (GSource * source, GSourceFunc callback, gpointer user_data)
{
  g_return_val_if_fail (source != NULL, G_SOURCE_REMOVE);
  return callback != NULL ? callback (user_data) : G_SOURCE_CONTINUE;
}

static void
finalize (GSource * source)
{
  g_return_if_fail (source != NULL);

  struct GCronSource *cron_source = (struct GCronSource *) source;
  for (GSList * g = cron_source->pulse_trains; g != NULL; g = g->next)
    {
      g_slist_free (g_steal_pointer (&g->data));
    }
  g_slist_free (g_steal_pointer (&cron_source->pulse_trains));
}
