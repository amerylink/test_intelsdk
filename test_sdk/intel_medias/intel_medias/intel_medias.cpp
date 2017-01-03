#include "stdafx.h"
#include "intel_medias.h"
#include "pipe_decode.h"
#include "pipe_bitstream.h"
#include "pipe_encode.h"
#include "mfxvp8.h"
#include "mfxstructures.h"


INTEL_MEDIAS_API handle_t intelmedia_bitstream_open()
{
	CBitstreamPipe* bitstream = new CBitstreamPipe();
	return bitstream ? bitstream : NULL;
}

INTEL_MEDIAS_API bool intelmedia_bitstream_init(handle_t& bitstream_handle, unsigned int size, unsigned int count /*= 10*/)
{
	return bitstream_handle ? ((CBitstreamPipe*)bitstream_handle)->Init(size, count) : false;
}

INTEL_MEDIAS_API void intelmedia_bitstream_close(handle_t& bitstream_handle)
{
	if (bitstream_handle)
	{
		delete bitstream_handle;
		bitstream_handle = NULL;
	}
}

INTEL_MEDIAS_API bool intelmedia_bitstream_input(handle_t& bitstream_handle, BYTE* data, unsigned int size)
{
	return bitstream_handle ? ((CBitstreamPipe*)bitstream_handle)->PutData(data, size) : false;
}

//-----------------------format----------------------------
//INTEL_MEDIAS_API bool intelmedia_format_decoder_params(sInputParams_Dec& Param)
//{
//	sInputParams_Dec* param = &Param;
//	switch (param->codecId)//格式支持
//	{
//	case MFX_CODEC_MPEG2:
//	case MFX_CODEC_AVC:
//	case MFX_CODEC_HEVC:
//	case MFX_CODEC_VC1:
//	case CODEC_MVC:
//	case MFX_CODEC_JPEG:
//	case MFX_CODEC_VP8:
//	case MFX_CODEC_CAPTURE:
//		break;
//	default:
//		return false;
//	}
//	if (param->codecId == CODEC_MVC)
//	{
//		param->codecId = MFX_CODEC_AVC;
//		param->MVC_flags = MVC_ENABLED;
//	}
//	if (param->bLowLat || param->bCalLat)
//	{
//		switch (param->codecId)
//		{
//		case MFX_CODEC_HEVC:
//		case MFX_CODEC_AVC:
//		case MFX_CODEC_JPEG:
//		{
//			if (param->MVC_flags&MVC_ENABLED)
//				return false;
//			else
//				break;
//		}
//		default:
//			return false;
//		}
//	}
//	if (!(param->nRotation == 0 || param->nRotation == 90 || param->nRotation == 180 || param->nRotation == 270))
//		return false;
//	if (param->codecId == MFX_CODEC_JPEG)
//		param->chromaType = MFX_JPEG_COLORFORMAT_RGB;
//	if (param->codecId == MFX_CODEC_CAPTURE && (param->screenHeight == 0 || param->screenWidth == 0))
//		return false;
//	if (param->nAsyncDepth == 0)
//		param->nAsyncDepth = 4;
//	return true;
//}
//
//INTEL_MEDIAS_API bool intelmedia_format_encoder_params(sInputParams_Enc& Param)
//{
//	sInputParams_Enc* param = &Param;
//	switch (param->codecId)//格式支持
//	{
//	case MFX_CODEC_MPEG2:
//	case MFX_CODEC_AVC:
//	case MFX_CODEC_HEVC:
//	case CODEC_MVC:
//	case MFX_CODEC_JPEG:
//	case MFX_CODEC_VP8:
//		break;
//	default:
//		return false;
//	}
//	if (param->codecId == CODEC_MVC)
//	{
//		param->codecId = MFX_CODEC_AVC;
//		param->MVC_flags |= MVC_ENABLED;
//	}
//	if (param->nDstWidth == 0 || param->nDstHeight == 0 || param->nWidth==0 || param->nHeight == 0)
//		return false;
//	//if (param->CodecId != MFX_CODEC_JPEG && param->ColorFormat == MFX_FOURCC_YUY2 &&  !pParams->isV4L2InputEnabled)
//	//{
//	//	MSDK_STRING("-yuy2 option is supported only for JPEG encoder");
//	//	return false;
//	//}
//	param->dFrameRate = param->dFrameRate <= 0 ? 30 : param->dFrameRate;
//	param->nBitRate = param->nBitRate == 0 ? CalculateDefaultBitrate(param->codecId,
//		param->nTargetUsage, param->nDstWidth, param->nDstHeight, param->dFrameRate) : param->nBitRate;
//	param->fourcc = param->fourcc == 0 ? MFX_FOURCC_NV12 : param->fourcc;
//	param->nPicStruct = param->nPicStruct == 0 ? MFX_PICSTRUCT_PROGRESSIVE : param->nPicStruct;
//	if (param->nRateControlMethod == MFX_RATECONTROL_LA && !param->bUseHWLib)
//	{
//		msdk_printf(MSDK_STRING("Look ahead BRC is supported only with hardware!"));
//		return false;
//	}
//	if (param->nMaxSliceSize && !param->bUseHWLib)
//	{
//		msdk_printf(MSDK_STRING("MaxSliceSize option is supported only with hardware!"));
//		return false;
//	}
//	if (param->nMaxSliceSize && param->nNumSlice)
//	{
//		msdk_printf(MSDK_STRING("MaxSliceSize and nNumSlice options are not compatible!"));
//		return false;
//	}
//	if (param->nLADepth && (param->nLADepth<10 || param->nLADepth>100))
//	{
//		if (param->nMaxSliceSize == 0)
//			param->nLADepth = 1;
//	}
//	param->nAsyncDepth = param->nAsyncDepth == 0 ? 4 : param->nAsyncDepth;
//	// Ignoring user-defined Async Depth for LA
//	param->nAsyncDepth = param->nMaxSliceSize > 0 ? 1 : param->nAsyncDepth;
//	param->nRateControlMethod = param->nRateControlMethod == 0 ? MFX_RATECONTROL_CBR : param->nRateControlMethod;
//	return true;
//}

//-----------------------dec----------------------------
INTEL_MEDIAS_API handle_t intelmedia_decoder_open()
{
	CDecodingPipe* decoder = new CDecodingPipe();
	return decoder ? decoder : NULL;
}

INTEL_MEDIAS_API void intelmedia_decoder_close(handle_t& decoder_handle)
{
	if (decoder_handle)
	{
		delete decoder_handle;
		decoder_handle = NULL;
	}
}

INTEL_MEDIAS_API bool intelmedia_decoder_init(handle_t decoder_handle, sInputParams& InputParams)
{
	return decoder_handle ? ((CDecodingPipe*)decoder_handle)->Init(InputParams) == 0 : false;
}

INTEL_MEDIAS_API void intelmedia_set_decoder_tex2d_callback(handle_t decoder_handle, intelmedia_decoder_tex2d_callback _cb, void* reserve)
{
	//if (decoder_handle)
	//	((CDecodingPipe*)decoder_handle)->Set_decoder_tex2d_callback(_cb, reserve);
}

INTEL_MEDIAS_API void intelmedia_decode_connect(handle_t& decoder_handle, handle_t& handle, SourceType type /*= SourceType_Bitstream*/)
{
	if (decoder_handle && handle)
	{
		if (type == SourceType_Bitstream)
		{
			sStrQueEx* out = &((CBitstreamPipe*)handle)->m_listOut;
			((CDecodingPipe*)decoder_handle)->Connect(out);
			out->Bind(true);
		}
	}
}

//-----------------------enc----------------------------
//INTEL_MEDIAS_API handle_t intelmedia_encoder_open()
//{
//	CEncodingPipe* encoder = new CEncodingPipe();
//	return encoder ? encoder : NULL;
//}
//
//INTEL_MEDIAS_API void intelmedia_encoder_close(handle_t& encoder_handle)
//{
//	if (encoder_handle)
//	{
//		delete encoder_handle;
//		encoder_handle = NULL;
//	}
//}
//INTEL_MEDIAS_API bool intelmedia_encoder_init(handle_t encoder_handle, sInputParams_Enc *pParams)
//{
//	return encoder_handle ? ((CEncodingPipe*)encoder_handle)->Init(pParams) == 0 : false;
//}
//
//INTEL_MEDIAS_API void intelmedia_encoder_connect(handle_t& encoder_handle, handle_t& handle, SourceType type /*= SourceType_Bitstream*/)
//{
//	if (encoder_handle && handle)
//	{
//		sStrQueEx* out = NULL;
//		if (type == SourceType_Bitstream)
//			out = &((CBitstreamPipe*)handle)->m_listOut;
//		else if (type == SourceType_Surface_Dec)
//			out = &((CDecodingPipe*)handle)->m_listOut;
//		//else if (type == SourceType_Surface_Vpp)
//		//	out = &((CVppPipe*)handle)->m_listOut;
//		if (out)
//		{
//			((CEncodingPipe*)encoder_handle)->Connect(out, type);
//			out->Bind(true);
//		}
//	}
//}

