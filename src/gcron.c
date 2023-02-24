#include <glib.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "gcron.h"
#include "gcron_private.h"


static bool
try_extend (struct PulseTrain *train, guint shift, guint width)
{
  if (train->shift + train->width != shift)
    return FALSE;

  train->width += width;
  return TRUE;
}

static GSList *
flat_functor (GSList * trains, guint * shift, struct PulseTrain *model)
{
  if (trains == NULL || !try_extend ((struct PulseTrain *) trains->data,
                                     model->shift + *shift, model->width))
    {
      trains = g_slist_prepend (trains, g_memdup2 (model, sizeof *model));
      ((struct PulseTrain *) trains->data)->shift += *shift;
    }
  return trains;
}

static GSList *
annual_functor (GSList * trains, guint * shift, struct PulseTrain *model)
{
  (void) model;

  static struct PulseTrain months[] = {
    {},                         // dummy
    {.period = 365u,.width = 31u,.shift = 0u,.children = NULL}, // January
    {.period = 365u,.width = 28u,.shift = 31u,.children = NULL},        // Febuary
    {.period = 365u,.width = 31u,.shift = 59u,.children = NULL},        // March
    {.period = 365u,.width = 30u,.shift = 90u,.children = NULL},        // April
    {.period = 365u,.width = 31u,.shift = 120u,.children = NULL},       // May
    {.period = 365u,.width = 30u,.shift = 151u,.children = NULL},       // June
    {.period = 365u,.width = 31u,.shift = 181u,.children = NULL},       // July
    {.period = 365u,.width = 31u,.shift = 212u,.children = NULL},       // August
    {.period = 365u,.width = 30u,.shift = 243u,.children = NULL},       // September
    {.period = 365u,.width = 31u,.shift = 273u,.children = NULL},       // October
    {.period = 365u,.width = 30u,.shift = 304u,.children = NULL},       // November
    {.period = 365u,.width = 31u,.shift = 334u,.children = NULL},       // December
  };

  guint s = *shift % (sizeof months / sizeof (struct PulseTrain));
  if (trains == NULL || !try_extend ((struct PulseTrain *) trains->data,
                                     months[s].shift, months[s].width))
    trains =
      g_slist_prepend (trains, g_memdup2 (months + s, sizeof months[s]));
  return trains;
}

typedef GSList *(*ForEachFunc) (GSList *, guint *, struct PulseTrain *);

static GSList *for_each (gchar * items, ForEachFunc func,
                         struct PulseTrain *model, guint asterisk_min,
                         guint asterisk_max);

static GSList *
make_pulse_trains (gchar * schedule)
{
  g_return_val_if_fail (schedule != NULL, NULL);

  static struct PulseTrain base_schedules[] = {
    {.period = 60u,.width = 1u,.shift = 0u,.unit = 60u,.children = NULL},       // minute
    {.period = 24u,.width = 1u,.shift = 0u,.unit = 60u * 60u,.children = NULL}, // hour
    {.period = 28u,.width = 1u,.shift = -1,.unit = 60u * 60u * 24u,.children = NULL},   // day of month
    {.period = 1u,.width = 1u,.shift = 0u,.unit = 60u,.children = NULL},        // grid
    {.period = 7u,.width = 1u,.shift = 3u,.unit = 60u * 60u * 24u,.children = NULL},    // day of week
  };

  static guint asterisk_min[] = { 0u, 0u, 1u, 1u, 0u };
  static guint asterisk_max[] = { 59u, 23u, 31u, 12u, 7u };

  struct PulseTrain *grid =
    g_memdup2 (base_schedules + 3, sizeof (struct PulseTrain));
  GSList *pulse_train_groups =
    g_slist_prepend (NULL, g_slist_prepend (NULL, grid));
  GSList *days_train_group = NULL;

  g_auto (GStrv) words = g_regex_split_simple ("\\s+", schedule, 0, 0);
  guint empty = 0u;
  for (GStrv word = words; *word != NULL; ++word)
    if (!strcmp (*word, ""))
      ++empty;
    else if (strcmp (*word, "*"))
      {
        ptrdiff_t i = (word - words - empty) % 5;
        GSList *trains = for_each (*word,
                                   i == 3 ? annual_functor : flat_functor,
                                   base_schedules + i,
                                   asterisk_min[i],
                                   asterisk_max[i]);
        if (trains == NULL)
          continue;

        switch (i)
          {
          case 2:
            days_train_group =
              g_slist_concat (days_train_group,
                              make_from_monthly_pulses (trains));
            break;
          case 3:
            pulse_train_groups =
              g_slist_prepend (pulse_train_groups, make (trains));
            break;
          case 4:
            days_train_group = g_slist_concat (days_train_group, trains);
            break;
          default:
            pulse_train_groups = g_slist_prepend (pulse_train_groups, trains);
          }
      }

  if (days_train_group != NULL)
    pulse_train_groups =
      g_slist_prepend (pulse_train_groups, days_train_group);

  return pulse_train_groups;
}

static GSList *
for_each (gchar * items, ForEachFunc func, struct PulseTrain *model,
          guint asterisk_min, guint asterisk_max)
{
  g_return_val_if_fail (func != NULL, NULL);

  GSList *accu = NULL;
  g_autoptr (GRegex) re = g_regex_new
    ("^(\\*|(\\d+)(?:-(\\d+))?|(\\d+)?~(\\d+)?)(?:/([1-9]\\d*))?$", 0, 0,
     NULL);

  g_auto (GStrv) intervals = g_strsplit (items, ",", 0);
  for (GStrv interval = intervals; *interval != NULL; ++interval)
    {
      g_autoptr (GMatchInfo) match_info = NULL;
      if (g_regex_match (re, *interval, 0, &match_info))
        {
          guint first = asterisk_min;
          guint last = asterisk_max;
          guint step = 1u;

          g_autofree gchar *base = g_match_info_fetch (match_info, 1);
          g_autofree gchar *intmin = g_match_info_fetch (match_info, 2);
          g_autofree gchar *intmax = g_match_info_fetch (match_info, 3);
          g_autofree gchar *randmin = g_match_info_fetch (match_info, 4);
          g_autofree gchar *randmax = g_match_info_fetch (match_info, 5);
          g_autofree gchar *intstep = g_match_info_fetch (match_info, 6);

          if (strcmp (intmin, ""))
            {
              first = atoi (intmin);
              last = first;
            }
          else if (strcmp (base, "*"))
            {
              if (strcmp (randmin, ""))
                first = atoi (randmin);
              if (strcmp (randmax, ""))
                last = atoi (randmax);
              first = g_random_int_range (first, last + 1);
              last = first;
            }

          if (strcmp (intmax, ""))
            last = atoi (intmax);
          if (strcmp (intstep, ""))
            step = atoi (intstep);

          for (guint i = first; i <= last; i += step)
            accu = func (accu, &i, model);
        }
    }

  return accu;
}

static void
g_cron_source_init (struct GCronSource *source, gchar * schedule)
{
  source->pulse_trains = make_pulse_trains (schedule);
  source->pulse_train_destructor = g_free;
}

GSource *
g_cron_source_new (gchar * schedule)
{
  GSource *source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));
  g_cron_source_init ((struct GCronSource *) source, schedule);
  return source;
}

guint
g_cron_add_full (gint priority, gchar * schedule, GSourceFunc function,
                 gpointer data, GDestroyNotify notify)
{
  g_return_val_if_fail (function != NULL, 0);

  g_autoptr (GSource) source = g_cron_source_new (schedule);

  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority (source, priority);

  g_source_set_callback (source, function, data, notify);
  guint id = g_source_attach (source, NULL);

  return id;
}

guint
g_cron_add (gchar * schedule, GSourceFunc function, gpointer data)
{
  return g_cron_add_full (G_PRIORITY_DEFAULT, schedule, function, data, NULL);
}
