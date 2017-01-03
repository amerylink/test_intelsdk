#ifndef _LINK_VIDEO_H_
#define _LINK_VIDEO_H_

extern "C" 
{
#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"
}

#include "mfxstructures.h"

/**
 * @def    VIDBITSTREAM_MAX_BITSTREAM_BUFS
 * @brief  Maximum number of bitstream buf in a Bitstream_BufList @sa Bitstream_BufList
 */
#define VIDBITSTREAM_MAX_BITSTREAM_BUFS  (20)

#define MAX_FRAME_BUFS  (20)

/**
    \brief Bit stream buffer
*/
typedef struct Bitstream_Buf {

	mfxBitstream Bitstream;
	/**< Buffer Pointer */
	Uint32 channelNum;
	/**< Channel number */
	Uint32 codingType;
	/**< Coding type */
	void *appData;
	/**< Additional application parameter per buffer */

	Uint32 timeStamp;
	/**< Original Capture time stamp */

	Uint32 encodeTimeStamp;
	/**< Encode complete time stamp */

	Uint32 isKeyFrame;
	/**< Flag indicating whether is currentFrame is key frame */
	Uint32 frameWidth;
	/**< Width of the encoded frame */
	Uint32 frameHeight;
	/**< Height of the encoded frame */
} Bitstream_Buf;

/**
 *  \brief Bit stream Buffer List used to exchange multiple Bitstream Buffers
 *         between links
 */
typedef struct
{
	Bitstream_Buf       *bufs[VIDBITSTREAM_MAX_BITSTREAM_BUFS];
	/**< Array of Bitstream_Buf pointers that are to given or received from the
	     codec. */

	Uint32              numBufs;
	/**< Number of frames that are given or received from the codec
	   i.e number of valid pointers in the array containing Bitstream_Buf
	   pointers. */

	Uint32 NeedGetBits;

	void                *appData;
	/**< Additional application parameter per buffer list */

} Bitstream_BufList;

/**
 *  \brief link frame buffer structure.
 *
 */
typedef struct
{
	mfxFrameSurface1 *pSurfaces;

	Uint32 ColorFormat;
	/*For valid values see #FRAME_ColorFormat. */

	Uint32              fid;
	/**< Indicates whether this frame belong to top or bottom field.
	     For valid values see #FRAME_Fid. */
	Uint32              channelNum;
	/**< Channel number to which this frame belongs to. */
	Uint32 selectOrgChannelNum[7]; /* original channeNum at input of select Link */

	Uint32              timeStamp;
	/**< Frame Time stamp, in units of msecs. */
	Uint32              ValidFrame; /*是否是有用的帧*/
	Uint8                 *appData;
	/**< Additional application parameter per frame. This is not modified by
	     driver. */
	Int32 		dupCount;
} FRAME_Buf;

typedef struct
{
	FRAME_Buf       *bufs[MAX_FRAME_BUFS];
	/**< Array of Frame_Buf pointers that are to given or received from the
	     codec. */

	Uint32              numBufs;
	/**< Number of frames that are given or received from the codec
	   i.e number of valid pointers in the array containing Frame_Buf
	   pointers. */

	void                *appData;
	/**< Additional application parameter per buffer list */

} FRAME_BufList;

/**
**	编码信息
*/
typedef struct __enc_info
{
	FRAME_Buf* 			pEncSurfaces;			/*编码输入buf*/
	Bitstream_Buf*	  	pEncBitStrameBuff;/*编码输出buff*/	
	Bitstream_BufList     	EncBitStrameBuffList;/*编码码流输出buff列表*/
	FRAME_BufList		  	EncFrameBuffList;/*编码帧输出buff列表*/
}video_info_t;

#endif

