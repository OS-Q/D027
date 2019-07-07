#ifndef RTC_CONFIG_H
#define RTC_CONFIG_H

typedef struct _deviceTime
{
	uint8_t	init;/* 0 û�г�ʼ�� ��1 rtc ��ʼ����2 �������ѳ�ʼ��*/
	uint32_t	set_s;/* ϵͳ����ʱ�䣬��λs  */
	uint32_t	local_s;/*����ʱϵͳ�ϵ絽��ǰ������ʱ�䣬��λs  */
} deviceTime;

int rtc_config_init(void);
int get_local_time(uint32_t *time);
int set_local_time(uint8_t *date);
int get_rtc_time(unsigned int *rtc);
void get_rtc_to_local_time(void);
int set_rtc_time_second(uint32_t rtc);
void rtc_init_error(void);

#endif

