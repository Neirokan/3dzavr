#include <io/Mouse.h>
#include <utils/Time.h>
#include <utils/Log.h>
#include <Consts.h>

Mouse *Mouse::_instance = nullptr;

Vec2D Mouse::getMousePosition() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return Vec2D(x, y);
}

Vec2D Mouse::getMouseDisplacement() {
    if (_instance == nullptr) {
        return Vec2D(0, 0);
    }

    Vec2D motion(_instance->_motion);
    _instance->_motion = Vec2D();

    return motion;
}

Vec2D Mouse::getMouseScroll() {

    if (_instance == nullptr) {
        return Vec2D(0, 0);
    }

    Vec2D scroll(_instance->_scroll);
    _instance->_scroll = Vec2D();

    return scroll;
}

bool Mouse::isButtonPressed(uint8_t button) {
    if (_instance == nullptr) {
        return false;
    }

    auto it = _instance->_buttons.find(button);

    if (it != _instance->_buttons.end()) {
        return it->second;
    }

    return false;
}

bool Mouse::isButtonTapped(uint8_t button) {
    if (_instance == nullptr) {
        return false;
    }

    if (!Mouse::isButtonPressed(button)) {
        return false;
    }

    if (_instance->_tappedButtons.count(button) == 0) {
        _instance->_tappedButtons.emplace(button, Time::time());
        return true;
    } else if ((Time::time() - _instance->_tappedButtons[button]) > Consts::TAP_DELAY) {
        _instance->_tappedButtons[button] = Time::time();
        return true;
    }
    return false;
}

void Mouse::sendMouseEvent(const SDL_Event &event) {

    if (_instance == nullptr) {
        return;
    }

    switch (event.type) {
        case SDL_MOUSEMOTION:
            _instance->_motion += Vec2D(event.motion.xrel, event.motion.yrel);
            break;
        case SDL_MOUSEWHEEL:
            _instance->_scroll += Vec2D(event.wheel.x, event.wheel.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            _instance->_buttons[event.button.button] = true;
            break;
        case SDL_MOUSEBUTTONUP:
            _instance->_buttons[event.button.button] = false;
            break;
    }
}

void Mouse::init() {
    if(_instance) {
       Mouse::free();
    }

    _instance = new Mouse();

    Log::log("Mouse::init(): mouse was initialized");
}

void Mouse::free() {
    if(_instance) {
        _instance->_tappedButtons.clear();
        delete _instance;
        _instance = nullptr;
    }

    Log::log("Mouse::free(): mouse was freed");
}
