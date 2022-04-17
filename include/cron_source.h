#pragma once

#include <glib.h>

struct GCronSource
{
  GSource base_source;
};

GSourceFuncs g_cron_source_funcs;
