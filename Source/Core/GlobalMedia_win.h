//
// AirPodsDesktop - AirPods Desktop User Experience Enhancement Program.
// Copyright (C) 2021 SpriteOvO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include <Config.h>

#if !defined APD_OS_WIN
    #error "This file shouldn't be compiled."
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Control.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../Status.h"
#include "GlobalMedia_abstract.h"

namespace Core::GlobalMedia {
namespace Details {

enum class ActionId : uint32_t { Play, Pause };

class MediaProgramAbstract {
public:
    enum class Priority : uint32_t {
        Max = 0,

        SystemSession = 1,
        MusicPlayer = 2,

        Min = std::numeric_limits<uint32_t>::max()
    };

    virtual inline ~MediaProgramAbstract(){};

    virtual bool IsAvailable() = 0;
    virtual bool IsPlaying() const = 0;
    virtual Status Play() = 0;
    virtual Status Pause() = 0;

    virtual std::wstring GetProgramName() const = 0;
    virtual Priority GetPriority() const = 0;
};

class VolumeLevelLimiter {
private:
    class Callback : public IAudioEndpointVolumeCallback {
    public:
        Callback(std::function<bool(uint32_t)> volumeLevelSetter);

        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvInterface) override;

        HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

        void SetMaxValue(std::optional<uint32_t> volumeLevel);

    private:
        std::atomic<ULONG> _ref{1};
        std::atomic<std::optional<uint32_t>> _volumeLevel;
        std::function<bool(uint32_t)> _volumeLevelSetter;
    };

public:
    VolumeLevelLimiter();
    ~VolumeLevelLimiter();

    void SetMaxValue(std::optional<uint32_t> volumeLevel);

    std::optional<uint32_t> GetVolumeLevel() const;
    bool SetVolumeLevel(uint32_t volumeLevel) const;

private:
    OS::Windows::Com::UniquePtr<IAudioEndpointVolume> _endpointVolume;
    Callback _callback;

    bool Initialize();
};
} // namespace Details

bool Initialize();

class Controller final : public Details::ControllerAbstract<Controller> {
public:
    Controller() = default;

    void Play() override;
    void Pause() override;

    void LimitVolume(std::optional<uint32_t> volumeLevel) override;

private:
    std::mutex _mutex;
    std::vector<std::unique_ptr<Details::MediaProgramAbstract>> _pausedPrograms;
    Details::VolumeLevelLimiter _volumeLevelLimiter;
};
} // namespace Core::GlobalMedia
