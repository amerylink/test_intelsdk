
#include <pthread.h>

#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <assert.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include "4sdi_socket_port.h"
#include "nslog.h"
#include "RH_socket.h"

int g_nslog_status = NS_CLOSE;

#define NSLOG_STATUS_CONFIG "/usr/local/reach/.config/nslog_status.conf"
static int read_nslog_status()
{
	FILE *fpread = NULL;
	char tmp[10];
	int nslogStatus = 0;
	int ret = 0;

	fpread = fopen(NSLOG_STATUS_CONFIG, "r");

	if(NULL == fpread) {
		printf("[read_nslog_status]fopen  nslog config error!: %s\n", strerror(errno));
		ret = -1;
		goto EXIT_FLAG;
	}

	memset(tmp, 0, 10);

	ret = fread(tmp, 1, 1, fpread);

	if(ret <= 0) {
		ret = -1;
	} else {
		nslogStatus = atoi(tmp);
		ret = nslogStatus;
	}

EXIT_FLAG:

	if(NULL != fpread) {
		fclose(fpread);
	}

	return ret;
}

static int write_nslog_status(int nslogStatus)
{
	FILE *fpwrite = NULL;
	char tmp[10];
	int ret = 0;

	fpwrite = fopen(NSLOG_STATUS_CONFIG, "w");

	if(NULL == fpwrite) {
		printf("[write_nslog_status]fopen  nslog config error!: %s\n", strerror(errno));
		ret = -1;
		goto EXIT_FLAG;
	}

	memset(tmp, 0, 10);

	snprintf(tmp, 10, "%d", nslogStatus);

	fwrite(tmp, 1, 1, fpwrite);
EXIT_FLAG:

	if(NULL != fpwrite) {
		fclose(fpwrite);
	}

	return ret;
}

static int nslog_mk_rules_file(char *rule_file, char *rule_cname, char *out)
{
	if(!rule_file || !rule_cname) {
		printf("error : rule_file = %p, rule_cname = %p\n", rule_file, rule_cname);
		return -1;
	}

	FILE *fp = NULL;

	fp =	fopen(rule_file, "w+");

	if(NULL == fp) {
		printf("fopen  : %s\n", strerror(errno));
		return -1;
	}

	fprintf(fp, \
	        "[global]\n"\
	        "default format = \"%%D.%%ms%%V [%%t:%%p:%%f:%%U:%%L] %%m%%n\"\n"\
	        "[levels]\n"\
	        "INFO = 40, LOG_INFO\n"\
	        "[rules]\n"\
	        "%s.DEBUG %s\n", \
	        rule_cname, out);
	fclose(fp);
	printf("[nslog_mk_rules_file] end!!!\n");
	//system("sync");
	return 0;
}

static int s_remote_sockfd = -1;
zlog_category_t *s_remote_handle = NULL;


static int creatZlogSocket()
{
	int sock;
	sock = RH_Socket(__FILE__, (char *)__func__, AF_INET, SOCK_DGRAM, 0);
	s_remote_sockfd  = sock ;
	return sock;
}

static struct sockaddr_in recvAddr;
static  void setZlogSocketDst(int ip, int port)
{
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(port);
	recvAddr.sin_addr.s_addr = ip;
}

static int output(zlog_msg_t *msg)
{
	if(NS_OPEN == g_nslog_status) {
		printf("%s", msg->buf);
	}

	int alreadSendLen = 0, leth = 0;
	int sock = s_remote_sockfd;

	if(sock < 0) {
		return -1;
	}

	while(alreadSendLen < msg->len) {
		leth = sendto(sock, msg->buf + alreadSendLen, msg->len - alreadSendLen, 0, (struct sockaddr *)&recvAddr, sizeof(struct sockaddr));

		if(leth < 0) {
			return -1;
		}

		alreadSendLen += leth;
	}

	return alreadSendLen;
}

static int creatZlogUDPSend(int ip, nslog_conf_info_t *info)
{
	zlog_set_record(info->output_type, output);
	creatZlogSocket();
	setZlogSocketDst(ip, info->port);
	return 0;
}


int init_remote_zlog(nslog_conf_info_t *remote_info)
{
	int addr;
	int32_t ret = 0;
	zlog_category_t *handle = NULL;

	inet_pton(AF_INET, remote_info->ip, &addr);
	printf("init_remote_zlog:-ip=%s,conf_name:%s\n", remote_info->ip, remote_info->conf_name);

	ret = zlog_init(remote_info->conf_name);

	if(ret) {
		fprintf(stderr, "zlog init failed\n");
	}

	ret = creatZlogUDPSend(addr, remote_info);
	printf("init_remote_zlog: ret=%d----------------\n", ret);


	handle = zlog_get_category(remote_info->rules_name);

	if(handle == NULL) {
		zlog_fini();
		fprintf(stderr, "get g_debug_log fail\n");
		return -1;
	}

	nslog(NS_INFO, "\n\n\n---App %s Start----!!!!!\n\n\n\n", remote_info->conf_name);

	printf("init_remote_zlog:success----------------\n");

	s_remote_handle = handle;
	return 0;
}

int NslogInit(nslog_conf_info_t *info)
{
	int rc = 0;

	/*取nslog 状态，打开或关闭*/
	g_nslog_status = read_nslog_status();

	if(-1 == g_nslog_status) {
		g_nslog_status = NS_CLOSE;
		write_nslog_status(g_nslog_status);
	}

	if(info->port > 0) {
		rc = init_remote_zlog(info);
		return rc;
	} else {
		rc = dzlog_init(info->conf_name, info->rules_name);
	}

	if(rc) {
		printf("dzlog_init failed : <%s><%s>\n", info->conf_name, info->rules_name);
		nslog_mk_rules_file(info->conf_name, info->rules_name, info->output_type);
		rc = dzlog_init(info->conf_name, info->rules_name);

		if(rc) {
			printf("dzlog_init failed !!!<%s><%s>\n", info->conf_name, info->rules_name);
			return -1;
		}

		printf("dzlog_init [%s]  File Not Found! Please restart the application !\n", info->conf_name);

		printf("[global]\n"\
		       "default format = \"%%D.%%ms%%V [%%t:%%p:%%f:%%U:%%L] %%m%%n\"\n"\
		       "[levels]\n"\
		       "INFO = 40, LOG_INFO\n"\
		       "[rules]\n"\
		       "%s.DEBUG %s\n", \
		       info->rules_name, info->output_type);
		return -1;
	}

	return 0;
}

#if 1
static int g_pthread_num = 0 ;
void printf_pthread_create(char *file, char *func)
{
	if(g_pthread_num > 0xffff) {
		g_pthread_num = 0;
	}

	nslog(NS_DEBUG, "pthread %d create in [%s:%s]\n", g_pthread_num++, file, func);
	return ;
}

static int g_pthread_del_num = 0 ;
void printf_pthread_delete(char *file, char *func)
{
	if(g_pthread_del_num > 0xffff) {
		g_pthread_del_num = 0;
	}

	nslog(NS_DEBUG, "pthread %d delete in [%s:%s]\n", g_pthread_del_num++, file, func);
	return ;
}
#endif


