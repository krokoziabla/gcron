#include <stdio.h>

#include "gcron.h"
#include "gcron_private.h"

struct MonthlyTestData
{
    time_t first_day, last_day;
    char schedule[11];
};

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
    g_slist_prepend (NULL, make (g_slist_append (NULL, g_memdup2(&train, sizeof train))));
  cron_source->override_timefunc = timefunc;
  cron_source->pulse_train_destructor = g_free;

  gint timeout;

  cron_source->override_timefuncdata =
    (gpointer) (*data - train.width * train.unit);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata = (gpointer) *data;
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata =
    (gpointer) (*data + train.width * train.unit);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
}

static void
test_month (struct MonthlyTestData * data)
{
  g_autoptr (GSource) source = g_cron_source_new (data->schedule);
  struct GCronSource *cron_source = (struct GCronSource *) source;

  cron_source->override_timefunc = timefunc;

  time_t one_day = 60u * 60u * 24u;
  gint timeout;

  cron_source->override_timefuncdata =
    (gpointer) (data->first_day - one_day);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata = (gpointer) data->first_day;
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata =
    (gpointer) (data->first_day + one_day);
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));

  cron_source->override_timefuncdata =
    (gpointer) (data->last_day - one_day);
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata = (gpointer) data->last_day;
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata =
    (gpointer) (data->last_day + one_day);
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

  char name[25];

  for (size_t i = 0u; i < sizeof single_data / sizeof (time_t); ++i)
    {
      snprintf (name, sizeof name, "/annual/single%ld", i);
      g_test_add_data_func (name, (gconstpointer) (single_data + i),
                            (GTestDataFunc) test_single);
    }

  static struct MonthlyTestData leap_data[] = {
    {.first_day=63072000u, .last_day=65664000u},
    {.first_day=65750400u, .last_day=68169600u},
    {.first_day=68256000u, .last_day=70848000u},
    {.first_day=70934400u, .last_day=73440000u},
    {.first_day=73526400u, .last_day=76118400u},
    {.first_day=76204800u, .last_day=78710400u},
    {.first_day=78796800u, .last_day=81388800u},
    {.first_day=81475200u, .last_day=84067200u},
    {.first_day=84153600u, .last_day=86659200u},
    {.first_day=86745600u, .last_day=89337600u},
    {.first_day=89424000u, .last_day=91929600u},
    {.first_day=92016000u, .last_day=94608000u},
  };
  for (size_t i = 0u; i < sizeof leap_data / sizeof(struct MonthlyTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/leap_month%ld", i);
      snprintf (leap_data[i].schedule, sizeof leap_data[i].schedule, "* * * %ld *", i + 1);
      g_test_add_data_func (name, (gconstpointer) (leap_data + i),
                            (GTestDataFunc) test_month);

  }

  static struct MonthlyTestData non_leap_data[] = {
    {.first_day=126230400u, .last_day=128822400u},
    {.first_day=128908800u, .last_day=131241600u},
    {.first_day=131328000u, .last_day=133920000u},
    {.first_day=134006400u, .last_day=136512000u},
    {.first_day=136598400u, .last_day=139190400u},
    {.first_day=139276800u, .last_day=141782400u},
    {.first_day=141868800u, .last_day=144460800u},
    {.first_day=144547200u, .last_day=147139200u},
    {.first_day=147225600u, .last_day=149731200u},
    {.first_day=149817600u, .last_day=152409600u},
    {.first_day=152496000u, .last_day=155001600u},
    {.first_day=155088000u, .last_day=157680000u},
  };
  for (size_t i = 0u; i < sizeof non_leap_data / sizeof(struct MonthlyTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/non_leap_month%ld", i);
      snprintf (non_leap_data[i].schedule, sizeof non_leap_data[i].schedule, "* * * %ld *", i + 1);
      g_test_add_data_func (name, (gconstpointer) (non_leap_data + i),
                            (GTestDataFunc) test_month);

  }
}
