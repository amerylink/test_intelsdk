#ifdef INTEL_MEDIAS_EXPORTS
#define INTEL_MEDIAS_API __declspec(dllexport)
#else
#define INTEL_MEDIAS_API __declspec(dllimport)
#endif

#include "comm_def.h"

extern "C"
{
	//-----------------------bitstream----------------------
	INTEL_MEDIAS_API handle_t intelmedia_bitstream_open();
	INTEL_MEDIAS_API bool intelmedia_bitstream_init(handle_t& bitstream_handle, unsigned int size, unsigned int count=10);
	INTEL_MEDIAS_API void intelmedia_bitstream_close(handle_t& bitstream_handle);
	INTEL_MEDIAS_API bool intelmedia_bitstream_input(handle_t& bitstream_handle, BYTE* data, unsigned int size);

	//-----------------------format---------------------------
	//INTEL_MEDIAS_API bool intelmedia_format_decoder_params(sInputParams_Dec& Param);
	//INTEL_MEDIAS_API bool intelmedia_format_encoder_params(sInputParams_Enc& Param);

	//-----------------------dec----------------------------
	INTEL_MEDIAS_API handle_t intelmedia_decoder_open();
	INTEL_MEDIAS_API void intelmedia_decoder_close(handle_t& decoder_handle);
	INTEL_MEDIAS_API bool intelmedia_decoder_init(handle_t decoder_handle, sInputParams& InputParams);
	INTEL_MEDIAS_API void intelmedia_set_decoder_tex2d_callback(handle_t decoder_handle, intelmedia_decoder_tex2d_callback _cb, void* reserve);
	INTEL_MEDIAS_API void intelmedia_decode_connect(handle_t& decoder_handle, handle_t& handle, SourceType type = SourceType_Bitstream);

	//-----------------------enc----------------------------
	//INTEL_MEDIAS_API handle_t intelmedia_encoder_open();
	//INTEL_MEDIAS_API void intelmedia_encoder_close(handle_t& encoder_handle);
	//INTEL_MEDIAS_API bool intelmedia_encoder_init(handle_t encoder_handle, sInputParams_Enc *pParams);
	//INTEL_MEDIAS_API void intelmedia_encoder_connect(handle_t& encoder_handle, handle_t& handle, SourceType type = SourceType_Bitstream);

};
