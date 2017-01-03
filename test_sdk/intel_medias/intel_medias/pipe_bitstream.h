#pragma once
#include "comm_def.h"
#include <mfxcommon.h>

class CBitstreamPipe
{
public:
	CBitstreamPipe();
	virtual ~CBitstreamPipe();

	bool Init(unsigned int size, unsigned int count);
	bool PutData(BYTE* data, unsigned int size);

	sStrQueEx       m_listOut;
private:
	mfxBitstream* Get();
	void Put(mfxBitstream* value);
	void Release();

	sStrQueExt      m_listIn;
	vector<mfxBitstream*> m_vecBitstreams;
	unsigned int    m_nMaxsize;

};

