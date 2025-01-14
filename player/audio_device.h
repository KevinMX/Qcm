#pragma once

#include <atomic>
#include <cmath>
#include <cubeb/cubeb.h>
#include <optional>
#include <thread>
#include <utility>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>
#include <memory>

#include "core/core.h"
#include "core/log.h"
#include "audio_frame_queue.h"
#include "player/notify.h"

DEFINE_CONVERT(AVSampleFormat, cubeb_sample_format) {
    switch (in) {
    case CUBEB_SAMPLE_S16LE:
    case CUBEB_SAMPLE_S16BE: out = AVSampleFormat::AV_SAMPLE_FMT_S16; break;
    case CUBEB_SAMPLE_FLOAT32LE:
    case CUBEB_SAMPLE_FLOAT32BE:
    default: out = AVSampleFormat::AV_SAMPLE_FMT_FLT;
    }
}

DEFINE_CONVERT(player::AudioParams, cubeb_stream_params) {
    out.sample_rate = in.rate;
    out.format      = convert_from<AVSampleFormat>(in.format);
    av_channel_layout_default(&out.ch_layout, in.channels);
};

namespace player
{

struct DeviceError {
    using Self = DeviceError;

    DeviceError(int e): code(e) {}
    Self& operator=(int e) noexcept {
        code = e;
        return *this;
    }
    bool operator==(int e) const noexcept { return e == code; }
    operator bool() const { return code < 0; }

    int code;
};

struct DeviceDescription {
    cubeb_devid devid;
    std::string device_id;
    std::string friendly_name;
};

class DeviceContext : NoCopy {
public:
    using up_device_collection =
        up<cubeb_device_collection, std::function<void(cubeb_device_collection*)>>;

    DeviceContext(std::string_view name): m_name(name) {
        cubeb_init(&m_cubeb_ctx, m_name.c_str(), NULL);
    }
    ~DeviceContext() { cubeb_destroy(m_cubeb_ctx); }

    std::vector<DeviceDescription> get_devices(cubeb_device_type type) {
        auto up_devices =
            up_device_collection(new cubeb_device_collection(), [this](cubeb_device_collection* p) {
                cubeb_device_collection_destroy(m_cubeb_ctx, p);
            });

        auto& devices = *up_devices;
        if (cubeb_enumerate_devices(m_cubeb_ctx, type, &devices) != CUBEB_OK) return {};

        std::span<cubeb_device_info> device_infos { devices.device, devices.count };

        std::vector<DeviceDescription> out;
        for (auto& info : device_infos) {
            DeviceDescription dp { info.devid, info.device_id, info.friendly_name };
            out.emplace_back(dp);
        }
        return out;
    }

    int get_min_latency(cubeb_stream_params& params, u32& latency) {
        return cubeb_get_min_latency(m_cubeb_ctx, &params, &latency);
    }

    nstd::expected<rc<cubeb_stream>, int> stream_init(
        char const* stream_name, cubeb_devid input_device, cubeb_stream_params* input_stream_params,
        cubeb_devid output_device, cubeb_stream_params* output_stream_params, unsigned int latency,
        cubeb_data_callback data_callback, cubeb_state_callback state_callback, void* user_ptr

    ) {
        cubeb_stream* st { nullptr };
        auto          res = cubeb_stream_init(m_cubeb_ctx,
                                     &st,
                                     stream_name,
                                     input_device,
                                     input_stream_params,
                                     output_device,
                                     output_stream_params,
                                     latency,
                                     data_callback,
                                     state_callback,
                                     user_ptr);
        if (res != CUBEB_OK) {
            return nstd::unexpected(res);
        } else {
            return std::shared_ptr<cubeb_stream>(st, &cubeb_stream_destroy);
        }
    }

private:
    std::string m_name;
    cubeb*      m_cubeb_ctx;
};

class Device {
public:
    using Self                       = Device;
    static constexpr u32 kBlockCount = 24;

    Device(rc<DeviceContext> ctx, cubeb_devid devid, i32 channels, i32 samplerate,
           Notifier notifier)

        : m_ctx(ctx),
          m_stream(nullptr),
          m_channels(channels),
          m_volume(1.0f),
          m_paused(false),
          m_mark_pos(0),
          m_mark_serial(-1),
          m_notifier(notifier),
          m_last_pts(0) {
        cubeb_stream_params output_params;

        output_params.rate     = samplerate;
        output_params.channels = channels;
        output_params.format   = CUBEB_SAMPLE_S16NE;
        output_params.prefs    = CUBEB_STREAM_PREF_NONE;
        output_params.layout   = CUBEB_LAYOUT_UNDEFINED;

        u32 latency = 1;
        m_ctx->get_min_latency(output_params, latency);

        // m_buffer.reserve(m_bytes_per_block * kBlockCount);

        if (auto res = m_ctx->stream_init("Cubeb output",
                                          nullptr,
                                          nullptr,
                                          devid,
                                          &output_params,
                                          latency,
                                          Self::data_cb,
                                          Self::state_cb,
                                          this);
            res) {
            m_stream = res.value();
        } else {
            throw std::runtime_error("can't initialize cubeb device");
        }

        m_audio_params = convert_from<AudioParams>(output_params);
    };
    ~Device() {
        if (m_thread.joinable()) m_thread.join();
    };

    void start() { cubeb_stream_start(m_stream.get()); }
    void stop() { cubeb_stream_stop(m_stream.get()); }

    bool set_volume(float v) const {
        return CUBEB_OK == cubeb_stream_set_volume(m_stream.get(), v);
    }

    void set_output(rc<AudioFrameQueue> in) {
        m_output_queue = in;
        m_output_queue->set_audio_params(m_audio_params);
    }

    bool paused() const { return m_paused; }
    void set_pause(bool v) {
        m_paused = v;
        notify::playstate s;
        s.value = v ? PlayState::Paused : PlayState::Playing;
        m_notifier.send(s).wait();
    }

    void mark_dirty() { m_mark_serial = m_output_queue->serial(); }

    bool dirty() const { return m_mark_serial == m_output_queue->serial(); }

private:
    struct Frame {
        static std::optional<Frame> from(std::optional<AudioFrame> f) {
            return helper::map(f, [](AudioFrame& f_) {
                if (f_.eof()) {
                    return Frame { .frame = std::move(f_), .data = {} };
                } else {
                    auto data = f_.channel_data(0);
                    return Frame { .frame = std::move(f_), .data = data };
                }
            });
        }
        AudioFrame            frame;
        std::span<const byte> data;
        bool                  notified { false };
    };

    void notify(Frame& frame) {
        if (frame.frame.eof()) {
            notify::playstate p { PlayState::Stopped };
            m_notifier.send(p);
        } else {
            notify::position pos;
            pos.value = duration_cast<milliseconds>(frame.frame.pts_duration()).count();
            if (! dirty()) {
                m_notifier.try_send(pos);
            }
        }
    }

    static long data_cb(cubeb_stream*, void* user, const void*, void* outputbuffer, long nframes) {
        auto*      self = (Self*)user;
        const auto size =
            (usize)nframes * self->m_channels * self->m_audio_params.bytes_per_sample();
        std::span<byte> output { (byte*)outputbuffer, size };

        if (! self->dirty()) {
            auto& frame = self->m_cached_frame;
            if (! frame) {
                frame = Frame::from(self->m_output_queue->try_pop());
            }

            while (frame && ! self->paused()) {
                if (! frame->notified) self->notify(frame.value());

                auto copied = std::min(output.size(), frame->data.size());
                std::copy_n(frame->data.begin(), copied, output.begin());
                output      = output.subspan(copied);
                frame->data = frame->data.subspan(copied);

                if (frame->data.empty()) {
                    frame = Frame::from(self->m_output_queue->try_pop());
                }
                if (output.empty()) break;
            };
        } else {
            self->m_output_queue->try_pop();
            self->m_cached_frame = std::nullopt;
        }

        // silence
        std::fill(output.begin(), output.end(), byte {});
        return nframes;
    }

    static void state_cb(cubeb_stream* stream, void*, cubeb_state state) {
        if (! stream) return;
        switch (state) {
        case CUBEB_STATE_STARTED: INFO_LOG("stream started"); break;
        case CUBEB_STATE_STOPPED: INFO_LOG("stream stopped"); break;
        case CUBEB_STATE_DRAINED: INFO_LOG("stream drained"); break;
        default: INFO_LOG("unknown stream state {}", (int)state);
        }
    }

private:
    std::thread m_thread;

    rc<DeviceContext> m_ctx;
    rc<cubeb_stream>  m_stream;

    i32   m_channels;
    float m_volume;

    std::optional<Frame> m_cached_frame;
    std::atomic<bool>    m_paused;
    std::atomic<i32>     m_mark_pos;
    std::atomic<usize>   m_mark_serial;

    AudioParams         m_audio_params;
    rc<AudioFrameQueue> m_output_queue;
    Notifier            m_notifier;
    i64                 m_last_pts;
};

} // namespace player
