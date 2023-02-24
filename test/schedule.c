#include <stdio.h>
#include <string.h>

#include "gcron_private.h"
#include "gcron.h"

static void
test_grid_only (gchar * schedule)
{
  g_autoptr (GSource) source = g_cron_source_new (schedule);

  GSList *trains = ((struct GCronSource *) source)->pulse_trains;

  g_assert_cmpuint (g_slist_length (trains), ==, 1u);

  GSList *grid = trains->data;

  g_assert_cmpuint (g_slist_length (grid), ==, 1u);

  struct PulseTrain *train = grid->data;

  g_assert_cmpuint (train->period, ==, 1u);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, 0u);
  g_assert_cmpuint (train->unit, ==, 60u);
}

struct FlatScheduleTestData
{
  gchar *schedule;
  time_t period;
  time_t shift;
  time_t unit;
  time_t first_rand;
  time_t second_rand;
};

static void
test_flat_schedule (struct FlatScheduleTestData *data)
{
  g_random_set_seed (0u);

  g_autoptr (GSource) source = g_cron_source_new (data->schedule);

  GSList *trains = ((struct GCronSource *) source)->pulse_trains;

  g_assert_cmpuint (g_slist_length (trains), ==, 2u);

  GSList *minutes = trains->data;

  g_assert_cmpuint (g_slist_length (minutes), ==, 10u);

  struct PulseTrain *train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, data->period + data->shift + 1u);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, data->second_rand + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, data->first_rand + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 2u);
  g_assert_cmpuint (train->shift, ==, 2u + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, data->period / 2u + data->shift + 1u);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, 0u + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 2u);
  g_assert_cmpuint (train->shift, ==, 5u + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 1u);
  g_assert_cmpuint (train->shift, ==, 3u + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 2u);
  g_assert_cmpuint (train->shift, ==, 0u + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);

  minutes = minutes->next;
  train = minutes->data;

  g_assert_cmpuint (train->period, ==, data->period);
  g_assert_cmpuint (train->width, ==, 5u);
  g_assert_cmpuint (train->shift, ==, 1u + data->shift);
  g_assert_cmpuint (train->unit, ==, data->unit);
}

void
test_suite_schedule (void)
{
  g_test_add_data_func ("/schedule/grid0", "",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid1", " ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid2", " * ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid3", " * * ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid4", " * *  * ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid5", " * *  * * ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid6", " * *  * *  * ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid7", " * *  * *  * * ",
                        (GTestDataFunc) test_grid_only);
  g_test_add_data_func ("/schedule/grid8", "6-5",
                        (GTestDataFunc) test_grid_only);

  static struct FlatScheduleTestData flat_data[] = {
    {.schedule = "1,2,3-5,0,1-5/2,6,*/31,0~5,~6,7~,~,61",.period = 60u,
     .shift = 0u,.unit = 60u,.first_rand = 35u,.second_rand = 0u},
    {.schedule = "* 1,2,3-5,0,1-5/2,6,*/13,0~5,~6,7~,~,25",.period = 24u,
     .shift = 0u,.unit = 60u * 60u,.first_rand = 19u,.second_rand = 0u},
    {.schedule = "* * * * 1,2,3-5,0,1-5/2,6,*/4,0~5,~6,3~,~,8",.period = 7u,
     .shift = 3u,.unit = 60u * 60u * 24u,.first_rand = 6,.second_rand = 0u},
  };

  char name[28];
  for (size_t i = 0u;
       i < sizeof flat_data / sizeof (struct FlatScheduleTestData); ++i)
    {
      snprintf (name, sizeof name, "/schedule/flat_schedule%ld", i);
      g_test_add_data_func (name, (gconstpointer) (flat_data + i),
                            (GTestDataFunc) test_flat_schedule);
    }

}
