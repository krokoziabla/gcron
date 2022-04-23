#include <stdlib.h>
#include <time.h>

#include <glib.h>

#include "cron_source.h"

static gboolean
callback (gpointer user_data)
{
  time_t now = time (NULL);
  char message[20];
  strftime (message, sizeof message, "%F %T %z", localtime (&now));
  g_print ("%s\n", message);

  return G_SOURCE_CONTINUE;
}

int
main (int argc, char *argv[])
{
  GMainContext *context = g_main_context_new ();
  GSource *source = g_cron_source_new ("* * *");        // every minute, every hour, every day of week
  g_source_set_callback (source, callback, NULL, NULL);

  g_source_attach (source, context);
  GMainLoop *loop = g_main_loop_new (context, TRUE);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);
  g_source_unref (source);
  g_main_context_unref (context);
  return EXIT_SUCCESS;
}
