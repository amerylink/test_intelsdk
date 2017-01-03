/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2005-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#pragma once

#if !defined(_WIN32) && !defined(_WIN64)

#include "logger.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <va/va.h>

#define DCL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/cl.h>
#include <CL/opencl.h>
#include <CL/va_ext.h>

#include "opencl_filter.h"

class OpenCLFilterVA : public OpenCLFilterBase
{
public:
    OpenCLFilterVA();
    virtual ~OpenCLFilterVA();
    virtual cl_int OCLInit(mfxHDL device);

protected: // functions
    virtual cl_int InitDevice();
    virtual cl_int InitSurfaceSharingExtension();

    virtual cl_mem CreateSharedSurface(mfxMemId mid, int nView, bool bIsReadOnly);
    virtual bool EnqueueAcquireSurfaces(cl_mem* surfaces, int nSurfaces);
    virtual bool EnqueueReleaseSurfaces(cl_mem* surfaces, int nSurfaces);

protected: // variables
    VADisplay m_vaDisplay;
    VASurfaceID m_SharedSurfaces[c_shared_surfaces_num];
};

#endif // #if !defined(_WIN32) && !defined(_WIN64)
