#include <string.h>

#include "gcron_private.h"
#include "gcron.h"

static void
test_all_asterisks (void)
{
  GSource *source = g_cron_source_new ("* * * * *");
  g_source_unref (source);
}

static GSList *
functor (GSList * trains, guint * next, struct PulseTrain *model)
{
  g_print ("%d\n", *next);
  return trains;
}

static void
test_for_each (void)
{
  for_each ("11,2-5/2,*/3", (ForEachFunc) functor, NULL, 0u, 30u);
}

void
test_suite_schedule (void)
{
  g_test_add_func ("/schedule/all_asterisks", test_all_asterisks);
  g_test_add_func ("/schedule/for_each", test_for_each);
}
