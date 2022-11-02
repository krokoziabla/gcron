#include <glib.h>
#include <math.h>
#include <time.h>

#include "cron_source.h"

typedef gboolean (*GPrepareFunc) (GSource * source, gint * timeout_);
typedef void (*GFinaliseFunc) (GSource * source);

static gboolean prepare (struct GCronSource *source, gint * timeout);
static gboolean
dispatch (GSource * source, GSourceFunc callback, gpointer user_data);
static void finalize (struct GCronSource *source);

GSourceFuncs g_cron_source_funcs = {
  .prepare = (GPrepareFunc) prepare,
  .check = NULL,
  .dispatch = dispatch,
  .finalize = (GFinaliseFunc) finalize,
};

static gboolean
in_itself (time_t t, struct PulseTrain *train, time_t * pulse)
{
  g_return_val_if_fail (train != NULL, FALSE);
  g_return_val_if_fail (train->period != 0u, FALSE);

  if (t % train->period == 0u && train->width > 0u)
    {
      if (pulse != NULL)
        *pulse = t / train->period;
      return TRUE;
    }

  time_t e = t - train->width;
  if (e % train->period == 0u && train->width < train->period)
    {
      if (pulse != NULL)
        *pulse = e / train->period + 1u;
      return FALSE;
    }
  time_t a = ceil (e / (double) train->period);
  time_t b = floor (t / (double) train->period);
  if (pulse != NULL)
    *pulse = a;
  return a <= b;
}

static gboolean in_any (time_t t, GSList * trains, time_t * pulse);

static gboolean
in (time_t t, struct PulseTrain *train, time_t * pulse)
{
  g_return_val_if_fail (train != NULL, FALSE);
  g_return_val_if_fail (pulse != NULL, FALSE);

  if (!in_itself (t / train->unit - train->shift, train, pulse))
    return FALSE;
  *pulse = (*pulse * train->period + train->shift) * train->unit;
  return in_any (t - *pulse, train->children, pulse);
}

static gboolean
in_any (time_t t, GSList * trains, time_t * pulse)
{
  time_t minimal = t + 1u;

  for (GSList * train = trains; train != NULL; train = train->next)
    {
      time_t pulse;
      if (!in (t, (struct PulseTrain *) train->data, &pulse))
        continue;
      if (pulse < minimal)
        minimal = pulse;
    }

  if (pulse != NULL && minimal != t + 1u)
    *pulse = minimal;

  return trains == NULL || minimal != t + 1u;
}

static gboolean
prepare (struct GCronSource *source, gint * timeout)
{
  g_return_val_if_fail (timeout != NULL, FALSE);
  g_return_val_if_fail (source != NULL, FALSE);

  *timeout = 1000;

  time_t now = source->override_timefunc == NULL
    ? time (NULL) : source->override_timefunc (NULL,
                                               source->override_timefuncdata);

  time_t maximal = source->last;
  for (GSList * g = source->pulse_trains; g != NULL; g = g->next)
    {
      time_t pulse = maximal;
      if (!in_any (now, (GSList *) g->data, &pulse))
        return FALSE;
      if (pulse > maximal)
        maximal = pulse;
    }
  if (maximal == source->last)
    return FALSE;
  source->last = maximal;
  return TRUE;
}

static gboolean
dispatch (GSource * source, GSourceFunc callback, gpointer user_data)
{
  g_return_val_if_fail (source != NULL, G_SOURCE_REMOVE);
  return callback != NULL ? callback (user_data) : G_SOURCE_CONTINUE;
}

void
pulse_train_for_each (GSList * trains, GFunc func, gpointer user_data)
{
  for (GSList * t = trains; t != NULL; t = t->next)
    {
      struct PulseTrain *train = (struct PulseTrain *) t->data;
      pulse_train_for_each (train->children, func, user_data);

      func (t->data, user_data);
    }
}

static void
add_children_to_hash (struct PulseTrain *train,
                      GHashTable * children_to_dispose)
{
  g_return_if_fail (train != NULL);
  g_return_if_fail (children_to_dispose != NULL);

  g_hash_table_add (children_to_dispose, g_steal_pointer (&train->children));
}

static void
destroy_wrapper (gpointer data, GDestroyNotify destroyer)
{
  destroyer (data);
}

static void
finalize (struct GCronSource *source)
{
  g_return_if_fail (source != NULL);

  GHashTable *children_to_dispose = g_hash_table_new (NULL, NULL);

  for (GSList * g = source->pulse_trains; g != NULL; g = g->next)
    {
      pulse_train_for_each (g->data, (GFunc) add_children_to_hash,
                            children_to_dispose);
      g_hash_table_add (children_to_dispose, g_steal_pointer (&g->data));
    }

  GHashTableIter iter;
  gpointer key, value;

  if (source->pulse_train_destructor != NULL)
    {
      g_hash_table_iter_init (&iter, children_to_dispose);
      while (g_hash_table_iter_next (&iter, &key, &value))
        g_slist_foreach (key, (GFunc) destroy_wrapper,
                         source->pulse_train_destructor);
    }

  g_hash_table_iter_init (&iter, children_to_dispose);
  while (g_hash_table_iter_next (&iter, &key, &value))
    g_slist_free (key);

  g_clear_pointer (&children_to_dispose, g_hash_table_unref);

  g_slist_free (g_steal_pointer (&source->pulse_trains));
}
