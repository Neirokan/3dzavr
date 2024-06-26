#include <utils/Time.h>
#include <utils/Log.h>
#include <Consts.h>
#include <sstream>

using namespace std::chrono;

Time *Time::_instance = nullptr;

void Time::init() {
    if(_instance) {
        Time::free();
    }

    _instance = new Time();
    Log::log("Time::init(): time was initialized");
}

double Time::time() {
    if (!_instance) {
        return 0;
    }

    return _instance->_time;
}

unsigned int Time::frame() {
    if(!_instance) {
        return 0;
    }
    return _instance->_frame;
}

double Time::deltaTime() {
    if (!_instance) {
        return 0;
    }

    return _instance->_deltaTime;
}

void Time::update() {
    if (!_instance) {
        return;
    }

    high_resolution_clock::time_point t = high_resolution_clock::now();

    _instance->_deltaTime = duration<double>(t - _instance->_last).count();
    _instance->_time = duration<double>(t - _instance->_start).count();
    _instance->_last = t;

    _instance->_fpsCounter++;
    if (t - _instance->_fpsStart > _instance->_fpsCountTime) {
        _instance->_lastFps = static_cast<unsigned int>(_instance->_fpsCounter / duration<double>(t - _instance->_fpsStart).count());
        _instance->_fpsCounter = 0;
        _instance->_fpsStart = t;
    }

    _instance->_frame++;
}

unsigned int Time::fps() {
    if (!_instance) {
        return 0;
    }
    // Cast is faster than floor and has the same behavior for positive numbers
    return _instance->_lastFps;
}

void Time::startTimer(const std::string &timerName) {
    if (!_instance) {
        return;
    }

    if(!_instance->_timers.contains(timerName)) {
        _instance->_timers.insert({timerName, Timer()});
    }
    _instance->_timers[timerName].start();
}

void Time::pauseTimer(const std::string &timerName) {
    if (!_instance) {
        return;
    }
    if(_instance->_timers.contains(timerName)) {
        _instance->_timers[timerName].pause();
    }
}

void Time::stopTimer(const std::string &timerName) {
    if (!_instance) {
        return;
    }
    if(_instance->_timers.contains(timerName)) {
        _instance->_timers[timerName].stop();
    }
}

double Time::elapsedTimerMilliseconds(const std::string &timerName) {
    if (!_instance) {
        return 0;
    }
    if(_instance->_timers.count(timerName) > 0) {
        return _instance->_timers[timerName].elapsedMilliseconds();
    }
    return 0;
}

double Time::elapsedTimerSeconds(const std::string &timerName) {
    if (!_instance) {
        return 0;
    }
    if(_instance->_timers.count(timerName) > 0) {
        return _instance->_timers[timerName].elapsedSeconds();
    }
    return 0;
}


void Time::free() {
    if(_instance) {
        _instance->_timers.clear();

        delete _instance;
        _instance = nullptr;
    }

    Log::log("Time::free(): pointer to 'Time' was freed");
}

std::optional<std::reference_wrapper<const std::map<std::string, Timer>>> Time::timers() {
    if (!_instance) {
        return {};
    }

    return {_instance->_timers};
}

std::string Time::getLocalTimeInfo(const std::string &format) {
    std::time_t const now_c = std::time(nullptr);
    auto dt = std::put_time(std::localtime(&now_c), format.c_str());
    std::ostringstream result;
    result << dt;
    return result.str();
}

double Time::fixedDeltaTime() {
    if (!_instance) {
        return 0;
    }

    return _instance->_fixedDeltaTime;
}

void Time::setFixedUpdateInterval(double fixedDeltaTime) {
    if (!_instance) {
        return;
    }

    _instance->_fixedDeltaTime = fixedDeltaTime;
}
