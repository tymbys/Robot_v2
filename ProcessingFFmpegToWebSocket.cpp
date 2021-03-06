
#include "ProcessingFFmpegToWebSocket.h"

//#include <libavcodec/vda.h>
//#include <libavcodec/vdpau.h>


//char *input_name = "video4linux2";
//char *file_name = "/dev/video0";
//char *out_file = "yuv420.yuv";

//shared_ptr<PrintfLogger> logger;
//MyServer *ws_server;
//shared_ptr<MyHandler> handler;

struct IMovimg_AVG {
    //IMovimg_AVG(){};
    //IMovimg_AVG(int * buff, size_t window): window(window),start_index(window), avg_buff(buff){};

#define window 5


    const size_t avg_window = window;
    int avg_buff[window];
    size_t position = 0;
    size_t start_index = window;

    int processing(int val) {
        int data_out = 0;

        if (start_index > 0) start_index--;
        if (position >= window) position = 0;
        avg_buff[position] = val;

        if (start_index == 0) {

            for (size_t i = 0; i < window; i++)
                data_out += avg_buff[i];

            data_out = data_out / window;

        }


        return data_out;
    };

};

IMovimg_AVG moving_avg;

Processing::Processing() {
    cb64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    input_name = "video4linux2";
    file_name = "/dev/video2";
    out_file = "out.jpg";
    //out_file = "out.mpeg";

    //video_size_in = "640x480";
    video_size_in = "1024x720";
    //video_size_in = "1280x720";

    is_skip_frame = false;
}

Processing::~Processing() {

}

std::string Processing::base64Encode(const void* data, size_t length) {
    std::string output;
    const auto dataPtr = reinterpret_cast<const uint8_t*> (data);
    for (auto i = 0u; i < length; i += 3) {
        const auto bytesLeft = length - i;
        const auto b0 = dataPtr[i];
        const auto b1 = bytesLeft > 1 ? dataPtr[i + 1] : 0;
        const auto b2 = bytesLeft > 2 ? dataPtr[i + 2] : 0;
        output.push_back(cb64[b0 >> 2]);
        output.push_back(cb64[((b0 & 0x03) << 4) | ((b1 & 0xf0) >> 4)]);
        output.push_back((bytesLeft > 1 ? cb64[((b1 & 0x0f) << 2) | ((b2 & 0xc0) >> 6)] : '='));
        output.push_back((bytesLeft > 2 ? cb64[b2 & 0x3f] : '='));
    }
    return output;
}

void my_log_callback(void *ptr, int level, const char *fmt, va_list vargs) {
    //vprintf(fmt, vargs);
}

//void Processing::threadServer() {
//    logger = std::make_shared<PrintfLogger>(Logger::Level::DEBUG);
//    ws_server = new MyServer(logger);
//
//    ws_server->setClientBufferSize(100 * 1024 * 1020);
//    ws_server->setLameConnectionTimeoutSeconds(60);
//
//    //shared_ptr<MyHandler> handler = std::make_shared<MyHandler>(ws_server);
//    handler = std::make_shared<MyHandler>(ws_server);
//    //handler->chooseProtocol();
//    ws_server->addWebSocketHandler("/ws", handler);
//
//    ws_server->serve(".", 9090);
//}

void Processing::InitServer() {

    server = thread([this]() {
        //logger = std::make_shared<PrintfLogger>(Logger::Level::DEBUG);
        logger = std::make_shared<PrintfLogger>(verbose ? Logger::Level::DEBUG : Logger::Level::ACCESS);

        ws_server = make_shared<MyServer>(logger);
        //ws_server = new MyServer(logger);

        ws_server->setClientBufferSize(100 * 1024 * 1020);
        ws_server->setLameConnectionTimeoutSeconds(60);

        //shared_ptr<MyHandler> handler = std::make_shared<MyHandler>(ws_server);
        handler = std::make_shared<MyHandler>(ws_server.get());
        //handler->chooseProtocol();
        ws_server->addWebSocketHandler("/ws", handler);

        ws_server->serve(".", this->port);
    });

}

typedef struct Timestamp {
    int seconds;
    int useconds;
} Timestamp;

void get_current_time(Timestamp* timestamp) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timestamp->seconds = (int) (tv.tv_sec);
    timestamp->useconds = (int) (tv.tv_usec);
}

int Processing::ProcessingFFmpegToWebSocket() {

    AVFormatContext *fmtCtx = NULL;
    AVInputFormat *inputFmt = NULL;
    AVPacket *packet = NULL;
    AVCodecContext *pCodecCtx = NULL;
    struct SwsContext *sws_ctx = NULL;

    int i;
    int ret;
    int videoindex;

    enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_YUVJ420P;
    const char *dst_size = NULL;
    const char *src_size = NULL;
    uint8_t * src_data[4];
    uint8_t * dst_data[4];
    int src_linesize[4];
    int dst_linesize[4];
    int src_bufsize;
    int dst_bufsize;

    int src_w;
    int src_h;
//    int dst_w = 1280;
//    int dst_h = 720;

    int dst_w = 1024;
    int dst_h = 720;

    //    int dst_w = 640;
    //    int dst_h = 480;

    ////////////////////////////////////////////////////////////////////////////

    InitServer();

    ////////////////////////////////////////////////////////////////////////////

    if (!verbose) {
        //av_log_set_level(AV_LOG_ERROR);
        av_log_set_level(AV_LOG_QUIET);
        av_log_set_callback(my_log_callback);
    }

    av_register_all();
    avformat_network_init();
    avcodec_register_all();
    avdevice_register_all();
    //av_log_set_level(AV_LOG_QUIET);



    //av_log_set_level(AV_LOG_DEBUG);

    inputFmt = av_find_input_format(input_name.c_str());
    if (inputFmt == NULL) {
        std::cout << "can not find input format. " << std::endl;
        return -1;
    }

    //const char *videosize1 = "640x480";
    AVDictionary *option = NULL;
    av_dict_set(&option, "video_size", video_size_in.c_str(), 0);
    //av_dict_set(&option, "flush_packets", "0", 0);

    //fmtCtx->flush_packets =0;



    if (avformat_open_input(&fmtCtx, file_name.c_str(), inputFmt, &option) < 0) {
        std::cout << "can not open input file." << std::endl;
        return -1;
    }

    //   av_dump_format(fmtCtx, 0, NULL, 0);

    videoindex = -1;
    for (i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }

    if (videoindex == -1) {
        std::cout << "find video stream failed, now return." << std::endl;
        return -1;
    }

    pCodecCtx = fmtCtx->streams[videoindex]->codec;

    std::cout << "picture width height format" << pCodecCtx->width << pCodecCtx->height << pCodecCtx->pix_fmt << std::endl;

    std::cout << "origin pix_fmt " << pCodecCtx->pix_fmt << std::endl;
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt
            , dst_w, dst_h, dst_pix_fmt, SWS_BILINEAR, NULL, NULL, NULL); ////SWS_X SWS_SPLINE   SWS_BICUBIC  SWS_BILINEAR SWS_FAST_BILINEAR

    src_bufsize = av_image_alloc(src_data, src_linesize, pCodecCtx->width, pCodecCtx->height
            , pCodecCtx->pix_fmt, 16);

    dst_bufsize = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pix_fmt, 16);

    packet = (AVPacket *) av_malloc(sizeof (AVPacket));

    uint64_t old_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();


    AVOutputFormat *pOutFormat;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pAVCodecCtx;
    AVStream *pOStream;
    AVCodec *pAVcodec;
    AVFrame *pFrame;

    //const char *out_file = "out.jpg";

    pOutFormat = av_guess_format("mjpeg", NULL, NULL);

    if (!pOutFormat) {
        std::cout << "av guess format failed, now return. " << std::endl;
        return -1;
    }


    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file.c_str());
    pOutFormat = pFormatCtx->oformat;

    //pOutFormat->flags |=AV_FMT_RAWPICTURE;

    pOStream = avformat_new_stream(pFormatCtx, NULL);
    if (!pOStream) {
        std::cout << "new stream failed, now return. " << std::endl;
        return -1;
    }





    pAVCodecCtx = pOStream->codec;
    pAVCodecCtx->codec_id = pOutFormat->video_codec;
    pAVCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pAVCodecCtx->pix_fmt = dst_pix_fmt;
    pAVCodecCtx->width = dst_w;
    pAVCodecCtx->height = dst_h;
    pAVCodecCtx->time_base.num = 1;
    pAVCodecCtx->time_base.den = 25;
    //pAVCodecCtx->flags2 |= AV_CODEC_FLAG2_DROP_FRAME_TIMECODE; //FASTPSKIP;



    //    pAVCodecCtx->gop_size = 12;
    //    pAVCodecCtx->bit_rate_tolerance = 0;
    //    pAVCodecCtx->rc_max_rate = 0;
    //   pAVCodecCtx->rc_buffer_size = 0;
    //    pAVCodecCtx->gop_size = 5;
    //    pAVCodecCtx->max_b_frames = 0;
    //    pAVCodecCtx->b_frame_strategy = 1;
    //    pAVCodecCtx->coder_type = 1;
    //    pAVCodecCtx->me_cmp = 1;
    //    pAVCodecCtx->me_range = 16;
    //    pAVCodecCtx->qmin = 10;
    //    pAVCodecCtx->qmax = 51;
    //    pAVCodecCtx->scenechange_threshold = 40;
    //    //c->flags |= CODEC_FLAG_LOOP_FILTER;
    //    pAVCodecCtx->me_method = ME_HEX;
    //    pAVCodecCtx->me_subpel_quality = 5;
    //    pAVCodecCtx->i_quant_factor = 0.71;
    //    pAVCodecCtx->qcompress = 0.6;
    //pAVCodecCtx->max_qdiff = 4;



    std::cout << "out codec id " << pAVCodecCtx->codec_id << std::endl;

    pAVcodec = avcodec_find_encoder(pAVCodecCtx->codec_id);
    if (!pAVcodec) {
        std::cout << "find avcodec failed. " << std::endl;
        return -1;
    }


    pAVCodecCtx->thread_count = 4;
    pAVCodecCtx->thread_type = FF_THREAD_SLICE;
    //pAVCodecCtx->compression_level =1000;//frame_bits = 12;




    //pAVCodecCtx->flags |= AV_CODEC_FLAG_TRUNCATED; //  ;AV_CODEC_CAP_FRAME_THREADS

    //pAVcodec->capabilities |= AV_CODEC_CAP_FRAME_THREADS;

    av_opt_set(pAVCodecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pAVCodecCtx->priv_data, "tune", "zerolatency", 0);
    //av_dict_set(pAVCodecCtx->opts, "preset","ultrafast",0);




    //pAVCodecCtx->coded_frame->

    //pAVCodecCtx->skip_top=30;          
    if (avcodec_open2(pAVCodecCtx, pAVcodec, NULL) < 0) {
        std::cout << "avcodec open failed. " << std::endl;
        return -1;
    }

    //pAVcodec->capabilities |= AV_CODEC_CAP_FRAME_THREADS;

    int size = avpicture_get_size(pAVCodecCtx->pix_fmt, pAVCodecCtx->width, pAVCodecCtx->height);
    std::cout << "size " << size << std::endl;

    pFrame = av_frame_alloc();
    if (!pFrame) {
        std::cout << "out frame alloc failed, now return. " << std::endl;
        return -1;
    }

    int y_size = pAVCodecCtx->width * pAVCodecCtx->height;
    std::cout << "y size " << y_size << std::endl;




    int64_t num_frame = 0;
    string out = "";

    //fmtCtx->max_delay = 0;

    ret = 0;
    int64_t timeBase = (int64_t(pCodecCtx->time_base.num) * AV_TIME_BASE) / int64_t(pCodecCtx->time_base.den);
    int64_t stream_start_time;
    Timestamp timestamp_prec, timestamp_curr;
    int64_t pkt_ts, cur;

    int avg = 0;
    int diff_round = 0;
    int start_position = 5;

    int fps_max = 0;

    int64_t curent_index = 0;
    int count_skip = 0;

    bool isEncode = false;
    uint64_t now;
    int diff;

    int fps_from_info = fmtCtx->streams[videoindex]->r_frame_rate.num;
    volatile int scip_frame = 0;

    std::cout << "fps_from_info:  " << fps_from_info << std::endl;
    cout << "pAVCodecCtx->thread_count: " << pAVCodecCtx->thread_count << endl;

    //while (av_read_frame(fmtCtx, packet) >= 0) {
    while (1) {
        ret = av_read_frame(fmtCtx, packet);
        curent_index++;

        //uint64_t now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        //double diff = (double) 1000000 / (double) (now - old_time);

        now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        diff = 1000 / (now - old_time);
        old_time = now;



        if ((start_position > 0) && (curent_index > 30) && (is_skip_frame)) {
            start_position--;

            avg += diff;
            if (start_position == 0) {
                start_position--;
                fps_max = avg / moving_avg.avg_window;
                if (fps_max > 0)
                    scip_frame = fps_from_info / fps_max;
            }
        }


        //std::cout << "start_position:  " << start_position << std::endl;




        if (verbose) {
            //diff = avg.processing(diff);
            std::cout << "fps_max:  " << fps_max << std::endl;
            std::cout << "Is read FPS:  " << diff
                    << ",  scip_frame: " << scip_frame << std::endl;
        }

        //        stream_start_time = fmtCtx->streams[packet->stream_index]->start_time;
        //        pkt_ts = packet->pts == AV_NOPTS_VALUE ? packet->dts : packet->pts;
        //        //cur = fmtCtx->streams[packet->stream_index]->cur_dts;
        //        cur = fmtCtx->streams[packet->stream_index]->skip_samples = 15;


        memcpy(src_data[0], packet->data, packet->size);


        sws_scale(sws_ctx, src_data, src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);
        //sws_scale(sws_ctx, (const uint8_t **)packet->data, src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);

        //avpicture_fill((AVPicture *) pFrame, dst_data[0], pAVCodecCtx->pix_fmt, pAVCodecCtx->width, pAVCodecCtx->height);
        av_image_fill_arrays(pFrame->data, pFrame->linesize, dst_data[0], pAVCodecCtx->pix_fmt, pAVCodecCtx->width, pAVCodecCtx->height, 1);


        pAVCodecCtx->qmin = pAVCodecCtx->qmax = 3;
        pAVCodecCtx->mb_lmin = pAVCodecCtx->lmin = pAVCodecCtx->qmin * FF_QP2LAMBDA;
        pAVCodecCtx->mb_lmax = pAVCodecCtx->lmax = pAVCodecCtx->qmax * FF_QP2LAMBDA;
        pAVCodecCtx->flags |= CODEC_FLAG_QSCALE;

        //        pFrame->quality = 4;
        //pFrame->pts = ++i;

        //        avformat_write_header(pFormatCtx, NULL);


        AVPacket pkt = {0};
        int got_picture;
        av_init_packet(&pkt);


        //Encode  
        ret = avcodec_encode_video2(pAVCodecCtx, &pkt, pFrame, &got_picture);

        //ret = 0;
        //pAVCodecCtx->execute2

        if (ret < 0) {
            printf("Encode Error.\n");
            return -1;
        }


        //out = base64Encode((const void*) &pkt.data[0], pkt.size);
        //handler->setCanvas(out);

        handler->setCanvas(&pkt.data[0], pkt.size);



        av_free_packet(&pkt);
        av_free_packet(packet);

        //        if (start_position <= 0) {
        //            
        //            start_position = moving_avg.avg_window;
        //            avg = 0;
        //            scip_frame =0; //!!!!
        //        }



        if (is_skip_frame) {
            for (i = 0; i < scip_frame; i++) {
                ret = av_read_frame(fmtCtx, packet);
                av_free_packet(packet);

            }

            //    std::cout << "scip_frame: " << scip_frame << std::endl;
        }




        //av_free(pFrame);

    }


    if (pOStream) {
        avcodec_close(pOStream->codec);
        av_free(pFrame);
        //      av_free(pFrame_buf);  
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    //av_free_packet(packet);
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);
    avformat_close_input(&fmtCtx);


}

void Processing::SetVerbose(bool v) {
    this->verbose = v;
}

void Processing::SetPort(int port) {
    this->port = port;
}

void Processing::SetVideo(string dev) {
    file_name = dev;
}

void Processing::SetSkipFrame(bool v) {
    this->is_skip_frame = v;
}