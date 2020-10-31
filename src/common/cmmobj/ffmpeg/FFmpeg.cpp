#include <thread>
#include <chrono>
#include <iostream>
#include "log4cxx/Loging.h"
#include "comm/CommFun.h"
#include "FFmpeg.h"

using namespace std;
using namespace common_cmmobj;

CFFmpeg::CFFmpeg()
{
	av_log_set_level(AV_LOG_QUIET);
	av_register_all();
}

CFFmpeg::~CFFmpeg()
{
	av_free(m_uOBuffer);
	av_frame_free(&m_uFrame);
	av_frame_free(&m_uFrameBGR);
	avcodec_close(m_pCodecContext);
}

//打开H264解码器
bool CFFmpeg::OpenH264Avcodec()
{
	m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!m_pCodec) 
	{
		return false;
	}

	m_pCodecContext = avcodec_alloc_context3(m_pCodec);
	if(nullptr == m_pCodecContext)
	{
		return false;
	}
	
	int s32Result = avcodec_open2(m_pCodecContext, m_pCodec, NULL);
	if (0 > s32Result) 
	{
		return false;
	}
	return true;
}

//打开视频文件
bool CFFmpeg::OpenVideoSrc(const string& strSrcPath)
{
	try
	{
		//打开视频文件
		int ret = 0;
		m_pOFmtCtx = avformat_alloc_context();
		if ((ret = avformat_open_input(&m_pOFmtCtx, strSrcPath.c_str(), 0, 0)) < 0) {
			//LOG_SYS(WARN) << string_format("avformat_open_input fail: %s\n", strSrcPath);
			return false;
		}

		if ((ret = avformat_find_stream_info(m_pOFmtCtx, 0)) < 0) {
			//LOG_SYS(WARN) << string_format("avformat_find_stream_info fail: %s\n", strSrcPath);
			return false;
		}

		//获取视频流序号
		for (int i = 0; i < m_pOFmtCtx->nb_streams; i++) {
			AVStream *in_stream = m_pOFmtCtx->streams[i];
			if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
				m_nVideoIndex = i;
			}
		}

		//寻找解码器
		AVCodec *pCodec = NULL;
		m_pCodecContext = m_pOFmtCtx->streams[m_nVideoIndex]->codec;
		m_pCodec = avcodec_find_decoder(m_pCodecContext->codec_id);
		if (m_pCodec == NULL)
		{
			//LOG_SYS(WARN) << string_format("avcode find decoder failed: %s\n", strSrcPath);
			return false;
		}

		//打开解码器
		if (avcodec_open2(m_pCodecContext, m_pCodec, NULL) < 0)
		{
			//LOG_SYS(WARN) << string_format("avcode open failed: %s\n", strSrcPath);
			return false;
		}

		//求帧率
		int den = m_pOFmtCtx->streams[m_nVideoIndex]->r_frame_rate.den;
		int num = m_pOFmtCtx->streams[m_nVideoIndex]->r_frame_rate.num;
		m_fps = (float)num / den;

		return true;
	}
	catch (...)
	{
		//LOG_SYS(WARN) << string_format("OpenVideoSrc Failed: %s\n", strSrcPath);
	}

	return false;
}

//关闭视频文件
bool CFFmpeg::CloseVideoSrc()
{
	avformat_close_input(&m_pOFmtCtx);
}

//获取所选区间视频帧
bool CFFmpeg::GetSectionFrame(int nBeginSec, int nEndSec, FramCallBackFun framCallBackFun)
{
	if (m_pOFmtCtx == nullptr)
	{
		return false;
	}

	if (!SetFrameCallBackFun(framCallBackFun))
	{
		return false;
	}
	
	//根据帧率计算区间开始帧和结束帧
	int nStartFrame = m_fps * nBeginSec;
	int nEndFrame = m_fps * nEndSec;

	//第一关键帧开始检测位置(前移15秒)
	int nIframeBeginSec = (nBeginSec - 15) < 0 ? 0 : nBeginSec - 15;
	int nIframeStartFrame = m_fps * nIframeBeginSec;

	//是否区间内第一关键帧 和 结束帧
	bool bStartKeyFrame = false;
	bool bEndFrame = false;

	int nFrameFinished = 0;
	int nFrameIndex = 0;
	m_bCBSuccess = false;
	m_nFrameCount = 0;
	cv::Mat cvMat;
	AVPacket readPkt;
	while (true)
	{
		if (av_read_frame(m_pOFmtCtx, &readPkt) < 0)
		{
			ExecCallBackFun(cvMat, eStartEndFlag::End);
			break;
		}

		//检测是否视频流
		if (readPkt.stream_index != m_nVideoIndex)
		{
			continue;
		}

		//超过范围
		if (++nFrameIndex >= nEndFrame)
		{
			ExecCallBackFun(cvMat, eStartEndFlag::End);
			break;
		}
		
		//检测是否第一个关键帧
		if (!bStartKeyFrame)
		{
			if ((readPkt.flags & AV_PKT_FLAG_KEY) && nFrameIndex >= nIframeStartFrame)
			{
				bStartKeyFrame = true;
			}
			else
			{
				continue;
			}
		}

		avcodec_decode_video2(m_pCodecContext, m_uFrame, &nFrameFinished, &readPkt);
		if (nFrameFinished && (nFrameIndex > nStartFrame && nFrameIndex < nEndFrame) )
		{
			if (AVframeToCVMat(m_uFrame, cvMat))
			{
				ExecCallBackFun(cvMat, eStartEndFlag::Start);
			}
		}
		av_packet_unref(&readPkt);
	}

	return m_bCBSuccess;
}

//获取所选区间视频帧
bool CFFmpeg::GetFrame(FramCallBackFun framCallBackFun)
{
	if (m_pOFmtCtx == nullptr)
	{
		return false;
	}

	if (!SetFrameCallBackFun(framCallBackFun))
	{
		return false;
	}

	int nFrameFinished = 0;
	int nFrameIndex = 0;
	bool bEndFrame = false;
	m_bCBSuccess = true;
	m_nFrameCount = 0;
	AVPacket readPkt;
	cv::Mat cvMat;
	while (true)
	{
		if (av_read_frame(m_pOFmtCtx, &readPkt) < 0)
		{
			ExecCallBackFun(cvMat, eStartEndFlag::End);
			break;
		}

		//检测是否视频流
		if (readPkt.stream_index != m_nVideoIndex)
		{
			continue;
		}

		avcodec_decode_video2(m_pCodecContext, m_uFrame, &nFrameFinished, &readPkt);
		if (nFrameFinished)
		{
			if (AVframeToCVMat(m_uFrame, cvMat))
			{
				ExecCallBackFun(cvMat, eStartEndFlag::Start);
			}
		}
		av_packet_unref(&readPkt);
	}

	return m_bCBSuccess;
}

//AVframe To CVMat
bool CFFmpeg::AVframeToCVMat(const AVFrame *frame, cv::Mat& cvMat)
{
	AVFrame dst;
	memset(&dst, 0, sizeof(dst));

	int w = frame->width, h = frame->height;
	cvMat = cv::Mat(h, w, CV_8UC3);
	dst.data[0] = (uint8_t *)cvMat.data;
	avpicture_fill((AVPicture *)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);

	struct SwsContext *convert_ctx = NULL;
	enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)frame->format;
	enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;
	convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	sws_scale(convert_ctx, frame->data, frame->linesize, 0, h,
		dst.data, dst.linesize);

	sws_freeContext(convert_ctx);

	return true;
}

//图片解码
void CFFmpeg::DecodePicture(SPropRecord* pRecord, uint8_t* frameBuffer, int frameLength, uint8_t** pPicBuffer)
{
	unsigned char* sPropBytes = pRecord[0].sPropBytes;
	int sPropLength = pRecord[0].sPropLength;
	if (sPropLength <= 0 || sPropBytes == NULL)
	{
		return;
	}

	unsigned char* pPropBytes = pRecord[1].sPropBytes;
	int pPropLength = pRecord[1].sPropLength;
	if (pPropLength <= 0 || pPropBytes == NULL)
	{
		return;
	}

	if (frameLength <= 20) return;
	unsigned char nalu_header[4] = { 0x00, 0x00, 0x00, 0x01 };
	int totalSize = 4 + sPropLength + 4 + pPropLength + 4 + frameLength;
	unsigned char* tmp = new unsigned char[totalSize];
	memcpy(tmp, nalu_header, 4);
	memcpy(tmp + 4, sPropBytes, sPropLength);
	memcpy(tmp + 4 + sPropLength, nalu_header, 4);
	memcpy(tmp + 4 + sPropLength + 4, pPropBytes, pPropLength);
	memcpy(tmp + 4 + sPropLength + 4 + pPropLength, nalu_header, 4);
	memcpy(tmp + 4 + sPropLength + 4 + pPropLength + 4, frameBuffer, frameLength);
	int nFrameFinished = 0;
	AVPacket framePacket;
	av_init_packet(&framePacket);
	framePacket.size = totalSize;
	framePacket.data = tmp;

	do {
		int ret = avcodec_decode_video2(m_pCodecContext, m_uFrame, &nFrameFinished, &framePacket);
		if (ret < 0) {
			break;
		}

		if (nFrameFinished) {
			int p_nWidth = m_uFrame->width;
			int p_nHeight = m_uFrame->height;
			if (m_nWidth == 0) {
				m_nWidth = p_nWidth;
				m_nHeight = p_nHeight;
			}
			else if (m_nHeight != p_nHeight)
			{
				m_nHeight = p_nHeight;
				m_nWidth = p_nHeight;
			}
			if (m_pSwsContext == nullptr) {
				m_pSwsContext = sws_getCachedContext(m_pSwsContext, p_nWidth, p_nHeight, AVPixelFormat::AV_PIX_FMT_YUV420P, p_nWidth, p_nHeight, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
				int size = avpicture_get_size(AV_PIX_FMT_BGR24, m_pCodecContext->width, m_pCodecContext->height);
				m_uOBuffer = (uint8_t*)av_malloc(size);
				avpicture_fill((AVPicture*)m_uFrameBGR, m_uOBuffer, AV_PIX_FMT_BGR24, m_pCodecContext->width, m_pCodecContext->height);
			}
			sws_scale(m_pSwsContext, (const uint8_t* const*)m_uFrame->data, m_uFrame->linesize, 0, p_nHeight, m_uFrameBGR->data, m_uFrameBGR->linesize);
			*pPicBuffer = m_uOBuffer;
		}
	} while (0);
	av_free_packet(&framePacket);
	delete[] tmp;
	tmp = nullptr; //防止产生悬垂指针使程序产生没必要的错误
}

//获取图片宽度
int CFFmpeg::GetWidth()
{
	return m_nWidth;
}

//获取图片高度
int CFFmpeg::GetHeight()
{
	return m_nHeight;
}