#include <glib.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "annual.h"
#include "cron_source.h"


static bool
try_extend (struct PulseTrain *train, guint shift, guint width)
{
  if (train->shift + train->width != shift)
    return FALSE;

  train->width += width;
  return TRUE;
}

static GSList *
flat_functor (GSList * trains, guint * shift, const struct PulseTrain *model)
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
annual_functor (GSList * trains, guint * shift,
                const struct PulseTrain *model)
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

GSource *
g_cron_source_new (gchar * schedule)
{
  static struct PulseTrain base_schedules[] = {
    {.period = 60u,.width = 1u,.shift = 0u,.unit = 60u,.children = NULL},       // minute
    {.period = 24u,.width = 1u,.shift = 0u,.unit = 60u * 60u,.children = NULL}, // hour
    {.period = 28u,.width = 1u,.shift = -1,.unit = 60u * 60u * 24u,.children = NULL},   // day of month
    {.period = 12u},            // dummy
    {.period = 7u,.width = 1u,.shift = 3u,.unit = 60u * 60u * 24u,.children = NULL},    // day of week
  };

  static guint asterisk_min[] = { 0u, 0u, 1u, 1u, 0u };
  static guint asterisk_max[] = { 59u, 23u, 31u, 12u, 7u };

  struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
  train->period = 1u;
  train->width = 1u;
  train->shift = 0u;
  train->unit = 60u;

  GSList *pulse_train_groups =
    g_slist_prepend (NULL, g_slist_prepend (NULL, train));
  GSList *days_train_group = NULL;

  gchar **words = g_regex_split_simple ("\\s+", schedule, 0, 0);
  guint empty = 0u;
  for (gchar ** word = words; *word != NULL; ++word)
    {
      if (!strcmp (*word, ""))
        {
          ++empty;
          continue;
        }
      if (!strcmp (*word, "*"))
        continue;

      ptrdiff_t i = (word - words - empty) % 5;
      GSList *trains = for_each (*word,
                                 i == 3 ? annual_functor : flat_functor,
                                 base_schedules + i,
                                 asterisk_min[i],
                                 asterisk_max[i]);
      switch (i)
        {
        case 2:
          days_train_group = make_from_monthly_pulses (trains);
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
  g_strfreev (g_steal_pointer (&words));

  GSource *source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));
  struct GCronSource *cron_source = (struct GCronSource *) source;
  cron_source->pulse_trains =
    g_slist_prepend (pulse_train_groups, days_train_group);
  cron_source->pulse_train_destructor = g_free;
  return source;
}

GSList *
for_each (gchar * items, ForEachFunc func, const struct PulseTrain *model,
          guint asterisk_min, guint asterisk_max)
{
  g_return_val_if_fail (func != NULL, NULL);

  GSList *accu = NULL;
  GRegex *re = g_regex_new ("^(\\*|(\\d+)(?:(-|~)(\\d+))?)(?:/([1-9]\\d*))?$",
                            0, 0, NULL);

  gchar **intervals = g_strsplit (items, ",", 0);
  for (gchar ** interval = intervals; *interval != NULL; ++interval)
    {
      GMatchInfo *match_info = NULL;
      if (g_regex_match (re, *interval, 0, &match_info))
        {
          guint first = asterisk_min;
          guint last = asterisk_max;
          guint step = 1u;
          if (strcmp (g_match_info_fetch (match_info, 1), "*"))
            {
              first = atoi (g_match_info_fetch (match_info, 2));
              last = first;
            }

          if (strcmp (g_match_info_fetch (match_info, 4), ""))
            last = atoi (g_match_info_fetch (match_info, 4));
          if (strcmp (g_match_info_fetch (match_info, 5), ""))
            step = atoi (g_match_info_fetch (match_info, 5));

          for (guint i = first; i <= last; i += step)
            accu = func (accu, &i, model);
        }
      g_match_info_unref (g_steal_pointer (&match_info));
    }

  g_strfreev (g_steal_pointer (&intervals));
  g_regex_unref (re);
  return accu;
}
