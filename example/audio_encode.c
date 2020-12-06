#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>


int audio_encode(const char *input, const char *output)
{
    const AVCodec *pCodec = NULL;
    
    pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (pCodec == NULL){
        av_log(NULL, AV_LOG_INFO, "aac encodec find fail\n");
        exit(-1);
    }

    AVCodecContext *pCodecCtx = NULL;
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL){
        av_log(NULL, AV_LOG_INFO, "avcodec_alloc_context3 fail\n");
        exit(-1);
    }

    pCodecCtx->sample_rate = 44100;
    pCodecCtx->channels = 1;
    pCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    pCodecCtx->bit_rate = 640000;

    pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        av_log(NULL, AV_LOG_INFO, "avcodec open fail\n");
        exit(-1);
    }

    AVFormatContext *pFormatCtx = NULL;
    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, output);
    if (pFormatCtx == NULL){
        av_log(NULL, AV_LOG_INFO, "avformat_alloc_output_context2 fail\n");
        exit(-1);
    }

    AVStream *stream = avformat_new_stream(pFormatCtx, NULL);
    //stream->codecpar->codec_type
    avcodec_parameters_from_context(stream->codecpar, pCodecCtx);

    av_dump_format(pFormatCtx, 0, output, 1);

    if (avio_open2(&pFormatCtx->pb, output, AVIO_FLAG_WRITE, NULL, NULL) < 0){
        av_log(NULL, AV_LOG_INFO, "avio_open2 fail\n");
        exit(-1);
    }

    if (AVSTREAM_INIT_IN_WRITE_HEADER != avformat_write_header(pFormatCtx, NULL)){
        exit(-1);
    }

    SwrContext *swrCtx = NULL;
    swrCtx = swr_alloc_set_opts(swrCtx, pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate,
                        AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_FLTP, 44100, 0, NULL);

    if (swrCtx == NULL){
        av_log(NULL, AV_LOG_INFO, "swr_alloc_set_opts fail\n");
        exit(-1);
    }

    if (swr_init(swrCtx) < 0){
        exit(-1);
    }

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    if (packet == NULL || frame == NULL){
        av_log(NULL, AV_LOG_INFO, "alloc packet or frame fail\n");
        goto over;
    }

    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channels = 1;
    frame->channel_layout = AV_CH_LAYOUT_MONO;
    frame->nb_samples = 1024;

    if (0 != av_frame_get_buffer(frame, 0)){
        av_log(NULL, AV_LOG_INFO, "av_frame_get_buf fail\n");
        exit(-1);
    }
    int formatSize = av_get_bytes_per_sample(AV_SAMPLE_FMT_FLTP);
    int readSize = frame->nb_samples * frame->channels * formatSize;


    char *pcmBuf = (char *)av_malloc(readSize);
    
    FILE *ifile = fopen(input,"rb");
    //#####FILE *ofile = fopen(output,"wb");
    for (;;){
        int readLen = fread(pcmBuf, 1, readSize, ifile);
        if (readLen <= 0){
            break;
        }

        const uint8_t *pdata[1];
        pdata[0] = pcmBuf;

        swr_convert(swrCtx, frame->data, frame->nb_samples, pdata, frame->nb_samples);
        if (avcodec_send_frame(pCodecCtx, frame) != 0)
            continue;

        av_init_packet(packet);

        // if (avcodec_receive_packet(pCodecCtx, packet) != 0)
        //     continue;

        // packet->stream_index = 0;
        // packet->dts = 0;
        // packet->pts = 0;

        // av_interleaved_write_frame(pFormatCtx, packet);

        while(avcodec_receive_packet(pCodecCtx, packet) == 0){
            //#####fwrite(packet->data, 1, packet->size, ofile);
            packet->stream_index = 0;
            packet->dts = 0;
            packet->pts = 0;
            if (packet->size > 0){
                av_interleaved_write_frame(pFormatCtx, packet);
            }
        }

        av_packet_unref(packet);
    }

    av_write_trailer(pFormatCtx);
over:
    if (pFormatCtx->pb)
        avio_close(pFormatCtx->pb);
    if (pCodec)
        avcodec_close(pCodecCtx);
    if (pCodecCtx)
        avcodec_free_context(&pCodecCtx);
    if (pFormatCtx)
        avformat_free_context(pFormatCtx);
    if (pcmBuf)
        av_free(pcmBuf);
    return 0;
}