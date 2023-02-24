#include <stdio.h>
#include <time.h>

#include "gcron_private.h"

struct SingleTestData
{
  struct PulseTrain train;
  time_t now;
  gboolean expected_result;
};

struct LastTestData
{
  time_t last;
  time_t now;
  gboolean expected_result;
};

static time_t
timefunc (time_t * tloc, gpointer timefuncdata)
{
  if (tloc != NULL)
    *tloc = (time_t) timefuncdata;
  return (time_t) timefuncdata;
}

static void
test_single (struct SingleTestData *data)
{
  g_autoptr (GSource) source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));

  struct GCronSource *cron_source = (struct GCronSource *) source;

  cron_source->last = 0u;
  cron_source->pulse_trains =
    g_slist_prepend (NULL, g_slist_append (NULL, &data->train));
  cron_source->override_timefunc = timefunc;
  cron_source->override_timefuncdata = (gpointer) data->now;

  gint timeout;

  g_assert_false (data->expected_result ^ g_cron_source_funcs.prepare
                  (source, &timeout));
}

static void
test_last (const struct LastTestData *data)
{
  struct PulseTrain train = {
    .period = 4u,
    .width = 3u,
    .shift = 2u,
    .unit = 1u,
    .children = NULL,
  };

  g_autoptr (GSource) source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));

  struct GCronSource *cron_source = (struct GCronSource *) source;

  cron_source->last = data->last;
  cron_source->pulse_trains =
    g_slist_prepend (NULL, g_slist_append (NULL, &train));
  cron_source->override_timefunc = timefunc;
  cron_source->override_timefuncdata = (gpointer) data->now;

  gint timeout;

  g_assert_false (data->expected_result ^ g_cron_source_funcs.prepare
                  (source, &timeout));
}

void
test_suite_prepare (void)
{
  static struct SingleTestData single_data[] = {
    {
     .train = {.period = 1u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 1u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 1u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 2u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 0u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 1u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 2u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 5u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 3u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 0u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 5u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 1u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 6u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 2u,.unit = 1u,.children =
               NULL},
     .now = 4u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 0u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 1u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 2u,
     .expected_result = FALSE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 3u,
     .expected_result = TRUE,
     },
    {
     .train = {.period = 3u,.width = 4u,.shift = 3u,.unit = 1u,.children =
               NULL},
     .now = 7u,
     .expected_result = TRUE,
     },
  };

  char name[20];
  for (size_t i = 0u; i < sizeof single_data / sizeof (struct SingleTestData);
       ++i)
    {
      snprintf (name, sizeof name, "/prepare/single%ld", i);
      g_test_add_data_func (name, (gconstpointer) (single_data + i),
                            (GTestDataFunc) test_single);
    }

  static struct LastTestData last_data[] = {
    {.last = 2u,.now = 0u,.expected_result = FALSE},
    {.last = 2u,.now = 1u,.expected_result = FALSE},
    {.last = 2u,.now = 2u,.expected_result = FALSE},
    {.last = 2u,.now = 3u,.expected_result = FALSE},
    {.last = 2u,.now = 4u,.expected_result = FALSE},
    {.last = 2u,.now = 5u,.expected_result = FALSE},
    {.last = 2u,.now = 6u,.expected_result = TRUE},
  };

  for (size_t i = 0u; i < sizeof last_data / sizeof (struct LastTestData);
       ++i)
    {
      snprintf (name, sizeof name, "/prepare/last%ld", i);
      g_test_add_data_func (name, (gconstpointer) (last_data + i),
                            (GTestDataFunc) test_last);
    }
}
