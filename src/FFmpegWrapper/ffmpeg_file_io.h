#pragma once

#include "stdafx.h"
#include "ComInterfaceList.h"

enum FF_IO_DATA_TYPE{
	FF_IO_DATA_TYPE_NULL,
	FF_IO_DATA_TYPE_HFILE,
	FF_IO_DATA_TYPE_ISTREAM,
	FF_IO_DATA_TYPE_MFBYTESTREAM,
};

struct FF_IO_CONTEXT{
	FF_IO_DATA_TYPE type;
	union{
		HANDLE hFile;
		IStream* pStream;
		IMFByteStream* pMFStream;
	};
};

int ff_read_data(void* opaque,byte* p,int n);
int ff_write_data(void* opaque,byte* p,int n);
long long ff_seek_pointer(void* opaque,long long offset,int whence);
long long ff_get_total_size(void* opaque);