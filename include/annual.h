#pragma once

struct MonthlyPulse
{
  time_t width;
  time_t shift;
};

struct AnnualPulse
{
  time_t width;
  time_t leap_shift;
  time_t non_leap_shift;
};

struct LeapPulse
{
  time_t width;
  time_t shift;
};

GSList *make_from_pulse (struct AnnualPulse *pulse);
GSList *make_from_annual_pulses (GSList * pulses);
GSList *make_from_monthly_pulses (GSList * pulses);
GSList *make_from_leap_pulse (struct LeapPulse *pulse);
