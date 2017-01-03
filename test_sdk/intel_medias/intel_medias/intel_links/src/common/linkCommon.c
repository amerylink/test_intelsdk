#include "linkCommon.h"

Int32 linkOpenNslog(void)
{
	nslog_conf_info_t info ;
	Int32 ret = LINK_SOK;
	memset(&info, 0, sizeof(nslog_conf_info_t));
	strcpy(info.conf_name, LINKS_NSLOG_CONF);
	strcpy(info.rules_name, LINKS_NSLOG_CNAME);
	strcpy(info.output_type, LINKS_NSLOG_OUT);
	ret = NslogInit(&info);
	if(ret != 0) {
		return LINK_EFAIL;
	}

	return ret;
}

//获取毫秒级别
Uint32 link_get_run_time(void)
{
	Uint32 msec;
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;
	return msec;
}



