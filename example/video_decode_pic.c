#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

static void pic_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

void decode_vf(AVCodecContext *pCodecCtx, AVPacket *packet, AVFrame *frame)
{
    int ret = 0;
    int i = 0;
    int ch = 0;

    char buf[1024];

    if ((ret = avcodec_send_packet(pCodecCtx, packet)) < 0){
        return;
    }

    const char *filename = "dpic";

    while ((ret = avcodec_receive_frame(pCodecCtx, frame)) >= 0){
        snprintf(buf, sizeof(buf), "%s-%d", filename, pCodecCtx->frame_number);
        pic_save(frame->data[0], frame->linesize[0],
                 frame->width, frame->height, buf);        
    }
    
    return;
}

int decode_video_pic(const char *input, const char *output){
    
    AVFormatContext *pFormatCtx = NULL;

    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0)
    {
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input fail\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info fail\n");
        return -1;
    }

    int streamindex = -1;

    for (int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            streamindex = i;
            break;
        }
    }

    if (streamindex == -1)
    {
        av_log(NULL, AV_LOG_INFO, "can not find video stream\n");
        return -1;
    }

    av_dump_format(pFormatCtx, streamindex, input, 0);

    AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[streamindex]->codecpar->codec_id);
    if (pCodec == NULL)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_find_decoder \n");
        return -1;
    }

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_alloc_context3 fain\n");
        return -1;
    }

    if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[streamindex]->codecpar) < 0)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_parameters_to_context fail\n");
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) != 0)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_open2 fail\n");
        return -1;
    }

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    int got_frame = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0){

        if (packet->size <= 0){
            continue;
        }
        if (packet->stream_index == streamindex)
            decode_vf(pCodecCtx, packet, frame);
    }
    return 0;
}