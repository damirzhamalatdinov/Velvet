//SNTP
#include "time_synch.h"

#define ASTANA "<+06>-6"


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */

//char Current_Date_Time[100];

static const char *TAG = "SNTP";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void Get_current_date_time(char *date_time){
	char strftime_buf[64];
	time_t now;
	
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	
	//TimeStampUnix = tm.tv_sec * 1000LL + tm.tv_usec / 1000LL;
	setenv("TZ", ASTANA, 1);
	tzset();
	localtime_r(&now, &timeinfo);

	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current date/time in Astana is: %s", strftime_buf);
	strcpy(date_time, strftime_buf);
	TimeStamp = now;
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
	
}

static void obtain_time(void)
{
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

 void Set_SystemTime_SNTP()  {
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	// Is time set? If not, tm_year will be (1970 - 1900).
	if (timeinfo.tm_year < (2016 - 1900)) {
		ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
		obtain_time();
		// update 'now' variable with current time
		time(&now);
	}
}

void datetime_now(void *params){
	Set_SystemTime_SNTP();
	while(1){
		Get_current_date_time(Current_Date_Time);
		vTaskDelay(3000/portTICK_PERIOD_MS);
	}
}
