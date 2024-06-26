#ifndef ANIMATION_ROTATE_H
#define ANIMATION_ROTATE_H

#include <animation/Animation.h>
#include "components/TransformMatrix.h"

class Rotate final : public Animation {
private:
    const std::weak_ptr<TransformMatrix> _object;
    const Vec3D _rotationValue;

    void update() override {
        auto obj = _object.lock();

        if (obj == nullptr) {
            stop();
            return;
        }

        obj->rotate(_rotationValue * dprogress());
    }

public:
    Rotate(const std::weak_ptr<TransformMatrix>& object, const Vec3D &r, double duration = 1, LoopOut looped = LoopOut::None,
           InterpolationType interpolationType = InterpolationType::Bezier)
            : Animation(duration, looped, interpolationType), _object(object), _rotationValue(r) {
    }
};

#endif //ANIMATION_ROTATE_H
