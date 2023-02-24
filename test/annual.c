#include <stdio.h>

#include "gcron_private.h"

static time_t
timefunc (time_t * tloc, gpointer timefuncdata)
{
  if (tloc != NULL)
    *tloc = (time_t) timefuncdata;
  return (time_t) timefuncdata;
}

static void
test_single (time_t * data)
{
  g_autoptr (GSource) source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));

  struct GCronSource *cron_source = (struct GCronSource *) source;
  static struct PulseTrain train = {
    .period = 365u,
    .width = 1u,
    .shift = 0u,
    .unit = 60u * 60u * 24u,
    .children = NULL
  };
  cron_source->last = 0u;
  cron_source->pulse_trains =
    g_slist_prepend (NULL, make (g_slist_append (NULL, &train)));
  cron_source->override_timefunc = timefunc;

  gint timeout;

  cron_source->override_timefuncdata =
    (gpointer) (*data - train.width * train.unit);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata = (gpointer) * data;
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata =
    (gpointer) (*data + train.width * train.unit);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
}

void
test_suite_annual (void)
{
  static time_t single_data[] = {
    63072000u,                  // 1972-01-01
    126230400u,                 // 1974-01-01
    946684800u,                 // 2000-01-01
    1009843200u,                // 2002-01-01
    1577836800u,                // 2020-01-01
    1640995200u,                // 2022-01-01
    4102444800u,                // 2100-01-01
    4165516800u,                // 2102-01-01
  };

  char name[20];
  for (size_t i = 0u; i < sizeof single_data / sizeof (time_t); ++i)
    {
      snprintf (name, sizeof name, "/annual/single%ld", i);
      g_test_add_data_func (name, (gconstpointer) (single_data + i),
                            (GTestDataFunc) test_single);
    }

}
