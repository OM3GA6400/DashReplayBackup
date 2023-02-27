#include "recorder.h"
#include <sstream>
#include <CCGL.h>
#include <filesystem>
#include <fstream>
#include "replayEngine.h"
#include "clicks.h"

std::string narrow(const wchar_t* str);
inline auto narrow(const std::wstring& str) { return narrow(str.c_str()); }
std::wstring widen(const char* str);
inline auto widen(const std::string& str) { return widen(str.c_str()); }

std::string narrow(const wchar_t* str) {
    int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) { /* fuck */ }
    auto buffer = new char[size];
    WideCharToMultiByte(CP_UTF8, 0, str, -1, buffer, size, nullptr, nullptr);
    std::string result(buffer, size_t(size) - 1);
    delete[] buffer;
    return result;
}

std::wstring widen(const char* str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if (size <= 0) { /* fuck */ }
    auto buffer = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, buffer, size);
    std::wstring result(buffer, size_t(size) - 1);
    delete[] buffer;
    return result;
}

Recorder::Recorder() : m_width(1280), m_height(720), m_fps(60) {}

void Recorder::start(const std::string& path) {
    m_recording = true;
    m_frame_has_data = false;
    m_current_frame.resize(m_width * m_height * 3, 0);
    m_finished_level = false;
    m_last_frame_t = m_extra_t = 0;
    m_after_end_extra_time = 0.f;
    m_renderer.m_width = m_width;
    m_renderer.m_height = m_height;
    m_renderer.begin();
    auto gm = gd::GameManager::sharedState();
    auto self = gm->getPlayLayer();
    auto song_file = self->m_level->getAudioFileName();
    auto fade_in = self->m_levelSettings->m_fadeIn;
    auto fade_out = self->m_levelSettings->m_fadeOut;    
    auto bg_volume = gm->m_fBackgroundMusicVolume;
    auto sfx_volume = gm->m_fEffectsVolume;
    if (self->m_level->songID == 0)
        song_file = CCFileUtils::sharedFileUtils()->fullPathForFilename(song_file.c_str(), false);
    auto is_testmode = self->m_isTestMode;
    auto song_offset = start_song_offset;
    std::thread([&, path, song_file, fade_in, fade_out, bg_volume, sfx_volume, is_testmode, song_offset]() {
        std::stringstream stream;
        stream << '"' << m_ffmpeg_path << '"' << " -y -f rawvideo -pix_fmt rgb24 -s " << m_width << "x" << m_height << " -r " << m_fps
        << " -i - "; 
        if (!m_codec.empty())
            stream << "-c:v " << m_codec << " ";
        if (!m_bitrate.empty())
            stream << "-b:v " << m_bitrate << " ";
        if (!m_extra_args.empty())
            stream << m_extra_args << " ";
        else
            stream << "-pix_fmt yuv420p ";
        stream << "-vf \"vflip\" -an \"" << path << "\" "; // i hope just putting it in "" escapes it
        Console::WriteLine("executing: " + stream.str());
        auto process = subprocess::Popen(stream.str());
        while (m_recording || m_frame_has_data) {
            m_lock.lock();
            if (m_frame_has_data) {
                const auto frame = m_current_frame; // copy it
                m_frame_has_data = false;
                m_lock.unlock();
                process.m_stdin.write(frame.data(), frame.size());
            }
            else m_lock.unlock();
        }
        if (process.close()) {
            Console::WriteLine("ffmpeg errored :(");
            dashreplay::irecorder::recorder_c = false;
            return;
        }
        Console::WriteLine("video should be done now");
        if (!m_include_audio || !std::filesystem::exists(song_file)) return;
        else {
            wchar_t buffer[MAX_PATH];
            if (!GetTempFileNameW(widen(std::filesystem::temp_directory_path().string()).c_str(), L"rec", 0, buffer)) {
                Console::WriteLine("error getting temp file");
                return;
            }
            auto temp_path = narrow(buffer) + "." + std::filesystem::path(path).filename().string();
            std::filesystem::rename(buffer, temp_path);
            std::stringstream stream;
            stream << '"' << m_ffmpeg_path << '"' << " -y -ss " << song_offset << " -i \"" << song_file
            << "\" -i \"" << path << "\" -t " << m_last_frame_t << " -c:v copy ";
            if (!m_extra_audio_args.empty())
                stream << m_extra_audio_args << " ";
            if (clicks::include_clicks)
                stream << "-filter:a \"volume=0.25[0:a]";
            else 
                stream << "-filter:a \"volume=1[0:a]";
            if (fade_in && !is_testmode)
                stream << ";[0:a]afade=t=in:d=2[0:a]";
            if (fade_out && m_finished_level)
                stream << ";[0:a]afade=t=out:d=2:st=" << (m_last_frame_t - m_after_end_duration - 3.5f) << "[0:a]";
            Console::WriteLine("in " + to_string(fade_in) + " out " + to_string(fade_out));
            stream << "\" \"" << temp_path << "\"";
            Console::WriteLine("executing: " + stream.str());
            auto process = subprocess::Popen(stream.str());
            if (process.close()) {
                Console::WriteLine("oh god adding the song went wrong cmon");
                return;
            }
            std::filesystem::remove(widen(path));
            std::filesystem::rename(temp_path, widen(path));
            Console::WriteLine("video + audio should be done now!");
        }

        if (!clicks::include_clicks && dashreplay::replay::p1.empty()) return;
        else {
            if (!clicks::do_clicks(false)) {
                Console::WriteLine("Generating clicks went wrong :(");
                return;
            }
            string extension = path.substr(path.find_last_of(".") + 1);
            string merge = '"' + m_ffmpeg_path + "\" -y -i \"" + path + "\" -i \"" + clicks::output +
                "\" -c:v copy -filter_complex \" [0:a][1:a] amix=inputs=2:duration=longest [audio_out] \" -map 0:v -map \"[audio_out]\" \"" + path.substr(0, path.find_last_of(".")) + + "_clicks." + extension + "\"";
            Console::WriteLine("executing: " + merge);
            auto process = subprocess::Popen(merge);
            if (process.close()) {
                Console::WriteLine("adding clicks went wrong :(");
                return;
            }
            else {
                Console::WriteLine("clicks added");
            }
        }
        dashreplay::irecorder::recorder_c = false;
    }).detach();
}

void Recorder::stop() {
    m_renderer.end();
    m_recording = false;
}

void MyRenderTexture::begin() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);

    m_texture = new CCTexture2D;
    {
        auto data = malloc(m_width * m_height * 3);
        memset(data, 0, m_width * m_height * 3);
        m_texture->initWithData(data, kCCTexture2DPixelFormat_RGB888, m_width, m_height, CCSize(static_cast<float>(m_width), static_cast<float>(m_height)));
        free(data);
    }

    glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &m_old_rbo);

    glGenFramebuffersEXT(1, &m_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->getName(), 0);

    m_texture->setAliasTexParameters();

    m_texture->autorelease();

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_old_rbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
}

void MyRenderTexture::capture(std::mutex& lock, std::vector<u8>& data, volatile bool& lul) {
    glViewport(0, 0, m_width, m_height);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    auto director = CCDirector::sharedDirector();
    auto scene = director->getRunningScene();
    scene->visit();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    lock.lock();
    lul = true;
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    lock.unlock();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
    director->setViewport();
}

void MyRenderTexture::end() {
    m_texture->release();
}

void Recorder::capture_frame() {
    while (m_frame_has_data) {}
    m_renderer.capture(m_lock, m_current_frame, m_frame_has_data);
}

void Recorder::handle_recording(gd::PlayLayer* self, float dt) {
    if (!self->m_hasLevelCompleteMenu || m_after_end_extra_time < m_after_end_duration) {
        if (self->m_hasLevelCompleteMenu) {
            m_after_end_extra_time += dt;
            m_finished_level = true;
        }
        auto frame_dt = 1. / static_cast<double>(m_fps);
        auto time = self->m_time + m_extra_t - m_last_frame_t;
        if (time >= frame_dt) {
            gd::FMODAudioEngine::sharedEngine()->setBackgroundMusicTime(start_song_offset + (float)self->m_time);
            m_extra_t = time - frame_dt;
            m_last_frame_t = self->m_time;
            capture_frame();
        }
    } else {
        stop();
    }
}

void Recorder::update_song_offset(gd::PlayLayer* self) {
    start_song_offset = (float)self->m_time + (self->m_levelSettings->m_songStartOffset + self->timeForXPos2(
        self->m_player1->m_position.x, self->m_isTestMode));
}