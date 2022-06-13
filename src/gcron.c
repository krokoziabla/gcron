#include <glib.h>

#include "annual.h"
#include "cron_source.h"

struct ScheduleTraits
{
  time_t period;
  time_t width;
  time_t unit;
};

GSource *
g_cron_source_new (gchar * schedule)
{
  static struct ScheduleTraits schedule_traits[] = {
    {.period = 60u,.width = 1u,.unit = 60u},    // minute
    {.period = 24u,.width = 1u,.unit = 60u * 60u},      // hour
    {.period = 31u,.width = 1u,.unit = 60u * 60u * 24u},        // day of month
    {},                         // dummy
    {.period = 7u,.width = 1u,.unit = 60u * 60u * 24u}, // day of week
  };
  static struct AnnualPulse months[] = {
    {.width = 31u,.leap_shift = 0u,.non_leap_shift = 0u},       // January
    {.width = 28u,.leap_shift = 31u,.non_leap_shift = 31u},     // Febuary
    {.width = 31u,.leap_shift = 60u,.non_leap_shift = 59u},     // March
    {.width = 30u,.leap_shift = 91u,.non_leap_shift = 90u},     // April
    {.width = 31u,.leap_shift = 121u,.non_leap_shift = 120u},   // May
    {.width = 30u,.leap_shift = 152u,.non_leap_shift = 151u},   // June
    {.width = 31u,.leap_shift = 182u,.non_leap_shift = 181u},   // July
    {.width = 31u,.leap_shift = 213u,.non_leap_shift = 212u},   // August
    {.width = 30u,.leap_shift = 244u,.non_leap_shift = 243u},   // September
    {.width = 31u,.leap_shift = 274u,.non_leap_shift = 273u},   // October
    {.width = 30u,.leap_shift = 305u,.non_leap_shift = 304u},   // November
    {.width = 31u,.leap_shift = 335u,.non_leap_shift = 334u},   // December
  };
  static struct LeapPulse febuary29th = {
    .width = 1u,.shift = 59u
  };

  GSList *pulse_train_groups = NULL, *days_train_group = NULL;


  GError *error = NULL;
  GRegex *re = g_regex_new ("\\d+|\\*", 0, 0, &error);
  GMatchInfo *match_info;

  g_regex_match (re, schedule, 0, &match_info);
  size_t i = 0u;
  while (g_match_info_matches (match_info)
         && i < sizeof schedule_traits / sizeof (struct ScheduleTraits) + 1u)
    {
      gchar *word = g_match_info_fetch (match_info, 0);
      GArray *shifts = g_array_new (FALSE, FALSE, sizeof (time_t));

      if (!g_strcmp0 (word, "*") && i != 3)
        {
          for (time_t j = 0; j < schedule_traits[i].period;
               j += schedule_traits[i].width)
            {
              g_array_append_val (shifts, j);
            }
        }

      switch (i)
        {
        case 2:
          {
            GSList *pulses = NULL;
            for (size_t j = 0; j < shifts->len; ++j)
              {
                struct MonthlyPulse *pulse = g_new0 (struct MonthlyPulse, 1);
                pulse->width = schedule_traits[i].width;
                pulse->shift = g_array_index (shifts, time_t, j);
                pulses = g_slist_prepend (pulses, pulse);
              }
            days_train_group =
              g_slist_concat (days_train_group,
                              make_from_monthly_pulses (pulses));
            g_slist_free_full (g_steal_pointer (&pulses), g_free);
            break;
          }
        case 3:
          {
            GSList *pulses = NULL;
            for (size_t j = 0;
                 j < sizeof months / sizeof (struct AnnualPulse); ++j)
              pulses = g_slist_prepend (pulses, months + j);

            if (!g_strcmp0 (word, "*"))
              {
                pulse_train_groups =
                  g_slist_prepend (pulse_train_groups,
                                   g_slist_concat (make_from_annual_pulses
                                                   (pulses),
                                                   make_from_leap_pulse
                                                   (&febuary29th)));

              }
            g_slist_free (g_steal_pointer (&pulses));
            break;
          }
        default:
          {
            GSList *pulse_trains = NULL;
            // word -> list of shifts
            for (size_t j = 0; j < shifts->len; ++j)
              {
                struct PulseTrain *train = g_new0 (struct PulseTrain, 1);
                train->period = schedule_traits[i].period;
                train->width = schedule_traits[i].width;
                train->shift = g_array_index (shifts, time_t, j);
                train->unit = schedule_traits[i].unit;
                pulse_trains = g_slist_prepend (pulse_trains, train);
              }

            if (i == 4)
              days_train_group =
                g_slist_concat (days_train_group, pulse_trains);
            else
              pulse_train_groups =
                g_slist_prepend (pulse_train_groups, pulse_trains);
          }
        }
      g_array_unref (shifts);
      g_free (word);
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
  struct GCronSource *cron_source = (struct GCronSource *) source;
  cron_source->pulse_trains =
    g_slist_prepend (pulse_train_groups, days_train_group);
  cron_source->pulse_train_destructor = g_free;
  return source;
}
