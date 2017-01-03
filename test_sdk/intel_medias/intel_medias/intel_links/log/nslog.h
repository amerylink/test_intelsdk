/*
NodeServer-log module
*/

#ifndef __NSLOG_H__
#define __NSLOG_H__
#include "zlog.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define NONE				"\033[m"
#define RED				"\033[0;32;31m"
#define LIGHT_RED		"\033[1;31m"
#define GREEN			"\033[0;32;32m"
#define LIGHT_GREEN		"\033[1;32m"
#define BLUE				"\033[0;32;34m"
#define LIGHT_BLUE		"\033[1;34m"
#define DARY_GRAY		"\033[1;30m"
#define CYAN				"\033[0;36m"
#define LIGHT_CYAN		"\033[1;36m"
#define PURPLE			"\033[0;35m"
#define LIGHT_PURPLE		"\033[1;35m"
#define BROWN			"\033[0;33m"
#define YELLOW			"\033[1;33m"
#define LIGHT_GRAY		"\033[0;37m"
#define WHITE			"\033[1;37m"


#define NS_DEBUG 		ZLOG_LEVEL_DEBUG
#define NS_INFO  		ZLOG_LEVEL_INFO
#define NS_NOTICE	ZLOG_LEVEL_NOTICE
#define NS_WARN		ZLOG_LEVEL_WARN
#define NS_ERROR 	ZLOG_LEVEL_ERROR
#define NS_FATAL 		ZLOG_LEVEL_FATAL

#define NS_OPEN 1
#define NS_CLOSE 0

/*HD: Upgrade Zlog config*/
#define HD_UPGRADE_NSLOG_CONF		("/usr/local/reach/zlog_conf/HDupgrade_log.conf")
#define HD_UPGRADE_NSLOG_CNAME	("HDupgrade")
#define HD_UPGRADE_NSLOG_OUT		("HDupgrade_send")

/*HD:Live log config*/
#define LIVE_ROOM_NSLOG_CONF		("/usr/local/reach/zlog_conf/live_log.conf")
#define LIVE_ROOM_NSLOG_CNAME		("live")
#define LIVE_ROOM_NSLOG_OUT		    ("\"/var/log/recserver/live.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/live.%d(%u).#2r.log\"")
//#define LIVE_ROOM_NSLOG_OUT		("live_send")

/*HD:Enc_App log config*/
#define HD_APP_NSLOG_CONF		("/usr/local/reach/zlog_conf/HDapp_log.conf")
#define HD_APP_NSLOG_CNAME		("HDapp")
#define  HD_APP_NSLOG_OUT		("\"/var/log/recserver/HDapp_log.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/HDapp_log.%d(%u).#2r.log\"")

#define SUN_HD_APP_NSLOG_CONF		("/usr/local/reach/zlog_conf/sun_HDapp_log.conf")
#define SUN_HD_APP_NSLOG_CNAME		("sun_HDapp")
#define SUN_HD_APP_NSLOG_OUT		("\"/var/log/recserver/sun_HDapp_log.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/sun_HDapp_log.%d(%u).#2r.log\"")

#ifdef HAVE_RTSP_MODULE

/*HD:standard stream log config*/
#define STANDARD_STREAM_NSLOG_CONF		("/usr/local/reach/zlog_conf/stream_log.conf")
#define STANDARD_STREAM_NSLOG_CNAME		("stream")
#define STANDARD_STREAM_NSLOG_OUT		("stream_send")
#endif

/*SD:Record log config*/
#define RECORD_NSLOG_CONF		("/usr/local/reach/zlog_conf/record.conf")
#define RECORD_NSLOG_CNAME		("record")
#define RECORD_NSLOG_OUT		("\"/var/log/recserver/record.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/record.%d(%u).#2r.log\"")

/*SD:Enc_App log config*/
#define EDUKIT_NSLOG_CONF		("/usr/local/reach/zlog_conf/edukit_app.conf")
#define EDUKIT_NSLOG_CNAME		("edukit_app")
#define EDUKIT_NSLOG_OUT		("\"/var/log/recserver/edukit.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/edukit.%d(%u).#2r.log\"")

/*SD:upgrade log config*/
#define SD_UPGRADE_NSLOG_CONF		("/usr/local/reach/zlog_conf/upgrade.conf")
#define SD_UPGRADE_NSLOG_CNAME	("sd_upgrade")
#define SD_UPGRADE_NSLOG_OUT		("\"/var/log/recserver/sd_upgrade.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/sd_upgrade.%d(%u).#2r.log\"")

/*SD:Control log config*/
#define CONTROL_NSLOG_CONF		("/usr/local/reach/zlog_conf/control.conf")
#define CONTROL_NSLOG_CNAME	("control")
#define CONTROL_NSLOG_OUT		("\"/var/log/recserver/control.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/control.%d(%u).#2r.log\"")

/*SD:DataBase long config*/
#define DATABASE_NSLOG_CONF		("/usr/local/reach/zlog_conf/database.conf")
#define DATABASE_NSLOG_CNAME		("database")
#define DATABASE_NSLOG_OUT		("\"/var/log/recserver/database.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/database.%d(%u).#2r.log\"")


#define TRACK_NSLOG_CONF		("/usr/local/reach/zlog_conf/track.conf")
#define TRACK_NSLOG_CNAME		("TRACK")

/*codec config*/
#define CODEC_NSLOG_CONF		("/usr/local/reach/zlog_conf/codec_log.conf")
#define CODEC_NSLOG_CNAME		("codec")
#define CODEC_NSLOG_OUT		    ("\"/var/log/recserver/codec.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/codec.%d(%u).#2r.log\"")


/*Links log config*/
#define LINKS_NSLOG_CONF		("/usr/local/reach/zlog_conf/Links.conf")
#define LINKS_NSLOG_CNAME		("Links")
#define LINKS_NSLOG_OUT		("\"/var/log/recserver/Links.%d(%u).log\", 30MB *2 ~ \"/var/log/recserver/Links.%d(%u).#2r.log\"")

#if 0
extern int ctrl_room_id;
#define nslog(nLevel, format, args...) \
	dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	nLevel, "[ROOM_ID:%d]" format, ctrl_room_id, ##args)
#endif
extern zlog_category_t *s_remote_handle;
extern int g_nslog_status;
#define nslog(nLevel, format, args...) \
	do{\
		if(NS_OPEN == g_nslog_status)\
		{ \
			switch(nLevel) {\
				case NS_WARN:\
					if( NULL == s_remote_handle){\
						dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
						nLevel, YELLOW format NONE, ##args); \
					}else{ zlog_warn(s_remote_handle, format,##args);}\
					break;\
				case NS_ERROR:\
				case NS_FATAL:\
					if( NULL == s_remote_handle){\
						dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
						nLevel, RED format NONE, ##args); \
					}else{ zlog_error(s_remote_handle, format,##args);}\
					break;\
				case NS_INFO:\
					if( NULL == s_remote_handle){\
						dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
						nLevel, GREEN format NONE, ##args); \
					}else{ zlog_info(s_remote_handle, format,##args);}\
					break;\
				case NS_DEBUG:\
				default:\
					if( NULL == s_remote_handle){\
						dzlog(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
						nLevel, format, ##args); \
					}else{ zlog_info(s_remote_handle, format,##args);}\
					break;\
			}\
			printf(format,##args);\
		}\
	}while(0)

typedef struct nslog_conf_info_ {
	char conf_name[512];
	char rules_name[32];
	char output_type[512];
	char ip[16];
	unsigned short port;
}  nslog_conf_info_t;

int NslogInit(nslog_conf_info_t *info);
void printf_pthread_create(char *file,char *func);
void printf_pthread_delete(char *file,char *func);

#define	NslogFini		zlog_fini

//add by zm
#ifndef PRINTF
//#define PRINTF(X...)
#define PRINTF(format,args...) nslog(NS_DEBUG,format,##args)

#endif

#ifndef ERR_PRN
//#define ERR_PRN(X...)
#define ERR_PRN(format,args...) nslog(NS_ERROR,format,##args)

#endif

#ifndef WARN_PRN
//#define WARN_PRN(X...)
#define WARN_PRN(format,args...) nslog(NS_WARN,format,##args)

#endif



#ifdef X86
#define nslog(nLevel, format, args...) \
	do{\
		switch(nLevel) {\
			case NS_WARN:\
			case NS_ERROR:\
			case NS_FATAL:\
			case NS_INFO:\
			case NS_DEBUG:\
			default:\
				printf(format, ##args);\
				break;\
		}\
	}while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif //__NSLOG_H__

