#pragma once

#include <glib.h>

GSList *make (GSList * non_leap_trains);
GSList *make_from_monthly_pulses (GSList * pulses);
