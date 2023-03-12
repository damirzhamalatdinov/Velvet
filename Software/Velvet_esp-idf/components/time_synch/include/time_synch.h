#ifndef TIME_SYNCH_H_
#define TIME_SYNCH_H_

#include <time.h>
#include <sys/time.h>
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"


void initialize_sntp();
void datetime_now();

extern long long int TimeStamp;
//int64_t TimeStampUnix;

extern char Current_Date_Time[100];

#endif