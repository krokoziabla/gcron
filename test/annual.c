#include <stdio.h>

#include "gcron.h"
#include "gcron_private.h"

struct PeriodTestData
{
    time_t first, last;
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
    .period = 366u,
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
test_period (struct PeriodTestData * data)
{
  g_autoptr (GSource) source = g_cron_source_new (data->schedule);
  struct GCronSource *cron_source = (struct GCronSource *) source;

  cron_source->override_timefunc = timefunc;

  gint timeout;

  cron_source->override_timefuncdata = (gpointer) (data->first - 1u);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata = (gpointer) (data->first - 0u);
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));

  cron_source->override_timefuncdata = (gpointer) (data->last - 0u);
  g_assert_false (g_cron_source_funcs.prepare (source, &timeout));
  cron_source->override_timefuncdata = (gpointer) (data->last - 1u);
  g_assert_true (g_cron_source_funcs.prepare (source, &timeout));
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

  static struct PeriodTestData leap_data[] = {
    {.first=63072000u, .last=65750400u},
    {.first=65750400u, .last=68256000u},
    {.first=68256000u, .last=70934400u},
    {.first=70934400u, .last=73526400u},
    {.first=73526400u, .last=76204800u},
    {.first=76204800u, .last=78796800u},
    {.first=78796800u, .last=81475200u},
    {.first=81475200u, .last=84153600u},
    {.first=84153600u, .last=86745600u},
    {.first=86745600u, .last=89424000u},
    {.first=89424000u, .last=92016000u},
    {.first=92016000u, .last=94694400u},
  };
  for (size_t i = 0u; i < sizeof leap_data / sizeof(struct PeriodTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/leap_month%ld", i);
      snprintf (leap_data[i].schedule, sizeof leap_data[i].schedule, "* * * %ld *", i + 1);
      g_test_add_data_func (name, (gconstpointer) (leap_data + i),
                            (GTestDataFunc) test_period);

  }

  static struct PeriodTestData non_leap_data[] = {
    {.first=126230400u, .last=128908800u},
    {.first=128908800u, .last=131328000u},
    {.first=131328000u, .last=134006400u},
    {.first=134006400u, .last=136598400u},
    {.first=136598400u, .last=139276800u},
    {.first=139276800u, .last=141868800u},
    {.first=141868800u, .last=144547200u},
    {.first=144547200u, .last=147225600u},
    {.first=147225600u, .last=149817600u},
    {.first=149817600u, .last=152496000u},
    {.first=152496000u, .last=155088000u},
    {.first=155088000u, .last=157766400u},
  };
  for (size_t i = 0u; i < sizeof non_leap_data / sizeof(struct PeriodTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/non_leap_month%ld", i);
      snprintf (non_leap_data[i].schedule, sizeof non_leap_data[i].schedule, "* * * %ld *", i + 1);
      g_test_add_data_func (name, (gconstpointer) (non_leap_data + i),
                            (GTestDataFunc) test_period);

  }

  static struct PeriodTestData days_data[] = {
    {.first=65750400u, .last=65836800u},
    {.first=65836800u, .last=65923200u},
    {.first=65923200u, .last=66009600u},
    {.first=66009600u, .last=66096000u},
    {.first=66096000u, .last=66182400u},
    {.first=66182400u, .last=66268800u},
    {.first=66268800u, .last=66355200u},
    {.first=66355200u, .last=66441600u},
    {.first=66441600u, .last=66528000u},
    {.first=66528000u, .last=66614400u},
    {.first=66614400u, .last=66700800u},
    {.first=66700800u, .last=66787200u},
    {.first=66787200u, .last=66873600u},
    {.first=66873600u, .last=66960000u},
    {.first=66960000u, .last=67046400u},
    {.first=67046400u, .last=67132800u},
    {.first=67132800u, .last=67219200u},
    {.first=67219200u, .last=67305600u},
    {.first=67305600u, .last=67392000u},
    {.first=67392000u, .last=67478400u},
    {.first=67478400u, .last=67564800u},
    {.first=67564800u, .last=67651200u},
    {.first=67651200u, .last=67737600u},
    {.first=67737600u, .last=67824000u},
    {.first=67824000u, .last=67910400u},
    {.first=67910400u, .last=67996800u},
    {.first=67996800u, .last=68083200u},
    {.first=68083200u, .last=68169600u},
    {.first=68169600u, .last=68256000u},
  };
  for (size_t i = 0u; i < sizeof days_data / sizeof(struct PeriodTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/day%ld", i);
      snprintf (days_data[i].schedule, sizeof days_data[i].schedule, "* * %ld * *", i + 1);
      g_test_add_data_func (name, (gconstpointer) (days_data + i),
                            (GTestDataFunc) test_period);

  }

  static struct PeriodTestData d30_data[] = {
    {.first=65577600u, .last=65664000u},
    {.first=70761600u, .last=70848000u},
    {.first=73440000u, .last=73526400u},
    {.first=76032000u, .last=76118400u},
    {.first=78710400u, .last=78796800u},
    {.first=81302400u, .last=81388800u},
    {.first=83980800u, .last=84067200u},
    {.first=86659200u, .last=86745600u},
    {.first=89251200u, .last=89337600u},
    {.first=91929600u, .last=92016000u},
    {.first=94521600u, .last=94608000u},
  };
  for (size_t i = 0u; i < sizeof d30_data / sizeof(struct PeriodTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/30d%ld", i);
      snprintf (d30_data[i].schedule, sizeof days_data[i].schedule, "* * %d * *", 30);
      g_test_add_data_func (name, (gconstpointer) (d30_data + i),
                            (GTestDataFunc) test_period);
  }

  static struct PeriodTestData d31_data[] = {
    {.first=65664000u, .last=65750400u},
    {.first=70848000u, .last=70934400u},
    {.first=76118400u, .last=76204800u},
    {.first=81388800u, .last=81475200u},
    {.first=84067200u, .last=84153600u},
    {.first=89337600u, .last=89424000u},
    {.first=94608000u, .last=94694400u},
  };
  for (size_t i = 0u; i < sizeof d31_data / sizeof(struct PeriodTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/31d%ld", i);
      snprintf (d31_data[i].schedule, sizeof days_data[i].schedule, "* * %d * *", 31);
      g_test_add_data_func (name, (gconstpointer) (d31_data + i),
                            (GTestDataFunc) test_period);
  }

  static struct PeriodTestData week_data[] = {
    {.first=1682812800u, .last=1682899200u},
    {.first=1682899200u, .last=1682985600u},
    {.first=1682985600u, .last=1683072000u},
    {.first=1683072000u, .last=1683158400u},
    {.first=1683158400u, .last=1683244800u},
    {.first=1683244800u, .last=1683331200u},
    {.first=1683331200u, .last=1683417600u},
    {.first=1683417600u, .last=1683504000u},
  };
  for (size_t i = 0u; i < sizeof week_data / sizeof(struct PeriodTestData); ++i)
  {
      snprintf (name, sizeof name, "/annual/week%ld", i);
      snprintf (week_data[i].schedule, sizeof week_data[i].schedule, "* * * * %ld", i);
      g_test_add_data_func (name, (gconstpointer) (week_data + i),
                            (GTestDataFunc) test_period);

  }
}
