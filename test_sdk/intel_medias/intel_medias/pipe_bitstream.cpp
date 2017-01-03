#include "stdafx.h"
#include "pipe_bitstream.h"
#include "sample_utils.h"
#include "sample_defs.h"


CBitstreamPipe::CBitstreamPipe()
{
	m_vecBitstreams.clear();
	m_nMaxsize = 0;
}

CBitstreamPipe::~CBitstreamPipe()
{
	Release();
}

void CBitstreamPipe::Release()
{
	for (int i = 0; i < (int)m_vecBitstreams.size(); i++)
	{
		WipeMfxBitstream(m_vecBitstreams[i]);
		MSDK_SAFE_DELETE(m_vecBitstreams[i]);
	}
	m_vecBitstreams.clear();
}

bool CBitstreamPipe::Init(unsigned int size, unsigned int count)
{
	if (m_vecBitstreams.size()==0 && size>0 && count>0)
	{
		m_nMaxsize = size;
		mfxStatus sts = MFX_ERR_NONE;
		mfxBitstream* bitstream = NULL;
		vector<void*> vecT;
		for (int i = 0; i < (int)count; i++)
		{
			bitstream = new mfxBitstream;
			memset(bitstream, 0, sizeof(mfxBitstream));
			m_vecBitstreams.push_back(bitstream);
			if (!bitstream)
			{
				Release();
				return false;
			}
			bitstream->Data = NULL;
			sts = InitMfxBitstream(bitstream, size);
			if (sts != MFX_ERR_NONE)
			{
				Release();
				return false;
			}
			vecT.push_back(bitstream);
		}
		m_listIn.initList(vecT);
		m_listIn.connect((sStrQueExt*)&m_listOut);
		m_listOut.connect(&m_listIn);
		return true;
	}
	return false;
}

bool CBitstreamPipe::PutData(BYTE* data, unsigned int size)
{
	if (data && size <= m_nMaxsize)
	{
		mfxBitstream* bs = Get();
		if (bs)
		{
			memset(bs->Data, 0, sizeof(mfxU8)*bs->MaxLength);
			memcpy_s(bs->Data, bs->MaxLength, data, size);
			bs->DataLength = size;
			bs->DataOffset = 0;
			Put(bs);
			return true;
		}
	}
	return false;
}

mfxBitstream* CBitstreamPipe::Get()
{
	return (mfxBitstream*)m_listIn.queGet();
}

void CBitstreamPipe::Put(mfxBitstream* value)
{
	m_listIn.quePut(value);
}
