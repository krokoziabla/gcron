#include <glib.h>
#include <math.h>
#include <time.h>

#include <private.h>

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

static gboolean in_any (time_t t, GSList * trains, time_t * pulse);

static gboolean
in (time_t t, struct PulseTrain *train, time_t * pulse)
{
  g_return_val_if_fail (train != NULL, FALSE);
  g_return_val_if_fail (pulse != NULL, FALSE);

  time_t U = train->unit;
  time_t W = train->width;
  time_t P = train->period;
  time_t S = P - train->shift % P;

  time_t B = t / U + S;
  time_t E = B + P - W % P;

  time_t N = B / P;

  if (W == 0u || (W < P && B % P != 0u && (E % P == 0u || E / P != B / P)))
    return FALSE;

  if (!in_any (t + S * U - N * P * U, train->children, pulse))
    return FALSE;

  *pulse += (N * P + train->shift % P - P) * U;
  return TRUE;
}

static gboolean
in_any (time_t t, GSList * trains, time_t * pulse)
{
  g_return_val_if_fail (pulse != NULL, FALSE);

  time_t minimal = t + 1u;

  for (GSList * train = trains; train != NULL; train = train->next)
    {
      time_t pulse;
      if (!in (t, (struct PulseTrain *) train->data, &pulse))
        continue;
      if (pulse < minimal)
        minimal = pulse;
    }

  if (minimal != t + 1u)
    *pulse = minimal;
  else if (trains == NULL)
    *pulse = 0u;

  return trains == NULL || minimal != t + 1u;
}

static gboolean
prepare (struct GCronSource *source, gint * timeout)
{
  g_return_val_if_fail (timeout != NULL, FALSE);
  g_return_val_if_fail (source != NULL, FALSE);

  *timeout = 1000;

  time_t now = time (NULL);
  if (source->override_timefunc != NULL)
    now = source->override_timefunc (NULL, source->override_timefuncdata);

  time_t maximal = source->last;
  for (GSList * g = source->pulse_trains; g != NULL; g = g->next)
    {
      time_t pulse;
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

static void
collect_children (struct PulseTrain *train, GHashTable * children_to_dispose)
{
  g_return_if_fail (train != NULL);
  g_return_if_fail (children_to_dispose != NULL);

  g_slist_foreach (train->children, (GFunc) collect_children,
                   children_to_dispose);
  g_hash_table_add (children_to_dispose, g_steal_pointer (&train->children));
}

static void
noop_destroy (gpointer data)
{
  (void) data;
}

static void
finalize (struct GCronSource *source)
{
  g_return_if_fail (source != NULL);

  g_autoptr (GHashTable) children_to_dispose = g_hash_table_new (NULL, NULL);

  for (GSList * g = source->pulse_trains; g != NULL; g = g->next)
    {
      g_slist_foreach (g->data, (GFunc) collect_children,
                       children_to_dispose);
      g_hash_table_add (children_to_dispose, g_steal_pointer (&g->data));
    }

  GHashTableIter iter;
  gpointer key, value;

  GDestroyNotify destructor = source->pulse_train_destructor;
  if (destructor == NULL)
    destructor = noop_destroy;

  g_hash_table_iter_init (&iter, children_to_dispose);
  while (g_hash_table_iter_next (&iter, &key, &value))
    g_slist_free_full (key, destructor);

  g_slist_free (g_steal_pointer (&source->pulse_trains));
}
