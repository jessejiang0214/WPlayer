#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpeg_file_io.h"

#ifndef _USE_LEGACY_WIN32_API
BOOL GetFileSizeEx_WinRT(HANDLE hFile,PLARGE_INTEGER lpFileSize);
#define GetFileSizeEx GetFileSizeEx_WinRT
#endif

#pragma region
int ff_read_data_HFILE(HANDLE hFile,PBYTE p,int n)
{
	DWORD r = 0;
	ReadFile(hFile,p,n,&r,NULL);
	return r;
}

int ff_write_data_HFILE(HANDLE hFile,PBYTE p,int n)
{
	DWORD r = 0;
	WriteFile(hFile,p,n,&r,NULL);
	return r;
}

LONG64 ff_seek_pointer_HFILE(HANDLE hFile,LONG64 offset,int whence)
{
	LARGE_INTEGER li,liPos;
	li.QuadPart = offset;
	liPos.QuadPart = 0;
	BOOL b = SetFilePointerEx(hFile,li,&liPos,whence);
	return b ? liPos.QuadPart:-1;
}
#pragma endregion

#pragma region
int ff_read_data_IStream(IStream* pStream,PBYTE p,int n)
{
	ULONG r = 0;
	pStream->Read(p,n,&r);
	return r;
}

int ff_write_data_IStream(IStream* pStream,PBYTE p,int n)
{
	ULONG r = 0;
	pStream->Write(p,n,&r);
	return r;
}

LONG64 ff_seek_pointer_IStream(IStream* pStream,LONG64 offset,int whence)
{
	LARGE_INTEGER li,liPos;
	li.QuadPart = offset;
	liPos.QuadPart = 0;
	HRESULT hr = pStream->Seek(li,whence,(PULARGE_INTEGER)&liPos);
	return SUCCEEDED(hr) ? liPos.QuadPart:-1;
}
#pragma endregion

#pragma region
int ff_read_data_MFStream(IMFByteStream* pMFStream,PBYTE p,int n)
{
	ULONG r = 0;
	pMFStream->Read(p,n,&r);
	return r;
}

int ff_write_data_MFStream(IMFByteStream* pMFStream,PBYTE p,int n)
{
	ULONG r = 0;
	pMFStream->Write(p,n,&r);
	return r;
}

LONG64 ff_seek_pointer_MFStream(IMFByteStream* pMFStream,LONG64 offset,int whence)
{
	HRESULT hr = E_UNEXPECTED;
	QWORD qwPos = 0;
	switch (whence)
	{
	case SEEK_SET:
		hr = pMFStream->Seek(msoBegin,offset,0,&qwPos);
		break;
	case SEEK_CUR:
		hr = pMFStream->Seek(msoCurrent,offset,0,&qwPos);
		break;
	case SEEK_END:
		pMFStream->GetLength(&qwPos);
		hr = pMFStream->Seek(msoBegin,qwPos + offset,0,&qwPos);
		break;
	}
	return SUCCEEDED(hr) ? qwPos:-1;
}
#pragma endregion

int ff_read_data(void* opaque,byte* p,int n)
{
	register FF_IO_CONTEXT* io = (FF_IO_CONTEXT*)opaque;
	if (io == NULL)
		return 0;
	if (io->type == FF_IO_DATA_TYPE_HFILE)
		return ff_read_data_HFILE(io->hFile,p,n);
	else if (io->type == FF_IO_DATA_TYPE_ISTREAM)
		return ff_read_data_IStream(io->pStream,p,n);
	else if (io->type == FF_IO_DATA_TYPE_MFBYTESTREAM)
		return ff_read_data_MFStream(io->pMFStream,p,n);
	else
		return 0;
}

int ff_write_data(void* opaque,byte* p,int n)
{
	register FF_IO_CONTEXT* io = (FF_IO_CONTEXT*)opaque;
	if (io == NULL)
		return 0;
	if (io->type == FF_IO_DATA_TYPE_HFILE)
		return ff_write_data_HFILE(io->hFile,p,n);
	else if (io->type == FF_IO_DATA_TYPE_ISTREAM)
		return ff_write_data_IStream(io->pStream,p,n);
	else if (io->type == FF_IO_DATA_TYPE_MFBYTESTREAM)
		return ff_write_data_MFStream(io->pMFStream,p,n);
	else
		return 0;
}

long long ff_seek_pointer(void* opaque,long long offset,int whence)
{
	register FF_IO_CONTEXT* io = (FF_IO_CONTEXT*)opaque;
	if (io == NULL)
		return -1;
	if (io->type == FF_IO_DATA_TYPE_HFILE)
		return ff_seek_pointer_HFILE(io->hFile,offset,whence);
	else if (io->type == FF_IO_DATA_TYPE_ISTREAM)
		return ff_seek_pointer_IStream(io->pStream,offset,whence);
	else if (io->type == FF_IO_DATA_TYPE_MFBYTESTREAM)
		return ff_seek_pointer_MFStream(io->pMFStream,offset,whence);
	else
		return -1;
}

long long ff_get_total_size(void* opaque)
{
	register FF_IO_CONTEXT* io = (FF_IO_CONTEXT*)opaque;
	if (io == NULL)
		return 0;
	if (io->type == FF_IO_DATA_TYPE_HFILE)
	{
		LARGE_INTEGER li;
		li.QuadPart = 0;
		GetFileSizeEx(io->hFile,&li);
		return li.QuadPart;
	}else if (io->type == FF_IO_DATA_TYPE_ISTREAM)
	{
		STATSTG stat = {};
		io->pStream->Stat(&stat,0);
		return stat.cbSize.QuadPart;
	}else if (io->type == FF_IO_DATA_TYPE_MFBYTESTREAM)
	{
		QWORD qwLength = 0;
		io->pMFStream->GetLength(&qwLength);
		return qwLength;
	}else
		return 0;
}