#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

void
display(AVCodecContext *, AVPacket *, AVFrame *, double);

int
main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // ffmpeg part
    AVFormatContext *pFormatCtx = NULL;
    int vidId = -1, audId = -1;
    double fpsrendering = 0.0;
    AVCodecContext *vidCtx = NULL, *audCtx = NULL;
    AVCodec *vidCodec = NULL, *audCodec = NULL;
    AVCodecParameters *vidpar = NULL, *audpar = NULL;
    AVFrame *vframe = NULL, *aframe = NULL;
    AVPacket *packet = NULL;

    char bufmsg[1024];
    bool running = true;

    do {
        pFormatCtx = avformat_alloc_context();
        if (!pFormatCtx) {
            perror("Failed to allocate format context");
            break;
        }

        avformat_network_init();

        if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) < 0) {
            sprintf(bufmsg, "Cannot open %s", argv[1]);
            perror(bufmsg);
            break;
        }
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
            perror("Cannot find stream info. Quitting.");
            break;
        }

        bool foundVideo = false;
        for (int i = 0; i < pFormatCtx->nb_streams; i++) {
            AVCodecParameters *localparam = pFormatCtx->streams[i]->codecpar;
            AVCodec *localcodec = avcodec_find_decoder(localparam->codec_id);
            if (localparam->codec_type == AVMEDIA_TYPE_VIDEO && !foundVideo) {
                vidCodec = localcodec;
                vidpar = localparam;
                vidId = i;
                AVRational rational = pFormatCtx->streams[i]->avg_frame_rate;
                fpsrendering =
                  1.0 / ((double)rational.num / (double)(rational.den));
                foundVideo = true;
            }
        }

        if (!vidCodec) {
            perror("No video codec found");
            break;
        }

        vidCtx = avcodec_alloc_context3(vidCodec);
        if (!vidCtx) {
            perror("Failed to allocate video codec context");
            break;
        }

        if (avcodec_parameters_to_context(vidCtx, vidpar) < 0) {
            perror("Error copying video codec parameters to context");
            break;
        }
        if (avcodec_open2(vidCtx, vidCodec, NULL) < 0) {
            perror("Error opening video codec");
            break;
        }

        vframe = av_frame_alloc();
        aframe = av_frame_alloc();
        packet = av_packet_alloc();
        if (!vframe || !aframe || !packet) {
            perror("Error allocating frame or packet");
            break;
        }

        while (running && av_read_frame(pFormatCtx, packet) >= 0) {
            if (packet->stream_index == vidId) {
                display(vidCtx, packet, vframe, fpsrendering);
            }
            av_packet_unref(packet);
        }

    } while (0);

    if (packet)
        av_packet_free(&packet);
    if (vframe)
        av_frame_free(&vframe);
    if (aframe)
        av_frame_free(&aframe);
    if (vidCtx)
        avcodec_free_context(&vidCtx);
    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }

    return 0;
}

void
display(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, double fpsrend) {
    time_t start = time(NULL);
    if (avcodec_send_packet(ctx, pkt) < 0) {
        perror("send packet");
        return;
    }
    if (avcodec_receive_frame(ctx, frame) < 0) {
        perror("receive frame");
        return;
    }

    /* YUV
    frame->data[0], frame->linesize[0],
    frame->data[1], frame->linesize[1],
    frame->data[2], frame->linesize[2]);
    */

    time_t end = time(NULL);
    double diffms = difftime(end, start) / 1000.0;
    if (diffms < fpsrend) {
        uint32_t diff = (uint32_t)((fpsrend - diffms) * 1000);
        // delay(diff)
    }
}
