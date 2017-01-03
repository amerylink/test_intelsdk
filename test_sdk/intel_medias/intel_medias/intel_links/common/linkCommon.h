#ifndef _LINK_COMMON_H_
#define _LINK_COMMON_H_
//#include "stdint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>

//#include <sys/ipc.h>
//#include <sys/msg.h>
//#include <pthread.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <time.h>
//#include <errno.h>
//#include <dirent.h>
//#include <sys/prctl.h>
//#include "nslog.h"
//#include <strings.h>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <signal.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <sys/types.h>
//#include <sys/stat.h>

/* unsigned quantities */
typedef unsigned long long Uint64;      ///< Unsigned 64-bit integer
typedef unsigned int Uint32;            ///< Unsigned 32-bit integer
typedef unsigned short Uint16;          ///< Unsigned 16-bit integer
typedef unsigned char Uint8;            ///< Unsigned  8-bit integer
typedef int Bool;
typedef int Int32;            ///< 32-bit integer
typedef char Int8;            ///< 8-bit integer

/* signed quantities */
typedef long long Int64;               ///< Signed 64-bit integer

typedef unsigned int Uns;              ///< Unsigned int

#define	IPC_MAKEFOURCC(ch0, ch1, ch2, ch3)                      \
		((Uint32)(Uint8)(ch0) | ((Uint32)(Uint8)(ch1) << 8) |   \
		((Uint32)(Uint8)(ch2) << 16) | ((Uint32)(Uint8)(ch3) << 24 ))
#define	IPC_H264_CODEC_TYPE   	IPC_MAKEFOURCC('H','2','6','4')
#define	JPEG_CODEC_TYPE 		IPC_MAKEFOURCC('J','P','E','G')

#define LINK_SOK      0  ///< Status : OK
#define LINK_EFAIL   -1  ///< Status : Generic error
#define LINK_TRUE      1  
#define LINK_FALSE   0  
#define LINK_TIMEOUT_NONE        ((Uint32) 0)  // no timeout
#define LINK_TIMEOUT_FOREVER     ((Uint32)-1)  // wait forever

#define VIDEO_WIDTH 			(1920)					//视频宽度
#define VIDEO_HEIGHT			(1080)					//视频宽度

#define	SWMSLINK_TSK_PRI			 (90)
#define SWMSLINK_TSK_TIMER_PRI    (90)
#define NULLSRCLINK_TSK_TIMER_PRI   (98)
#define NULLSRCLINK_TSK_PRI               (80)
#define ADCAPTURELINK_TSK_PRI               (80)
#define AUDIOENCLINK_TSK_PRI               (80)
#define SCLRLINK_TSK_PRI               (85)
#define DECLINK_TSK_PRI               (91)
#define ENCLINK_TSK_PRI               (90)
#define MERGELINK_TSK_PRI           (80)
#define SELECTLINK_TSK_PRI           (80)
#define DUPLINK_TSK_PRI				(80)
#define	ALGLINK_TSK_PRI			 (90)
#define BITSOUTLINK_TSK_PRI               (91)
#define LINK_DEFAULT_ALLOC_BUFF_NUM (6)
#define SYSTEM_MAX_CH_PER_OUT_QUE       (16)

typedef enum
{
    VSYS_STD_NTSC = 0u,
    /**< 720x480 30FPS interlaced NTSC standard. */
    VSYS_STD_PAL,
    /**< 720x576 30FPS interlaced PAL standard. */

    VSYS_STD_480I,
    /**< 720x480 30FPS interlaced SD standard. */
    VSYS_STD_576I,
    /**< 720x576 30FPS interlaced SD standard. */

    VSYS_STD_CIF,
    /**< Interlaced, 360x120 per field NTSC, 360x144 per field PAL. */
    VSYS_STD_HALF_D1,
    /**< Interlaced, 360x240 per field NTSC, 360x288 per field PAL. */
    VSYS_STD_D1,
    /**< Interlaced, 720x240 per field NTSC, 720x288 per field PAL. */

    VSYS_STD_480P,
    /**< 720x480 60FPS progressive ED standard. */
    VSYS_STD_576P,
    /**< 720x576 60FPS progressive ED standard. */

    VSYS_STD_720P_60,
    /**< 1280x720 60FPS progressive HD standard. */
    VSYS_STD_720P_50,			// ------10
    /**< 1280x720 50FPS progressive HD standard. */

    VSYS_STD_1080I_60,
    /**< 1920x1080 30FPS interlaced HD standard. */
    VSYS_STD_1080I_50,
    /**< 1920x1080 50FPS interlaced HD standard. */

    VSYS_STD_1080P_60,
    /**< 1920x1080 60FPS progressive HD standard. */
    VSYS_STD_1080P_50,
    /**< 1920x1080 50FPS progressive HD standard. */

    VSYS_STD_1080P_24,
    /**< 1920x1080 24FPS progressive HD standard. */
    VSYS_STD_1080P_30,
    /**< 1920x1080 30FPS progressive HD standard. */
	
	VSYS_STD_VGA_60 = 0x100,

    VSYS_STD_VGA_72,
    /**< 640x480 72FPS VESA standard. */
    VSYS_STD_VGA_75,
    /**< 640x480 75FPS VESA standard. */
    VSYS_STD_VGA_85,			
    /**< 640x480 85FPS VESA standard. */
	VSYS_STD_WVGA_60,
	/**< 800x480 60PFS WVGA standard. */
    VSYS_STD_SVGA_60,
    /**< 800x600 60FPS VESA standard. */
    VSYS_STD_SVGA_72,
    /**< 800x600 72FPS VESA standard. */
    VSYS_STD_SVGA_75,
    /**< 800x600 75FPS VESA standard. */
    VSYS_STD_SVGA_85,
    /**< 800x600 85FPS VESA standard. */

	VSYS_STD_WSVGA_70,
	/**< 1024x600 70FPS standard. */

    VSYS_STD_XGA_60,
    /**< 1024x768 60FPS VESA standard. */
    VSYS_STD_XGA_70,
    /**< 1024x768 72FPS VESA standard. */
    VSYS_STD_XGA_75,
    /**< 1024x768 75FPS VESA standard. */
    VSYS_STD_XGA_85,
    /**< 1024x768 85FPS VESA standard. */

    VSYS_STD_WXGA_60,
    /**< 1280x768 60FPS VESA standard. */
    VSYS_STD_WXGA_75,			
    /**< 1280x768 75FPS VESA standard. */
    VSYS_STD_WXGA_85,
    /**< 1280x768 85FPS VESA standard. */

    VSYS_STD_SXGA_60,
    /**< 1280x1024 60FPS VESA standard. */
    VSYS_STD_SXGA_75,
    /**< 1280x1024 75FPS VESA standard. */
    VSYS_STD_SXGA_85,
    /**< 1280x1024 85FPS VESA standard. */

	VSYS_STD_WSXGAP_60,
	/**< 1680x1050 60 PFS VESA>*/

    VSYS_STD_SXGAP_60,
    /**< 1400x1050 60FPS VESA standard. */
    VSYS_STD_SXGAP_75,
    /**< 1400x1050 75FPS VESA standard. */

    VSYS_STD_UXGA_60,
    /**< 1600x1200 60FPS VESA standard. */

    VSYS_STD_1280x800_60,
    /**< 1280x800 60 PFS VESA>*/

	VSYS_STD_1368_768_60,
	/**< 1368x768 60 PFS VESA standard. */
	VSYS_STD_1366_768_60,
	/**< 1366x768 60 PFS VESA standard. */
	VSYS_STD_1360_768_60,
	/**< 1360x768 60 PFS VESA standard. */

	VSYS_STD_1440_900_60,
	/**< 1440x900 60 PFS VESA>*/

    VSYS_STD_1920x1200_60,
    /**< 1900x1200 60FPS VESA standard. */
	
    VSYS_STD_2560x1440_60,
    /**< 2560x1440 60FPS VESA standard. */

	VSYS_STD_3840x1200_60,
    /**< 3840x1200 60FPS VESA standard. */

	VSYS_STD_3840x2400_60,
    /**< 3840x2400 60FPS VESA standard. */

	VSYS_STD_1920x2160_30,
    /**< 1920x2160 30FPS VESA standard. */
	VSYS_STD_1920x2400_30,
    /**< 1920x2400 30FPS VESA standard. */
	VSYS_STD_3840x1080_30,
    /**< 3840x1080 30FPS VESA standard. */
	VSYS_STD_MUX_2CH_D1 = 0x200,
    /**< Interlaced, 2Ch D1, NTSC or PAL. */
    VSYS_STD_MUX_2CH_HALF_D1,
    /**< Interlaced, 2ch half D1, NTSC or PAL. */
    VSYS_STD_MUX_2CH_CIF,
    /**< Interlaced, 2ch CIF, NTSC or PAL. */
    VSYS_STD_MUX_4CH_D1,
    /**< Interlaced, 4Ch D1, NTSC or PAL. */
    VSYS_STD_MUX_4CH_CIF,
    /**< Interlaced, 4Ch CIF, NTSC or PAL. */
    VSYS_STD_MUX_4CH_HALF_D1,
    /**< Interlaced, 4Ch Half-D1, NTSC or PAL. */
    VSYS_STD_MUX_8CH_CIF,
    /**< Interlaced, 8Ch CIF, NTSC or PAL. */
    VSYS_STD_MUX_8CH_HALF_D1,
    /**< Interlaced, 8Ch Half-D1, NTSC or PAL. */

	VSYS_STD_AUTO_DETECT = 0x300,
    /**< Auto-detect standard. Used in capture mode. */

    VSYS_STD_REACH_LAST,

	VSYS_STD_CUSTOM,
    /**< Custom standard used when connecting to external LCD etc...
         The video timing is provided by the application.
         Used in display mode. */
 
    VSYS_STD_MAX
    /**< Should be the last value of this enumeration.
         Will be used by driver for validating the input parameters. */
} VSYS_VIDEO_STANDARD_E;

typedef struct _audio_buf_ {
	void *addr;
	Uint32 buf_size;
	Uint32 fill_length;
	Uint32 channel_num;
	Uint32 coding_type;

	Uint32 time_stamp;

	Uint32 sample_rate;
	Uint32 bit_rate;
	Uint32 channel;
	Uint32 phy_addr;
	Uint32 seq_id;

	Uint32 LVolume;
	Uint32 RVolume;

	Uint32 reserved[2];
} audio_buf;

Int32 linkOpenNslog(void);
Uint32 link_get_run_time(void);
#ifdef WIN32
void	link_set_run_time();
#endif
#endif
