#include "stdafx.h"
#include "ffmpeg_h264_extera_data_annexb.h"

//��FFMPEG����չ����ת��ΪMF��ʶ���H264��SPS��PPSͷ��
//΢��MF��H264������Ҫ���ύAnnex-B��ʽ��ͷ���ݡ�
bool ffmpeg_h264_sequence_header_parser(int* profile,int* level,int* flags,byte* psrc,int nsrc_size,byte* pdst,int* ndst_size,bool* raw)
{
	if (psrc == NULL || nsrc_size < 8) //ȷ��������ȷ
		return false;
	bool bRawAnnexB = false;
	if (*(int32_t*)psrc == 0x01000000)
		bRawAnnexB = true;
	if ((*(short*)psrc == 0) && (*(psrc + 2) == 1))
		bRawAnnexB = true;
	if (bRawAnnexB) //�ж��������ĸ�ʽԭʼ����AnnexB��ʽ�ģ����޸ġ�
	{
		*ndst_size = nsrc_size;
		if (profile)
			*profile = *level = *flags = 0;
		if (raw)
			*raw = true;
		memcpy(pdst,psrc,nsrc_size); //ֱ�Ӹ���
		return true;
	}
	if (*psrc != 1) //��ͨNALU��ʽ��mkv��flv��mp4�ȣ�m2ts��ts��H264��ͬ����
		return false;
	if (psrc[1] != 0 && profile) //H264�����ļ�
		*profile = psrc[1];
	if (psrc[3] != 0 && level) //�����ļ��ȼ�
		*level = psrc[3];
	if (psrc[4] != 0 && flags) //flag
		*flags = (psrc[4] & 3) + 1;
	unsigned short nSPS = 0,nPPS = 0;
	nSPS = *(unsigned short*)(psrc + 6) >> 8;
	if (nSPS == 0) //ȷ����SPS
		return false;
	if (nsrc_size < (nSPS + 7))
		return false;
	if (pdst)
	{
		*(int*)pdst = 0x01000000; //����Ϊ00 00 00 01
		memcpy(&pdst[4],psrc + 8,nSPS); //д��SPS
	}
	if (psrc[8 + nSPS] != 1)
		return false;
	nPPS = *(unsigned short*)(psrc + 9 + nSPS) >> 8;
	if (nPPS == 0) //ȷ����PPS
		return false;
	if (pdst)
	{
		*(int*)(&pdst[4 + nSPS]) = 0x01000000; //����00 00 00 01
		memcpy(&pdst[4 + 4 + nSPS],psrc + 11 + nSPS,nPPS); //д��PPS
	}
	if (ndst_size)
		*ndst_size = 4 + 4 + nSPS + nPPS; //д��ͷ��С
	return true;
}