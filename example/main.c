#include <stdlib.h>
#include <time.h>

#include <glib.h>

#include "gcron.h"

static gboolean
callback (gpointer user_data)
{
  time_t now = time (NULL);
  char message[26];
  strftime (message, sizeof message, "%F %T %z", localtime (&now));
  g_print ("%s\n", message);

  return G_SOURCE_CONTINUE;
}

int
main (int argc, char *argv[])
{
  g_autoptr (GMainContext) context = g_main_context_new ();
  g_autoptr (GSource) source = g_cron_source_new (" * * * * 0-4");
  g_source_set_callback (source, callback, NULL, NULL);

  g_source_attach (source, context);
  g_autoptr (GMainLoop) loop = g_main_loop_new (context, TRUE);

  g_main_loop_run (loop);

  return EXIT_SUCCESS;
}
