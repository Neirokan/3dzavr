#ifndef ANIMATION_AFUNCTION_H
#define ANIMATION_AFUNCTION_H

#include <functional>

#include <animation/Animation.h>

class AFunction final : public Animation {
private:
    int _callsCounter = 0;
    const int _allCalls = 1;
    const std::function<void()> _callBack;

    void update() override {
        if (_allCalls != 0 && progress() >= (double) (_callsCounter + 1) / _allCalls) {
            _callsCounter++;
            _callBack();
        }
    }

public:
    explicit AFunction(std::function<void()> function, int calls = 1, double duration = 1,
                       LoopOut looped = LoopOut::None, InterpolationType interpolationType = InterpolationType::Linear)
            : Animation(duration, looped, interpolationType), _callBack(std::move(function)), _allCalls(calls) {
    }
};

#endif //ANIMATION_AFUNCTION_H
