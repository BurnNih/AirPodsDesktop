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

#include <mutex>

#include <QSettings>

namespace Core::Settings {

enum class TrayIconBatteryBehavior : uint32_t { Disable, WhenLowBattery, Always };

namespace Impl {

template <class T>
class BasicSafeAccessor
{
public:
    BasicSafeAccessor(std::mutex &lock, T &fields) : _lock{lock}, _fields{fields} {}
    BasicSafeAccessor(const BasicSafeAccessor &rhs) = delete;

    T *operator->()
    {
        return &_fields;
    }

private:
    std::lock_guard<std::mutex> _lock;
    T &_fields;
};
} // namespace Impl

// Increase this value when the current ABI cannot be backward compatible
// For example, the name or type of an old key has changed
//
constexpr inline uint32_t kFieldsAbiVersion = 1;

// clang-format off
#define SETTINGS_FIELDS(callback)                                                                  \
    callback(bool, auto_run, {false}, Impl::OnApply(&OnApply_auto_run))                            \
    callback(bool, low_audio_latency, {false}, Impl::OnApply(&OnApply_low_audio_latency))          \
    callback(bool, automatic_ear_detection, {true}, Impl::OnApply(&OnApply_automatic_ear_detection))\
    callback(QString, skipped_version, {})                                                         \
    callback(int16_t, rssi_min, {-80}, Impl::OnApply(&OnApply_rssi_min))                           \
    callback(bool, reduce_loud_sounds, {false}, Impl::Deprecated())                                \
    callback(uint32_t, loud_volume_level, {40}, Impl::Deprecated())                                \
    callback(uint64_t, device_address, {0}, Impl::OnApply(&OnApply_device_address), Impl::Sensitive{})\
    callback(TrayIconBatteryBehavior, tray_icon_battery, {TrayIconBatteryBehavior::Disable}, Impl::OnApply(&OnApply_tray_icon_battery))
// clang-format on

struct Fields {
#define DECLARE_FIELD(type, name, dft, ...) type name dft;
    SETTINGS_FIELDS(DECLARE_FIELD)
#undef DECLARE_FIELD
};

enum class LoadResult : uint32_t { AbiIncompatible, NoAbiField, Successful };

LoadResult Load();
void Save(Fields newFields);
void Apply();
Fields GetCurrent();
Fields GetDefault();

using ConstSafeAccessor = Impl::BasicSafeAccessor<const Fields>;

class ModifiableSafeAccessor : public Impl::BasicSafeAccessor<Fields>
{
public:
    using Impl::BasicSafeAccessor<Fields>::BasicSafeAccessor;

    ModifiableSafeAccessor(std::mutex &lock, Fields &fields);
    ~ModifiableSafeAccessor();

private:
    Fields _oldFields;
};

// ConstSafeAccessor ConstAccess();
ModifiableSafeAccessor ModifiableAccess();

} // namespace Core::Settings
