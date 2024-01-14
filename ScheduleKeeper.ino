#include "Globals.h"

typedef struct ScheduleEntry
{
  ScheduleEntryType_t   entryType;
  time_t                time;
  LightMode_t           lightMode;
} ScheduleEntry_t;
