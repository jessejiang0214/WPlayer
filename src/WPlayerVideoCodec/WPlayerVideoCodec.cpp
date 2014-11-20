
#include "StdAfx.h"
#include "WPlayerVideoCodec.h"
#include "dbg_log_output.h"
#include "com_core_def.h"
#ifndef LODWORD
#define LODWORD(x)  ((DWORD)(((DWORD_PTR)(x)) & 0xffffffff))
#endif

const UINT32 MAX_VIDEO_WIDTH = 4095;        // per ISO/IEC 11172-2
const UINT32 MAX_VIDEO_HEIGHT = 4095;



//-------------------------------------------------------------------
// VideoDecoder class
//-------------------------------------------------------------------

VideoDecoder::VideoDecoder() :
m_pInputType(nullptr),
m_pOutputType(nullptr),
m_pBuffer(nullptr),
m_pbData(nullptr),
m_cbData(0),
m_imageWidthInPixels(0),
m_imageHeightInPixels(0),
m_cbImageSize(0),
m_rtFrame(0),
currentMediaType(0),
picture(NULL)
{
	InitializeCriticalSectionEx(&m_critSec, 100, 0);

	m_frameRate.Numerator = m_frameRate.Denominator = 0;

	avcodec_register_all();

}

VideoDecoder::~VideoDecoder()
{
	SafeRelease(&m_pBuffer);
	DeleteCriticalSection(&m_critSec);
}

// IMediaExtension methods

//-------------------------------------------------------------------
// Name: SetProperties
// Sets the configuration of the decoder
//-------------------------------------------------------------------
IFACEMETHODIMP VideoDecoder::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
	return S_OK;
}

// IMFTransform methods. Refer to the Media Foundation SDK documentation for details.

//-------------------------------------------------------------------
// Name: GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetStreamLimits(
	DWORD   *pdwInputMinimum,
	DWORD   *pdwInputMaximum,
	DWORD   *pdwOutputMinimum,
	DWORD   *pdwOutputMaximum
	)
{

	if ((pdwInputMinimum == NULL) ||
		(pdwInputMaximum == NULL) ||
		(pdwOutputMinimum == NULL) ||
		(pdwOutputMaximum == NULL))
	{
		return E_POINTER;
	}


	// This MFT has a fixed number of streams.
	*pdwInputMinimum = 1;
	*pdwInputMaximum = 1;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return S_OK;
}


//-------------------------------------------------------------------
// Name: GetStreamCount
// Returns the actual number of streams.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetStreamCount(
	DWORD   *pcInputStreams,
	DWORD   *pcOutputStreams
	)
{
	if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))

	{
		return E_POINTER;
	}

	// This MFT has a fixed number of streams.
	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return S_OK;
}



//-------------------------------------------------------------------
// Name: GetStreamIDs
// Returns stream IDs for the input and output streams.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetStreamIDs(
	DWORD   /*dwInputIDArraySize*/,
	DWORD   * /*pdwInputIDs*/,
	DWORD   /*dwOutputIDArraySize*/,
	DWORD   * /*pdwOutputIDs*/
	)
{
	// Do not need to implement, because this MFT has a fixed number of
	// streams and the stream IDs match the stream indexes.
	return E_NOTIMPL;
}


//-------------------------------------------------------------------
// Name: GetInputStreamInfo
// Returns information about an input stream.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetInputStreamInfo(
	DWORD                     dwInputStreamID,
	MFT_INPUT_STREAM_INFO *   pStreamInfo
	)
{
	if (pStreamInfo == NULL)
	{
		return E_POINTER;
	}

	if (!IsValidInputStream(dwInputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	pStreamInfo->hnsMaxLatency = 0;

	//  We can process data on any boundary.
	pStreamInfo->dwFlags = 0;

	pStreamInfo->cbSize = 1;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 1;

	return S_OK;
}



//-------------------------------------------------------------------
// Name: GetOutputStreamInfo
// Returns information about an output stream.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetOutputStreamInfo(
	DWORD                     dwOutputStreamID,
	MFT_OUTPUT_STREAM_INFO *  pStreamInfo
	)
{
	if (pStreamInfo == NULL)
	{
		return E_POINTER;
	}

	if (!IsValidOutputStream(dwOutputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	EnterCriticalSection(&m_critSec);

	// NOTE: This method should succeed even when there is no media type on the
	//       stream. If there is no media type, we only need to fill in the dwFlags
	//       member of MFT_OUTPUT_STREAM_INFO. The other members depend on having a
	//       a valid media type.

	pStreamInfo->dwFlags =
		MFT_OUTPUT_STREAM_WHOLE_SAMPLES |
		MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
		MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;

	if (m_pOutputType == NULL)
	{
		pStreamInfo->cbSize = 0;
		pStreamInfo->cbAlignment = 0;
	}
	else
	{
		pStreamInfo->cbSize = m_cbImageSize;
		pStreamInfo->cbAlignment = 1;
	}

	LeaveCriticalSection(&m_critSec);

	return S_OK;
}



//-------------------------------------------------------------------
// Name: GetAttributes
// Returns the attributes for the MFT.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetAttributes(IMFAttributes** /*pAttributes*/)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: GetInputStreamAttributes
// Returns stream-level attributes for an input stream.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetInputStreamAttributes(
	DWORD           /*dwInputStreamID*/,
	IMFAttributes   ** /*ppAttributes*/
	)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: GetOutputStreamAttributes
// Returns stream-level attributes for an output stream.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetOutputStreamAttributes(
	DWORD           /*dwOutputStreamID*/,
	IMFAttributes   ** /*ppAttributes*/
	)
{
	// This MFT does not support any attributes, so the method is not implemented.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: DeleteInputStream
//-------------------------------------------------------------------

HRESULT VideoDecoder::DeleteInputStream(DWORD /*dwStreamID*/)
{
	// This MFT has a fixed number of input streams, so the method is not implemented.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: AddInputStreams
//-------------------------------------------------------------------

HRESULT VideoDecoder::AddInputStreams(
	DWORD   /*cStreams*/,
	DWORD   * /*adwStreamIDs*/
	)
{
	// This MFT has a fixed number of output streams, so the method is not implemented.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: GetInputAvailableType
// Description: Return a preferred input type.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetInputAvailableType(
	DWORD           /*dwInputStreamID*/,
	DWORD           /*dwTypeIndex*/,
	IMFMediaType    ** /*ppType*/
	)
{
	return MF_E_NO_MORE_TYPES;
}



//-------------------------------------------------------------------
// Name: GetOutputAvailableType
// Description: Return a preferred output type.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetOutputAvailableType(
	DWORD           dwOutputStreamID,
	DWORD           dwTypeIndex, // 0-based
	IMFMediaType    **ppType
	)
{
	LogDbgPrint("VideoDecoder::GetOutputAvailableType\r\n");
	if (ppType == NULL)
	{
		return E_INVALIDARG;
	}

	if (!IsValidOutputStream(dwOutputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	if (dwTypeIndex != 0)
	{
		return MF_E_NO_MORE_TYPES;
	}

	EnterCriticalSection(&m_critSec);

	HRESULT hr = S_OK;

	IMFMediaType *pOutputType = NULL;

	if (m_pInputType == NULL)
	{
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	if (SUCCEEDED(hr))
	{
		hr = MFCreateMediaType(&pOutputType);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, m_cbImageSize);
	}

	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_imageWidthInPixels, m_imageHeightInPixels);
	}

	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_frameRate.Numerator, m_frameRate.Denominator);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}

	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}

	if (SUCCEEDED(hr))
	{
		*ppType = pOutputType;
		(*ppType)->AddRef();
	}

	SafeRelease(&pOutputType);
	LeaveCriticalSection(&m_critSec);
	return hr;
}



//-------------------------------------------------------------------
// Name: SetInputType
//-------------------------------------------------------------------

HRESULT VideoDecoder::SetInputType(
	DWORD           dwInputStreamID,
	IMFMediaType    *pType, // Can be NULL to clear the input type.
	DWORD           dwFlags
	)
{
	LogDbgPrint("VideoDecoder::SetInputType\r\n");
	if (!IsValidInputStream(dwInputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	// Validate flags.
	if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	EnterCriticalSection(&m_critSec);

	// Does the caller want us to set the type, or just test it?
	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	// If we have an input sample, the client cannot change the type now.
	if (HasPendingOutput())
	{
		hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	}

	// Validate the type, if non-NULL.
	if (SUCCEEDED(hr))
	{
		if (pType)
		{
			hr = OnCheckInputType(pType);
		}
	}

	if (SUCCEEDED(hr))
	{
		// The type is OK.
		// Set the type, unless the caller was just testing.
		if (bReallySet)
		{
			hr = OnSetInputType(pType);
		}
	}

	LeaveCriticalSection(&m_critSec);
	//TRACEHR_RET(hr);
	return hr;
}



//-------------------------------------------------------------------
// Name: SetOutputType
//-------------------------------------------------------------------

HRESULT VideoDecoder::SetOutputType(
	DWORD           dwOutputStreamID,
	IMFMediaType    *pType, // Can be NULL to clear the output type.
	DWORD           dwFlags
	)
{
	LogDbgPrint("VideoDecoder::SetOutputType\r\n");
	if (!IsValidOutputStream(dwOutputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	// Validate flags.
	if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	EnterCriticalSection(&m_critSec);

	// Does the caller want us to set the type, or just test it?
	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	// If we have an input sample, the client cannot change the type now.
	if (HasPendingOutput())
	{
		LeaveCriticalSection(&m_critSec);
		hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	}

	// Validate the type, if non-NULL.
	if (SUCCEEDED(hr))
	{
		if (pType)
		{
			hr = OnCheckOutputType(pType);
		}
	}

	if (SUCCEEDED(hr))
	{
		if (bReallySet)
		{
			// The type is OK.
			// Set the type, unless the caller was just testing.
			hr = OnSetOutputType(pType);
		}
	}

	LeaveCriticalSection(&m_critSec);
	//TRACEHR_RET(hr);
	return hr;
}



//-------------------------------------------------------------------
// Name: GetInputCurrentType
// Description: Returns the current input type.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetInputCurrentType(
	DWORD           dwInputStreamID,
	IMFMediaType    **ppType
	)
{
	if (ppType == NULL)
	{
		return E_POINTER;
	}

	if (!IsValidInputStream(dwInputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	EnterCriticalSection(&m_critSec);

	HRESULT hr = S_OK;

	if (!m_pInputType)
	{
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	if (SUCCEEDED(hr))
	{
		*ppType = m_pInputType;
		(*ppType)->AddRef();
	}

	LeaveCriticalSection(&m_critSec);
	return hr;
}



//-------------------------------------------------------------------
// Name: GetOutputCurrentType
// Description: Returns the current output type.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetOutputCurrentType(
	DWORD           dwOutputStreamID,
	IMFMediaType    **ppType
	)
{
	if (ppType == NULL)
	{
		return E_POINTER;
	}

	if (!IsValidOutputStream(dwOutputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	EnterCriticalSection(&m_critSec);

	HRESULT hr = S_OK;

	if (!m_pOutputType)
	{
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	if (SUCCEEDED(hr))
	{
		*ppType = m_pOutputType;
		(*ppType)->AddRef();
	}

	LeaveCriticalSection(&m_critSec);
	return hr;
}



//-------------------------------------------------------------------
// Name: GetInputStatus
// Description: Query if the MFT is accepting more input.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetInputStatus(
	DWORD           dwInputStreamID,
	DWORD           *pdwFlags
	)
{
	if (pdwFlags == NULL)
	{
		return E_POINTER;
	}

	if (!IsValidInputStream(dwInputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	EnterCriticalSection(&m_critSec);

	// If we already have an input sample, we don't accept
	// another one until the client calls ProcessOutput or Flush.
	if (HasPendingOutput())
	{
		*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
	}
	else
	{
		*pdwFlags = 0;
	}

	LeaveCriticalSection(&m_critSec);
	return S_OK;
}



//-------------------------------------------------------------------
// Name: GetOutputStatus
// Description: Query if the MFT can produce output.
//-------------------------------------------------------------------

HRESULT VideoDecoder::GetOutputStatus(DWORD *pdwFlags)
{
	if (pdwFlags == NULL)
	{
		return E_POINTER;
	}

	EnterCriticalSection(&m_critSec);

	// We can produce an output sample if (and only if)
	// we have an input sample.
	if (HasPendingOutput())
	{
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	}
	else
	{
		*pdwFlags = 0;
	}

	LeaveCriticalSection(&m_critSec);
	return S_OK;
}



//-------------------------------------------------------------------
// Name: SetOutputBounds
// Sets the range of time stamps that the MFT will output.
//-------------------------------------------------------------------

HRESULT VideoDecoder::SetOutputBounds(
	LONGLONG        /*hnsLowerBound*/,
	LONGLONG        /*hnsUpperBound*/
	)
{
	// Implementation of this method is optional.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: ProcessEvent
// Sends an event to an input stream.
//-------------------------------------------------------------------

HRESULT VideoDecoder::ProcessEvent(
	DWORD              /*dwInputStreamID*/,
	IMFMediaEvent      * /*pEvent */
	)
{
	// This MFT does not handle any stream events, so the method can
	// return E_NOTIMPL. This tells the pipeline that it can stop
	// sending any more events to this MFT.
	return E_NOTIMPL;
}



//-------------------------------------------------------------------
// Name: ProcessMessage
//-------------------------------------------------------------------

HRESULT VideoDecoder::ProcessMessage(
	MFT_MESSAGE_TYPE    eMessage,
	ULONG_PTR           /*ulParam*/
	)
{
	EnterCriticalSection(&m_critSec);

	HRESULT hr = S_OK;

	switch (eMessage)
	{
	case MFT_MESSAGE_COMMAND_FLUSH:
		// Flush the MFT.
		hr = OnFlush();
		break;

	case MFT_MESSAGE_COMMAND_DRAIN:
		// Set the discontinuity flag on all of the input.
		hr = OnDiscontinuity();
		break;

	case MFT_MESSAGE_SET_D3D_MANAGER:
		// The pipeline should never send this message unless the MFT
		// has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
		// do get this message, it's invalid and we don't implement it.
		hr = E_NOTIMPL;
		break;


	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		hr = AllocateStreamingResources();
		break;

	case MFT_MESSAGE_NOTIFY_END_STREAMING:
		hr = FreeStreamingResources();
		break;

		// These messages do not require a response.
	case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
	case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
		break;

	}

	LeaveCriticalSection(&m_critSec);
	return hr;
}



//-------------------------------------------------------------------
// Name: ProcessInput
// Description: Process an input sample.
//-------------------------------------------------------------------

HRESULT VideoDecoder::ProcessInput(
	DWORD               dwInputStreamID,
	IMFSample           *pSample,
	DWORD               dwFlags
	)
{
	//TRACE(0, L"VideoDecoder::ProcessInput\r\n");
	if (pSample == NULL)
	{
		return E_POINTER;
	}

	if (!IsValidInputStream(dwInputStreamID))
	{
		return MF_E_INVALIDSTREAMNUMBER;
	}

	if (dwFlags != 0)
	{
		return E_INVALIDARG; // dwFlags is reserved and must be zero.
	}

	EnterCriticalSection(&m_critSec);

	HRESULT hr = S_OK;
	LONGLONG pts = 0;
	AVPacket avpkt;
	av_init_packet(&avpkt);
	picture = av_frame_alloc();
	got_picture = 0;
	m_cbData = 0;
	if (!m_pInputType || !m_pOutputType)
	{
		hr = MF_E_NOTACCEPTING;   // Client must set input and output types.
	}
	else if (HasPendingOutput())
	{
		hr = MF_E_NOTACCEPTING;   // We already have an input sample.
	}

	// Convert to a single contiguous buffer.
	if (SUCCEEDED(hr))
	{
		// NOTE: This does not cause a copy unless there are multiple buffers
		hr = pSample->ConvertToContiguousBuffer(&m_pBuffer);
	}
	if (SUCCEEDED(hr))
	{
		// NOTE: This does not cause a copy unless there are multiple buffers
		hr = pSample->GetSampleTime(&pts);
		avpkt.pts = pts;
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pBuffer->GetCurrentLength(&m_cbData);
	}

	if (SUCCEEDED(hr) && m_cbData > 0)
	{
		hr = m_pBuffer->Lock(&m_pbData, NULL, &m_cbData);
	}

	if (SUCCEEDED(hr))
	{
		avpkt.size = m_cbData;
		avpkt.data = (uint8_t*)malloc(sizeof(uint8_t)*m_cbData);
		CopyMemory(avpkt.data, m_pbData, m_cbData);
	}

	if (SUCCEEDED(hr))
	{
		m_pBuffer->Unlock();
	}

	if (SUCCEEDED(hr))
	{
		while (avpkt.size > 0)
		{
			int len = avcodec_decode_video2(ctx, picture, &got_picture, &avpkt);
			if (got_picture){
				picture->pts = pts;
				hr = S_OK;
				frame++;
			}
			else
			{
				if (len > 0)
				{
					m_pbData = NULL;
					SafeRelease(&m_pBuffer);
				}
				else
				{
					break;
				}
			}
			*avpkt.data += len;
			avpkt.size -= len;

		}
	}
	free(avpkt.data);
	avpkt.data = NULL;
	LeaveCriticalSection(&m_critSec);
	return hr;
}



//-------------------------------------------------------------------
// Name: ProcessOutput
// Description: Process an output sample.
//-------------------------------------------------------------------

HRESULT VideoDecoder::ProcessOutput(
	DWORD                   dwFlags,
	DWORD                   cOutputBufferCount,
	MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
	DWORD                   *pdwStatus
	)
{
	//TRACE(0, L"VideoDecoder::ProcessOutput\r\n");

	// There are no flags that we accept in this MFT.
	// The only defined flag is MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER. This
	// flag only applies when the MFT marks an output stream as lazy or optional.
	// However there are no lazy or optional streams on this MFT, so the flag is
	// not valid.
	if (dwFlags != 0)
	{
		return E_INVALIDARG;
	}

	if (pOutputSamples == NULL || pdwStatus == NULL)
	{
		return E_POINTER;
	}

	// Must be exactly one output buffer.
	if (cOutputBufferCount != 1)
	{
		return E_INVALIDARG;
	}

	// It must contain a sample.
	if (pOutputSamples[0].pSample == NULL)
	{
		return E_INVALIDARG;
	}

	EnterCriticalSection(&m_critSec);

	HRESULT hr = S_OK;
	DWORD cbData = 0;

	IMFMediaBuffer *pOutput = NULL;

	// If we don't have an input sample, we need some input before
	// we can generate any output.
	if (!HasPendingOutput() || !picture)
	{
		hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
	}

	SafeRelease(&m_pBuffer);
	//m_pBuffer = NULL;
	// Get the output buffer.

	if (SUCCEEDED(hr))
	{
		hr = pOutputSamples[0].pSample->GetBufferByIndex(0, &pOutput);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutput->GetMaxLength(&cbData);
	}

	if (SUCCEEDED(hr))
	{
		if (cbData < m_cbImageSize)
		{
			hr = E_INVALIDARG;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = InternalProcessOutput(pOutputSamples[0].pSample, pOutput);
	}

	if (SUCCEEDED(hr))
	{
		pOutputSamples[0].dwStatus |= MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
	}

	SafeRelease(&pOutput);
	LeaveCriticalSection(&m_critSec);
	return hr;
}

// Private class methods


HRESULT VideoDecoder::InternalProcessOutput(IMFSample *pSample, IMFMediaBuffer *pOutputBuffer)
{
	//VideoBufferLock buffer(pOutputBuffer);

	HRESULT hr = S_OK;

	BYTE *pbData = NULL;
	LONG lDefaultStride = 0;
	LONG lActualStride = 0;

	DWORD cbData = 0;
	LONGLONG rt = 0;

	AVPicture pic;

	if (SUCCEEDED(hr))
	{
		memset(&pic, 0, sizeof(pic));

		avpicture_alloc(&pic, AV_PIX_FMT_NV12, ctx->width, ctx->height);
		hr = pOutputBuffer->SetCurrentLength((DWORD)m_cbImageSize);
	}
	if (SUCCEEDED(hr))
	{
		hr = pOutputBuffer->Lock(&pbData, NULL, &cbData);
	}

	if (SUCCEEDED(hr) && got_picture)
	{


		sws_ctx = sws_getCachedContext(sws_ctx,
			picture->width, picture->height, (AVPixelFormat)picture->format, ctx->width, ctx->height,
			AV_PIX_FMT_NV12, SWS_BICUBIC, NULL, NULL, NULL);
		if (sws_ctx == NULL) {
			fprintf(stderr, "Cannot initialize the conversion context\n");
			return E_INVALIDARG;
		}
		sws_scale(sws_ctx, picture->data, picture->linesize,
			0, ctx->height, pic.data, pic.linesize);

		memcpy(pbData, pic.data[0], m_cbImageSize);
		hr = pOutputBuffer->Unlock();
	}

	//  Set the timestamp
	//  Uncompressed video must always have a timestamp

	if (SUCCEEDED(hr))
	{
		hr = pSample->SetSampleTime(picture->pts);
		LogDbgPrint("VideoDecoder pst is %d\r\n", picture->pts);
	}

	if (SUCCEEDED(hr))
	{
		if (picture->pkt_duration >= 0)
			hr = pSample->SetSampleDuration(picture->pkt_duration);
	}

	if (SUCCEEDED(hr) && picture != NULL)
	{
		avpicture_free(&pic);
		av_frame_free(&picture);
		picture = NULL;
	}

	return hr;
}


HRESULT VideoDecoder::OnCheckInputType(IMFMediaType *pmt)
{
	LogDbgPrint("VideoDecoder::OnCheckInputType\r\n");
	HRESULT hr = S_OK;

	//  Check if the type is already set and if so reject any type that's not identical.
	if (m_pInputType)
	{
		DWORD dwFlags = 0;
		if (S_OK == m_pInputType->IsEqual(pmt, &dwFlags))
		{
			return S_OK;
		}
		else
		{
			return MF_E_INVALIDTYPE;
		}
	}

	GUID majortype = { 0 };
	GUID subtype = { 0 };
	UINT32 width = 0, height = 0;
	UINT32 cbSeqHeader = 0;

	//  We accept MFMediaType_Video, MEDIASUBTYPE_RVVideo

	hr = pmt->GetMajorType(&majortype);

	if (SUCCEEDED(hr))
	{
		if (majortype != MFMediaType_Video)
		{
			hr = MF_E_INVALIDTYPE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype);
	}

	if (SUCCEEDED(hr))
	{
		if (subtype != MFVideoFormat_FFmpeg_SW)		
		{
			hr = MF_E_INVALIDTYPE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = pmt->GetUINT32(MF_MT_MY_FFMPEG_CODECID, &currentMediaType);
		if (!currentMediaType)
		{
			hr = MF_E_INVALIDTYPE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &width, &height);
		if (width != 0 && height != 0)
		{
			m_imageWidthInPixels = width;
			m_imageHeightInPixels = height;
		}
	}

	if (SUCCEEDED(hr))
	{
		if (width > MAX_VIDEO_WIDTH || height > MAX_VIDEO_HEIGHT)
		{
			hr = MF_E_INVALIDTYPE;
		}
	}

	return hr;
}


HRESULT VideoDecoder::OnSetInputType(IMFMediaType *pmt)
{
	LogDbgPrint("VideoDecoder::OnSetInputType\r\n");
	HRESULT hr = S_OK;
	BYTE* pFormatBuffer = NULL;
	UINT32 nBufferSize = 0;
	SafeRelease(&m_pInputType);

	if (SUCCEEDED(hr))
	{
		hr = MFGetAttributeRatio(pmt, MF_MT_FRAME_RATE, (UINT32*)&m_frameRate.Numerator, (UINT32*)&m_frameRate.Denominator);
	}

	if (SUCCEEDED(hr))
	{

		hr = pmt->GetBlobSize(MF_MT_MY_BLOB_DATA, &nBufferSize);
	}

	if (SUCCEEDED(hr))
	{

		pFormatBuffer = (BYTE*)malloc(nBufferSize);
		UINT32 pcbsize = 0;
		hr = pmt->GetBlob(MF_MT_MY_BLOB_DATA, pFormatBuffer, nBufferSize, &pcbsize);
	}

	if (SUCCEEDED(hr))
	{
		m_pInputType = pmt;
		m_pInputType->AddRef();

		codec = avcodec_find_decoder((AVCodecID)currentMediaType);
		if (!codec)
		{
			hr = MF_E_INVALIDTYPE;
		}
	}

	if (SUCCEEDED(hr))
	{
		ctx = avcodec_alloc_context3(codec);
		ctx->thread_count = 4;
		ctx->thread_type = 1;
		ctx->err_recognition = AV_EF_CAREFUL;

		ctx->width = m_imageWidthInPixels;
		ctx->height = m_imageHeightInPixels;
		if (nBufferSize > 0)
		{
			ctx->extradata = pFormatBuffer;
			ctx->extradata_size = nBufferSize;
		}
		/*if (codec->capabilities&CODEC_CAP_TRUNCATED)
			ctx->flags |= CODEC_FLAG_TRUNCATED;*/
		if (avcodec_open2(ctx, codec, NULL) < 0)
			hr = MF_E_INVALIDTYPE;

		m_cbImageSize = av_image_get_buffer_size(AV_PIX_FMT_NV12, ctx->width, ctx->height, 1);
	}

	free(pFormatBuffer);
	pFormatBuffer = NULL;

	return hr;
}

HRESULT VideoDecoder::OnCheckOutputType(IMFMediaType *pmt)
{
	//  Check if the type is already set and if so reject any type that's not identical.
	if (m_pOutputType)
	{
		DWORD dwFlags = 0;
		if (S_OK == m_pOutputType->IsEqual(pmt, &dwFlags))
		{
			return S_OK;
		}
		else
		{
			return MF_E_INVALIDTYPE;
		}
	}

	if (m_pInputType == NULL)
	{
		return MF_E_TRANSFORM_TYPE_NOT_SET; // Input type must be set first.
	}


	HRESULT hr = S_OK;
	BOOL bMatch = FALSE;

	IMFMediaType *pOurType = NULL;

	// Make sure their type is a superset of our proposed output type.
	hr = GetOutputAvailableType(0, 0, &pOurType);

	if (SUCCEEDED(hr))
	{
		hr = pOurType->Compare(pmt, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch);
	}

	if (SUCCEEDED(hr))
	{
		if (!bMatch)
		{
			hr = MF_E_INVALIDTYPE;
		}
	}

	SafeRelease(&pOurType);
	//TRACEHR_RET(hr);
	return hr;
}


HRESULT VideoDecoder::OnSetOutputType(IMFMediaType *pmt)
{
	SafeRelease(&m_pOutputType);

	m_pOutputType = pmt;
	m_pOutputType->AddRef();

	return S_OK;
}


HRESULT VideoDecoder::AllocateStreamingResources()
{
	//  Reinitialize variables
	OnDiscontinuity();

	return S_OK;
}

HRESULT VideoDecoder::FreeStreamingResources()
{
	sws_freeContext(sws_ctx);
	avcodec_close(ctx);
	av_free(ctx);
	return S_OK;
}

HRESULT VideoDecoder::OnDiscontinuity()
{
	//  Zero our timestamp
	m_rtFrame = 0;

	got_picture = 0;
	return S_OK;
}

HRESULT VideoDecoder::OnFlush()
{
	OnDiscontinuity();

	//  Release buffer
	SafeRelease(&m_pBuffer);

	return S_OK;
}
