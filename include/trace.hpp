//
// Created by Lukas Miklosko on 9/1/21.
//
#pragma once

#include <mutex>
#include <iostream>
#include <chrono>
#include <thread>
#include "ilowleveldevices.hpp"

namespace
{
    template<typename... Args>
    void trace_helper(const char* file, int line, Args&& ...args)
    {
        static std::mutex m;

        std::lock_guard guard(m);
        std::cout << "[ TRACE ] " << file+SOURCE_PATH_SIZE << ":" << line << " " << std::this_thread::get_id() << "("
            << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "): ";
        ((std::cout << args),...) << std::endl;
    }
}

#define TRACE(...) do { if (Devices::LowLevelDevicesController::enableTrace) trace_helper(__FILE__, __LINE__, __VA_ARGS__); } while(0)