
#pragma once

#include "../AppConfig.h"

using FatalErrorCallback = std::function<void (g3::FatalMessagePtr)>;
using PreCrashCallback = std::function<void()>;

class LogHandler
{
 public:
    LogHandler (FatalErrorCallback crashCallback, PreCrashCallback precrashCallback);
    ~LogHandler() = default;

 private:
    std::unique_ptr<g3::LogWorker> worker;

}; // end class LogHandler