#include <glib.h>

#include "cron_source.h"

struct ScheduleTraits
{
  time_t period;
  time_t width;
};

static void
dispose (GSource * source)
{
  g_return_if_fail (source != NULL);

  struct GCronSource *cron_source = (struct GCronSource *) source;
  for (GSList * g = cron_source->pulse_trains; g != NULL; g = g->next)
    {
      for (GSList * t = (GSList *) g->data; t != NULL; t = t->next)
        {
          g_free (t->data);
        }
    }
}

GSource *
g_cron_source_new (gchar * schedule)
{
  static struct ScheduleTraits schedule_traits[] = {
    {.period = 60u * 60u,.width = 60u}, // minute
    {.period = 60u * 60u * 24u,.width = 60u * 60u},     // hour
    {.period = 60u * 60u * 24u * 7u,.width = 60u * 60u * 24u},  // day of week
  };

  GSList *pulse_train_groups = NULL;

  GError *error = NULL;
  GRegex *re = g_regex_new ("\\d+|\\*", 0, 0, &error);
  GMatchInfo *match_info;

  g_regex_match (re, schedule, 0, &match_info);
  size_t i = 0u;
  while (g_match_info_matches (match_info)
         && i < sizeof schedule_traits / sizeof (struct ScheduleTraits))
    {
      GSList *pulse_trains = NULL;
      GArray *shifts = g_array_new (FALSE, FALSE, sizeof (time_t));
      gchar *word = g_match_info_fetch (match_info, 0);
      if (!g_strcmp0 (word, "*"))
        {
          for (time_t j = 0; j < schedule_traits[i].period;
               j += schedule_traits[i].width)
            {
              g_array_append_val (shifts, j);
            }
        }
      // word -> list of shifts
      for (size_t j = 0; j < shifts->len; ++j)
        {
          struct PulseTrain *train = g_new (struct PulseTrain, 1);
          train->period = schedule_traits[i].period;
          train->width = schedule_traits[i].width;
          train->shift = g_array_index (shifts, time_t, j);
          pulse_trains = g_slist_append (pulse_trains, train);
        }

      pulse_train_groups = g_slist_append (pulse_train_groups, pulse_trains);
      g_free (word);
      g_array_unref (shifts);
      g_match_info_next (match_info, NULL);
      ++i;
    }
  if (g_match_info_matches (match_info)
      || i < sizeof schedule_traits / sizeof (struct ScheduleTraits))
    {
      g_warning ("hello\n");
    }
  g_match_info_free (match_info);
  g_regex_unref (re);

  GSource *source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));
  g_source_set_dispose_function (source, dispose);
  struct GCronSource *cron_source = (struct GCronSource *) source;
  cron_source->pulse_trains = pulse_train_groups;
  return source;
}
