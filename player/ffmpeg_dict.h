#pragma once

extern "C" {
#include <libavutil/dict.h>
}
#include <cstdlib>
#include <utility>

#include "core/core.h"

namespace player
{
class FFmpegDict : NoCopy {
public:
    FFmpegDict(): m_raw(NULL) {}
    ~FFmpegDict() { av_dict_free(&m_raw); }
    FFmpegDict(FFmpegDict&& o): m_raw(std::exchange(o.m_raw, nullptr)) {}
    FFmpegDict& operator=(FFmpegDict&& o) {
        m_raw = std::exchange(o.m_raw, nullptr);
        return *this;
    }

    int set(const char* key, const char* value, int flag = 0) {
        return av_dict_set(&m_raw, key, value, flag);
    }

    auto get(const char* key, int flag = 0) { return av_dict_get(m_raw, key, NULL, flag); }

    FFmpegDict clone(int flag = 0) {
        FFmpegDict out;
        av_dict_copy(&out.m_raw, m_raw, flag);
        return out;
    }
    auto raw() { return m_raw; }
    auto praw() { return &m_raw; }

private:
    AVDictionary* m_raw;
};

} // namespace player
