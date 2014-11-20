//////////////////////////////////////////////////////////////////////////
//
// decoder.h
// Implements the MPEG-1 decoder.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <wrl\implements.h>
#include <windows.media.h>

#include <mftransform.h>
#include <mfapi.h>
#include <mferror.h>
#include <assert.h>


extern "C"
{
//ffmepg
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h> 
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
 

const DWORD RV_VIDEO_SEQ_HEADER_MIN_SIZE = 12;       // Minimum length of the video sequence header.
static const REFERENCE_TIME INVALID_TIME = _I64_MAX;    //  Not really invalid but unlikely enough for sample code.



template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

//-------------------------------------------------------------------
// FFmpegVideoDecoder  class
//
// Byte-stream handler for WPlayer streams.
//-------------------------------------------------------------------
#ifndef RUNTIMECLASS_WPlayerVideoCodec_VideoDecoder_DEFINED
#define RUNTIMECLASS_WPlayerVideoCodec_VideoDecoder_DEFINED
extern const __declspec(selectany) WCHAR RuntimeClass_WPlayerVideoDecoder_VideoDecoder[] = L"WPlayerVideoCodec.VideoDecoder";
#endif


//-------------------------------------------------------------------
// ffmpeg video decoder
//
//
// The decoder outputs YUV I420
//
// Note: This MFT is derived from a sample that used to ship in the
// DirectX SDK.
//-------------------------------------------------------------------

class VideoDecoder
	: public Microsoft::WRL::RuntimeClass<
	Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >,
	ABI::Windows::Media::IMediaExtension,
	IMFTransform>
{
	InspectableClass(RuntimeClass_WPlayerVideoDecoder_VideoDecoder, BaseTrust)

public:
	VideoDecoder();
	~VideoDecoder();

	// IMediaExtension
	IFACEMETHOD(SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration);

	// IMFTransform methods
	STDMETHODIMP GetStreamLimits(
		DWORD   *pdwInputMinimum,
		DWORD   *pdwInputMaximum,
		DWORD   *pdwOutputMinimum,
		DWORD   *pdwOutputMaximum
		);

	STDMETHODIMP GetStreamCount(
		DWORD   *pcInputStreams,
		DWORD   *pcOutputStreams
		);

	STDMETHODIMP GetStreamIDs(
		DWORD   dwInputIDArraySize,
		DWORD   *pdwInputIDs,
		DWORD   dwOutputIDArraySize,
		DWORD   *pdwOutputIDs
		);

	STDMETHODIMP GetInputStreamInfo(
		DWORD                     dwInputStreamID,
		MFT_INPUT_STREAM_INFO *   pStreamInfo
		);

	STDMETHODIMP GetOutputStreamInfo(
		DWORD                     dwOutputStreamID,
		MFT_OUTPUT_STREAM_INFO *  pStreamInfo
		);

	STDMETHODIMP GetAttributes(IMFAttributes** pAttributes);

	STDMETHODIMP GetInputStreamAttributes(
		DWORD           dwInputStreamID,
		IMFAttributes   **ppAttributes
		);

	STDMETHODIMP GetOutputStreamAttributes(
		DWORD           dwOutputStreamID,
		IMFAttributes   **ppAttributes
		);

	STDMETHODIMP DeleteInputStream(DWORD dwStreamID);

	STDMETHODIMP AddInputStreams(
		DWORD   cStreams,
		DWORD   *adwStreamIDs
		);

	STDMETHODIMP GetInputAvailableType(
		DWORD           dwInputStreamID,
		DWORD           dwTypeIndex, // 0-based
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetOutputAvailableType(
		DWORD           dwOutputStreamID,
		DWORD           dwTypeIndex, // 0-based
		IMFMediaType    **ppType
		);

	STDMETHODIMP SetInputType(
		DWORD           dwInputStreamID,
		IMFMediaType    *pType,
		DWORD           dwFlags
		);

	STDMETHODIMP SetOutputType(
		DWORD           dwOutputStreamID,
		IMFMediaType    *pType,
		DWORD           dwFlags
		);

	STDMETHODIMP GetInputCurrentType(
		DWORD           dwInputStreamID,
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetOutputCurrentType(
		DWORD           dwOutputStreamID,
		IMFMediaType    **ppType
		);

	STDMETHODIMP GetInputStatus(
		DWORD           dwInputStreamID,
		DWORD           *pdwFlags
		);

	STDMETHODIMP GetOutputStatus(DWORD *pdwFlags);

	STDMETHODIMP SetOutputBounds(
		LONGLONG        hnsLowerBound,
		LONGLONG        hnsUpperBound
		);

	STDMETHODIMP ProcessEvent(
		DWORD              dwInputStreamID,
		IMFMediaEvent      *pEvent
		);

	STDMETHODIMP ProcessMessage(
		MFT_MESSAGE_TYPE    eMessage,
		ULONG_PTR           ulParam
		);

	STDMETHODIMP ProcessInput(
		DWORD               dwInputStreamID,
		IMFSample           *pSample,
		DWORD               dwFlags
		);

	STDMETHODIMP ProcessOutput(
		DWORD                   dwFlags,
		DWORD                   cOutputBufferCount,
		MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
		DWORD                   *pdwStatus
		);

protected:

	// HasPendingOutput: Returns TRUE if the MFT is holding an input sample.
	BOOL HasPendingOutput() const { return m_pBuffer != NULL; }

	// IsValidInputStream: Returns TRUE if dwInputStreamID is a valid input stream identifier.
	BOOL IsValidInputStream(DWORD dwInputStreamID) const
	{
		return dwInputStreamID == 0;
	}

	// IsValidOutputStream: Returns TRUE if dwOutputStreamID is a valid output stream identifier.
	BOOL IsValidOutputStream(DWORD dwOutputStreamID) const
	{
		return dwOutputStreamID == 0;
	}

	//  Internal processing routine
	HRESULT InternalProcessOutput(IMFSample *pSample, IMFMediaBuffer *pOutputBuffer);
	HRESULT Process();
	HRESULT OnCheckInputType(IMFMediaType *pmt);
	HRESULT OnCheckOutputType(IMFMediaType *pmt);
	HRESULT OnSetInputType(IMFMediaType *pmt);
	HRESULT OnSetOutputType(IMFMediaType *pmt);
	HRESULT OnDiscontinuity();
	HRESULT AllocateStreamingResources();
	HRESULT FreeStreamingResources();
	HRESULT OnFlush();


protected:

	CRITICAL_SECTION        m_critSec;

	//  Streaming locals
	IMFMediaType            *m_pInputType;              // Input media type.
	IMFMediaType            *m_pOutputType;             // Output media type.

	IMFMediaBuffer          *m_pBuffer;
	BYTE *                  m_pbData;
	DWORD                   m_cbData;

	// Fomat information
	UINT32                      m_imageWidthInPixels;
	UINT32                      m_imageHeightInPixels;
	MFRatio                     m_frameRate;
	DWORD                       m_cbImageSize;              // Image size, in bytes.

	//  Fabricate timestamps based on the average time per from if there isn't one in the stream
	REFERENCE_TIME          m_rtFrame;

	//ffmpeg
	AVCodec *codec;
	AVCodecContext *ctx = NULL;
	AVDictionary *opts = NULL;
	AVFrame * picture;
	int frame, got_picture;
	SwsContext *sws_ctx = NULL;
	UINT32 currentMediaType;

};

