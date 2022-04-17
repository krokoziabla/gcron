#include <glib.h>

static gboolean
dispatch (GSource * source, GSourceFunc callback, gpointer user_data);

GSourceFuncs g_cron_source_funcs = {
  .prepare = NULL,
  .check = NULL,
  .dispatch = dispatch,
  .finalize = NULL,
};

static gboolean
dispatch (GSource * source, GSourceFunc callback, gpointer user_data)
{
  g_return_val_if_fail (source != NULL, G_SOURCE_REMOVE);
  return G_SOURCE_CONTINUE;
}
