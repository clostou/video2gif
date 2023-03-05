//////////////////////////////////////////////////////////////////////////////
//
//
//  --- video2gif.c ---
//
//
//  Function��Convert the video to GIF format, using FFmpeg API.
//
//  Author: Yangwang (GitHub@clostou)
//
//  Creation Date: 2023/3/5
//
//  Last Modified: 2023/3/5
//
//
//////////////////////////////////////////////////////////////////////////////

#include <io.h>
#include <tchar.h>
#include <string.h>
#include <Windows.h>
#include <pthread.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter//buffersink.h>
#include <libavutil/opt.h>

#define MAX_PATH_LENGTH 256
#define MAX_FILTER_LENTH 128
#define FILTER_N 3
#define WAIT_TIME 0.1
//#define DEBUG


/**
* �����ļ���ʽ�����������Ϣ�Ľṹ�塣
*/
typedef struct FileContext {
    AVFormatContext* fmt;
    AVStream* st;
    AVCodecContext* codec;
}FileContext;


/**
* ����һ���µ�FileContext�ṹ�塣ʹ�ý�������Ҫ����file_free�ͷš�
*/
FileContext* file_alloc()
{
    FileContext* file_ctx;

    file_ctx = (FileContext*)malloc(sizeof(FileContext));
    if (file_ctx) {
        file_ctx->fmt = NULL;
        file_ctx->st = NULL;
        file_ctx->codec = NULL;
    }

    return file_ctx;
}


/**
* ���FileContext�ṹ�����������ڴ档
*/
void file_free(FileContext** ctx)
{
    if (*ctx) {
        if ((*ctx)->codec) {
            avcodec_free_context(&(*ctx)->codec);
        }
        (*ctx)->st = NULL;
        if ((*ctx)->fmt) {
            if ((*ctx)->fmt->oformat && ((*ctx)->fmt->oformat->flags & AVFMT_NOFILE)) {
                avio_close((*ctx)->fmt->pb);
            }
            avformat_close_input(&(*ctx)->fmt);
        }
        free(*ctx);
        *ctx = NULL;
    }
}


/**
* ��ý���ļ��е���Ƶ��������FileContext�ṹ�����ں������롣
*/
int read_video(FileContext** ctx, const char* filename, int thread_count)
{
    int ret = 0, stream_idx;
    FileContext* input_ctx;
    AVCodecParameters* codecpar;
    const AVCodec* codec;
    AVDictionary* opts = NULL;

    // �ڴ����
    input_ctx = file_alloc();
    if (!input_ctx) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }
    *ctx = input_ctx;
    
    // ���ļ�����ȡ��ʽͷ������Ϣ
    ret = avformat_open_input(&input_ctx->fmt, filename, NULL, NULL);
    if (ret < 0) {
        printf("Fail to open input file.\n");
        goto end;
    }
    ret = avformat_find_stream_info(input_ctx->fmt, NULL);
    if (ret < 0) {
        printf("Fail to parse stream information.\n");
        goto end;
    }
    
    // ѡ����Ƶ��������ʼ����Ӧ�Ľ�����
    stream_idx = av_find_best_stream(input_ctx->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (stream_idx < 0) {
        printf("Fail to find video stream.\n");
        ret = stream_idx;
        goto end;
    }
    input_ctx->st = input_ctx->fmt->streams[stream_idx];
    codecpar = input_ctx->st->codecpar;
    codec = avcodec_find_decoder(codecpar->codec_id);
    input_ctx->codec = avcodec_alloc_context3(codec);
    ret = avcodec_parameters_to_context(input_ctx->codec, codecpar);
    ret = avcodec_open2(input_ctx->codec, codec, &opts);
    if (ret < 0) {
        printf("Fail to initialize decoder.\n");
        goto end;
    }
    input_ctx->codec->thread_count = thread_count;

end:
    return ret;
}


/**
* ÿ�ε��ö����ctxָ������Ƶ���ж�ȡһ֡��д�뵽frame�С���Ҫһ��AVPacket���͵Ļ��档
*/
int decode(FileContext* ctx, AVFrame* frame, AVPacket* buffer)
{
    int ret;

    do {
        ret = avcodec_receive_frame(ctx->codec, frame);
        if (ret >= 0) {
            frame->pts = frame->best_effort_timestamp;
            break;    // �ɹ���ȡһ֡���˳�
        }
        if (ret == AVERROR(EAGAIN)) {
            av_packet_unref(buffer);    // д��packetǰ���������û���
            // ��ȡԴ�ļ�
            ret = av_read_frame(ctx->fmt, buffer);
            if (ret >= 0 && buffer->stream_index == ctx->st->index || ret == AVERROR_EOF) {
                // ����
                ret = avcodec_send_packet(ctx->codec, buffer);
            }
        }
    } while (ret >= 0);
#ifdef DEBUG
    printf("Decode frame %I64d:  %s (%d)\n", frame->pts, av_err2str(ret), ret);
#endif

    return ret;
}


/**
* ��������ʽ�½�һ��GIF�ļ�������FileContext�ṹ�����ں������롣
*/
int write_gif(FileContext** ctx, const char* filename, int width, int height, \
    enum AVPixelFormat pixel_fmt, int thread_count)
{
    int ret = 0;
    FileContext* output_ctx;
    const AVOutputFormat* fmt;
    enum AVCodecID codec_id;
    const AVCodec* codec;
    AVCodecContext* codec_ctx;
    AVDictionary* opt = NULL;
    AVStream* stream;
    AVRational time_base = { 1, 100 };    // gif��ʽʱ���Ĭ��Ϊ100���Ҳ��ɸı�

    // �ڴ����
    output_ctx = file_alloc();
    if (!output_ctx) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }
    *ctx = output_ctx;

    // ��ʼ������ļ�
    ret = avformat_alloc_output_context2(&output_ctx->fmt, NULL, "gif", filename);
    if (ret < 0) {
        printf("Fail to open output file.");
        goto end;
    }
    fmt = output_ctx->fmt->oformat;
    if (!(fmt->flags & AVFMT_NOFILE)) {
        // ����װ��ʽҪ�����ֶ�������ļ�
        ret = avio_open(&output_ctx->fmt->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("Fail to open output file.");
            goto end;
        }
    }	

    // ���Ҽ����ý�����
    codec_id = fmt->video_codec;
    codec = avcodec_find_encoder(codec_id);
    codec_ctx = avcodec_alloc_context3(codec);
    if (codec_ctx) {
        codec_ctx->codec_id = codec_id;
        codec_ctx->width = width;
        codec_ctx->height = height;
        codec_ctx->pix_fmt = pixel_fmt;
        codec_ctx->time_base = time_base;
        codec_ctx->thread_count = thread_count;
        if (fmt->flags & AVFMT_GLOBALHEADER)
            // ���ָ�ʽʹ��ȫ������ͷ�������Ǳ��ָÿ���ؼ�֡��
            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    ret = avcodec_open2(codec_ctx, codec, &opt);
    if (ret < 0) {
        printf("Fail to initialize encoder.\n");
        goto end;
    }
    output_ctx->codec = codec_ctx;

    // ������������ø�������Ϣ
    stream = avformat_new_stream(output_ctx->fmt, codec);
    if (!stream) {
        printf("Fail to add media stream.\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ret = avcodec_parameters_from_context(stream->codecpar, codec_ctx);
    stream->id = output_ctx->fmt->nb_streams - 1;
    stream->time_base = time_base;
    if (ret < 0) {
        printf("Fail to initialize media stream.\n");
        goto end;
    }
    output_ctx->st = stream;
    
end:
    return ret;
}


/**
* ÿ�ε��ö��Ὣframe������һ֡����д��ctxָ��������ļ��С���Ҫһ��AVPacket���͵Ļ��档
*/
int encode(FileContext* ctx, AVFrame* frame, AVPacket* buffer)
{
    int ret;

    // ����
    ret = avcodec_send_frame(ctx->codec, frame);
    if (ret >= 0) {
        av_packet_unref(buffer);    // д��packetǰ���������û���
        ret = avcodec_receive_packet(ctx->codec, buffer);
        if (ret >= 0) {
            // д��Ŀ���ļ�
            ret = av_interleaved_write_frame(ctx->fmt, buffer);
        }
    }
#ifdef DEBUG
    if (frame) {
        printf("Encode frame %I64d:  %s (%d)\n", frame->pts, av_err2str(ret), ret);
    }
#endif

    return ret;
}


/**
* ����һ�׶�������ͼ���˾��߳̽ṹ�塣
*/
typedef struct FilterThreadContext {
    int id;
    AVFrame* frame;
    AVFilterGraph* graph;
    AVFilterContext* buf_filter;
    AVFilterContext* sink_filter;
    double pts_factor;
    int* flag;
}FilterThreadContext;


/**
* ���FilterThreadContext�ṹ�����������ڴ档
*/
void filter_free(FilterThreadContext** ctx)
{
    if (*ctx) {
        if ((*ctx)->frame) {
            av_frame_free(&(*ctx)->frame);
        }
        (*ctx)->buf_filter = NULL;
        (*ctx)->sink_filter = NULL;
        if ((*ctx)->graph) {
            avfilter_graph_free(&(*ctx)->graph);
        }
        if ((*ctx)->flag) {
            free((*ctx)->flag);
        }
    }
}


/**
* �ɸ�����������һ���µ��˾��ṹ�塣ע��ʹ�ý�������Ҫ����filter_free���ͷ��ڴ档
*/
int create_filter(FilterThreadContext** ctx, char** filters, int count, \
    char* buf_filter_args, enum AVPixelFormat sink_filter_pix_fmt)
{
    int ret = 0;
    FilterThreadContext* filter_ctx;
    const AVFilter* buf_filter, * sink_filter;
    enum AVPixelFormat pixel_fmts[] = {sink_filter_pix_fmt, AV_PIX_FMT_NONE};
    AVFilterInOut* inputs = NULL, * outputs = NULL;

    // �ڴ����
    filter_ctx = (FilterThreadContext*)malloc(sizeof(FilterThreadContext));
    if (!filter_ctx) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }
    *ctx = filter_ctx;
    filter_ctx->frame = av_frame_alloc();
    filter_ctx->graph = avfilter_graph_alloc();
    filter_ctx->buf_filter = NULL;
    filter_ctx->sink_filter = NULL;
    filter_ctx->flag = (int*)malloc(sizeof(int));
    if (!filter_ctx->frame || !filter_ctx->graph || !filter_ctx->flag) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }

    // ����Դ�������ͽ��չ�����
    buf_filter = avfilter_get_by_name("buffer");
    ret = avfilter_graph_create_filter(&filter_ctx->buf_filter, buf_filter, "buffer", buf_filter_args, NULL, filter_ctx->graph);
    sink_filter = avfilter_get_by_name("buffersink");
    ret = avfilter_graph_create_filter(&filter_ctx->sink_filter, sink_filter, "buffersink", NULL, NULL, filter_ctx->graph);
    ret = av_opt_set_int_list(filter_ctx->sink_filter, "pix_fmts", pixel_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0 || !filter_ctx->buf_filter || !filter_ctx->sink_filter) {
        printf("Fail to initialize filter graph.\n");
        if (ret >= 0) ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }

    // �������������ӱ�
    outputs = avfilter_inout_alloc();
    inputs = avfilter_inout_alloc();
    if (outputs && inputs) {
        outputs->name = av_strdup("in");
        outputs->filter_ctx = filter_ctx->buf_filter;
        outputs->pad_idx = 0;
        outputs->next = NULL;
        inputs->name = av_strdup("out");
        inputs->filter_ctx = filter_ctx->sink_filter;
        inputs->pad_idx = 0;
        inputs->next = NULL;
    }
    else {
        printf("Fail to initialize filter graph.\n");
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }

    // �����������ַ���������ӹ�����
    for (int i = 0; i < count; i++) {
        ret = avfilter_graph_parse_ptr(filter_ctx->graph, filters[i], &inputs, &outputs, NULL);
        if (ret < 0) {
            printf("Fail to parse filter '%s'.\n", filters[i]);
            goto end;
        }
    }

    // ��֤����������
    ret = avfilter_graph_config(filter_ctx->graph, NULL);
    if (ret < 0) {
        printf("Fail to verify filter graph.\n");
        goto end;
    }

end:
    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);

    return ret;
}


void* filting(void* arg)
{
    FilterThreadContext* td = (FilterThreadContext*)arg;
    int64_t pts;
    int ret;
    
    while (1) {
        while (*td->flag != 1) {
            if (*td->flag == -1) {
                pthread_exit(NULL);
            }
            Sleep(WAIT_TIME);    // ��flag��Ϊ��1ʱ�ȴ�
        };
#ifdef DEBUG
        printf("Filt-%d:  Receive\n", td->id);
#endif
        pts = td->frame->pts;
        ret = av_buffersrc_add_frame(td->buf_filter, td->frame);
        ret = av_buffersink_get_frame(td->sink_filter, td->frame);
        if (ret >= 0) {
            td->frame->pts = (int64_t)(td->pts_factor * pts);    // �ֶ�����ʱ���
        }
        *td->flag = ret;
#ifdef DEBUG
        printf("Filt-%d:  %s (%d)\n", td->id, av_err2str(ret), ret);
#endif
        };
    
    return NULL;
}


/**
* ���ڴ���gifת�����õı����ṹ�塣
*/
typedef struct ConfigureData {
    float scale;
    float speed;
    int fps;
    int depth;
    int thread;
}ConfigureData;


/**
* ���ļ��ж�ȡת�����á����ļ������ڣ�������д�������ļ���
*/
int read_config(ConfigureData* config, const char* ini_file)
{
    int ret = 0;
    FILE* fp = NULL;

    if (!_access(ini_file, 0)) {
        ret = fopen_s(&fp, ini_file, "r");
        if (fp) {
            ret = fscanf_s(fp, "Image Scale\t=\tx%f\nPlay Speed\t=\tx%f\nFrame Rate\t=\t%d\nColor Depth\t=\t%dbit\nThread Count\t=\t%d", \
                &config->scale, &config->speed, &config->fps, &config->depth, &config->thread);
            ret = ret == 5 ? 0 : -1;
            fclose(fp);
            if (ret < 0) {
                printf("Failed to parse configure from ini file.\n");
                goto end;
            }
        }
        else {
            printf("Failed to open ini file.\n");
            goto end;
        }
    }
    else {
        ret = fopen_s(&fp, ini_file, "w");
        if (fp) {
            ret = fprintf(fp, "Image Scale\t=\tx%.1f\nPlay Speed\t=\tx%.1f\nFrame Rate\t=\t%d\nColor Depth\t=\t%dbit\nThread Count\t=\t%d", \
                config->scale, config->speed, config->fps, config->depth, config->thread);
            ret = ret > 0 ? 0 : -1;
            fclose(fp);
            if (ret < 0) {
                printf("Failed to write configure to ini file.\n");
                goto end;
            }
        }
        else {
            printf("Failed to create ini file.\n");
            goto end;
        }
    }
#ifdef DEBUG
    printf("Configure: scale=x%.1f,speed=x%.1f,fps=%d,depth=%dbit,thread=%d\n\n", \
        config->scale, config->speed, config->fps, config->depth, config->thread);
#endif

end:
    return ret;
}


/**
* ����configָ�������ã�����Ƶ�ļ�srcת��ΪGIF��ʽ�����ļ�dst��
*/
int video2gif(const char* src, const char* dst, ConfigureData* config)
{
    int ret = 0, t_ptr = 0, flag;
    FileContext* input = NULL, * output = NULL;
    FilterThreadContext** filter = NULL;
    pthread_t* threads = NULL;
    AVPacket* packet = NULL;
    AVFrame* frame = NULL, * frame_tmp = NULL;
    double pts_factor;
    double pts_offset = .0, pts_interval;

    char filter_args[MAX_FILTER_LENTH] = { 0 };
    char* filter_list[FILTER_N] = { NULL };
    enum AVPixelFormat pixel_fmt = AV_PIX_FMT_PAL8;

    // �������ļ�
    ret = read_video(&input, src, config->thread);
    if (ret < 0) {
        goto end;
    }
    av_dump_format(input->fmt, input->st->index, src, 0);
    printf("\n");

    // �����˾�����˲���
    snprintf(filter_args, sizeof(filter_args), \
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", \
        input->codec->width, input->codec->height, input->codec->pix_fmt, input->st->time_base.num, \
        input->st->time_base.den, input->st->sample_aspect_ratio.num, input->st->sample_aspect_ratio.den);
#ifdef DEBUG
    printf("\nFilter Input:\n%s\n", filter_args);
#endif

    // �����˾��������ַ���
    for (int i = 0; i < FILTER_N; i++) {
        filter_list[i] = (char*)malloc(MAX_FILTER_LENTH * sizeof(char));
        if (!filter_list[i]) {
            ret = AVERROR_BUFFER_TOO_SMALL;
            goto end;
        }
    }
    snprintf(filter_list[0], MAX_FILTER_LENTH, "[in]scale=%.0f:-1,split[split1][split2]", \
        (double)config->scale * input->codec->width);
    snprintf(filter_list[1], MAX_FILTER_LENTH, "[split1]palettegen=max_colors=%d:stats_mode=single[pal]", \
        (config->depth >= 8)?256:256>>(8 - config->depth));
    snprintf(filter_list[2], MAX_FILTER_LENTH, "[split2][pal]paletteuse=new=%d[out]", \
        (config->thread == 1) ? 0 : 1);
#ifdef DEBUG
    printf("\nFliter String:\n");
    for (int i = 0; i < FILTER_N; i++)
        printf("%s\n", filter_list[i]);
#endif

    // �����˾�
    filter = (FilterThreadContext**)calloc(config->thread, sizeof(FilterThreadContext*));
    if (!filter) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }
    for (int i = 0; i < config->thread; i++) {
        ret = create_filter(&filter[i], filter_list, FILTER_N, filter_args, pixel_fmt);
        if (ret < 0) {
            goto end;
        }
    }
#ifdef DEBUG
    printf("\nFilter Graph:\n%s\n", avfilter_graph_dump(filter[0]->graph, NULL));
#endif
    
    // ������ļ�
    ret = write_gif(&output, dst, (*filter[0]->sink_filter->inputs)->w, (*filter[0]->sink_filter->inputs)->h, \
        pixel_fmt, config->thread);
    if (ret < 0) {
        goto end;
    }
    output->st->avg_frame_rate = (AVRational){ config->fps, 1 };
    av_dump_format(output->fmt, 0, dst, 1);
    printf("\n");

    // �����˾��ṹ�����������
    pts_factor = (double)output->codec->time_base.den / ((double)config->speed * input->st->time_base.den);
    for (int i = 0; i < config->thread; i++) {
        filter[i]->id = i;
        filter[i]->pts_factor = pts_factor;
        *filter[i]->flag = AVERROR(EAGAIN);
    }

    // �ڴ����
    packet = av_packet_alloc();
    frame = av_frame_alloc();
    frame_tmp = av_frame_alloc();
    if (!packet || !frame || !frame_tmp) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }
    threads = (pthread_t*)malloc(config->thread * sizeof(pthread_t));
    if (!threads) {
        ret = AVERROR_BUFFER_TOO_SMALL;
        goto end;
    }

    // �����˾��߳�
    for (int i = 0; i < config->thread; i++) {
        pthread_create(&threads[i], NULL, filting, (void*)filter[i]);
    }

    ret = avformat_write_header(output->fmt, NULL);

    // ����-�˾�-���� ��ѭ��
    pts_interval = (double)output->codec->time_base.den / (pts_factor * config->fps);
#ifdef DEBUG
    printf("pts_factor: %.3f; pts_offset: %.3f; pts_interval: %.3f\n\n", pts_factor, pts_offset, pts_interval);
#endif
    do {
        ret = decode(input, frame, packet);
        if (ret < 0 && ret != AVERROR_EOF) break;
        while (*filter[t_ptr]->flag == 1) {
            Sleep(WAIT_TIME);
        }
        flag = *filter[t_ptr]->flag;
        if (ret >= 0) {
            if (flag != -1) {
                if (frame->pts >= pts_offset) {
                    av_frame_move_ref(frame_tmp, filter[t_ptr]->frame);
                    av_frame_move_ref(filter[t_ptr]->frame, frame);
                    av_frame_move_ref(frame, frame_tmp);
#ifdef DEBUG
                    printf("Exchange frame with thread-%d)\n", t_ptr);
#endif
                    *filter[t_ptr]->flag = 1;    // ֪ͨ�߳̽���֡����
                    if (flag == 0) {
                        ret = encode(output, frame, packet);
                    }
                    pts_offset += pts_interval;
                }
                else {
                    continue;
                }
            }
        }
        else {
            if (flag != -1) {
                *filter[t_ptr]->flag = -1;    // ����˾�Ϊ�ѹر�
                if (flag == 0) {
                    av_frame_unref(frame);
                    av_frame_move_ref(frame, filter[t_ptr]->frame);
                    ret = encode(output, frame, packet);
                }
            }
            else {
                ret = encode(output, NULL, packet);
            }
        }
        // �߳�ָ��ѭ��
        if (t_ptr < config->thread - 1) {
            t_ptr++;
        }
        else {
            t_ptr = 0;
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN));
    if (ret != AVERROR_EOF) {
        printf("Error occurred when precessing.\n");
        goto end;
    }

    ret = av_write_trailer(output->fmt);

    // �ͷ��ڴ�
end:
    if (packet)
        av_packet_free(&packet);
    if (frame)
        av_frame_free(&frame);
    if (frame_tmp)
        av_frame_free(&frame_tmp);
    if (threads)
        free(threads);
    if (input)
        file_free(&input);
    if (output)
        file_free(&output);
    for (int i = 0; i < FILTER_N; i++) {
        if (filter_list[i])
            free(filter_list[i]);
    }
    if (filter) {
        for (int i = 0; i < config->thread; i++) {
            if (filter[i])
                filter_free(&filter[i]);
        }
        free(filter);
    }

    if (ret < 0) {
        printf("Details: %s\n", av_err2str(ret));
    }
    else {
        printf("Convertion succeed.\n");
    }

    return ret;
}


/**
* ���ô��ڱ��⣬ͬʱ������·���޸�Ϊ�������ڵ��ļ��С�
*/
void set_default_path()
{
    TCHAR strPath[MAX_PATH_LENGTH] = { 0 };
    
    SetConsoleTitle(" Video2gif ");
    if (GetModuleFileName(NULL, strPath, MAX_PATH_LENGTH)) {
        _tcsrchr(strPath, _T('\\'))[1] = 0;
        SetCurrentDirectory(strPath);
    };
}


/**
* �������������ؽ�ԭ�ļ��ĺ�׺�޸�Ϊ.gif�����·����
*/
const char* gif_path(const char* path)
{
    char* suffix;
    static char new_path[MAX_PATH_LENGTH] = { 0 };

    suffix = strrchr(path, '.');
    strncpy_s(new_path, MAX_PATH_LENGTH, path, strlen(path) - strlen(suffix));
    strcat_s(new_path, MAX_PATH_LENGTH, ".gif");

    return new_path;
}


int main(int argc, char** argv)
{
    int ret = 0;
    ConfigureData config = {
        .scale = 1.0,
        .speed = 1.0,
        .fps = 10,
        .depth = 8,
        .thread = 1
    };
    
    set_default_path();
    
    if (argc <= 1) {
        printf("No video file is provided for transcoding, exit.\n");
        goto end;
    }

    ret = read_config(&config, "video2gif.ini");
    if (ret != 0) {
        goto end;
    }
    
    for (int i = 1; i < argc; i++) {
        printf("%d\n------------------------------------------------------------\n", i);
        if (!_access(argv[i], 4)) {
            ret = video2gif(argv[i], gif_path(argv[i]), &config);
        }
        else {
            printf("File doesn't exist or access denied. (%s)\n", argv[i]);
        }
        printf("\n\n");
    }

end:
    printf("\n\n");
    system("pause");

    return 0;
}


