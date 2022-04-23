#include <glib.h>

void test_suite_prepare (void);
void test_suite_schedule (void);

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);
  test_suite_prepare ();
  test_suite_schedule ();
  return g_test_run ();
}
