#include <glib.h>
#include "cron_source.h"

struct Fixture
{
  GMainContext *context;
  GSource *source;
};

void
setup (struct Fixture *fixture, gconstpointer user_data)
{
  fixture->source =
    g_source_new (&g_cron_source_funcs, sizeof (struct GCronSource));
  fixture->context = g_main_context_new ();
}

void
teardown (struct Fixture *fixture, gconstpointer user_data)
{
  g_main_context_unref (fixture->context);
  g_source_unref (fixture->source);
}


static void
test_bla (struct Fixture *fixture, gconstpointer user_data)
{
  g_source_attach (fixture->source, fixture->context);

  g_assert_false (g_main_context_iteration
                  (fixture->context, /* may_block */ TRUE));

  g_source_destroy (fixture->source);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add ("/bla", struct Fixture, NULL, setup, test_bla, teardown);
  return g_test_run ();
}
