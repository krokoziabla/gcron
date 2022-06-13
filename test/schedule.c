#include "cron_source.h"

static void
test_all_asterisks (void)
{
  GSource *source = g_cron_source_new ("* * * * *");
  g_source_unref (source);
}

void
test_suite_schedule (void)
{
  g_test_add_func ("/schedule/all_asterisks", test_all_asterisks);
}
