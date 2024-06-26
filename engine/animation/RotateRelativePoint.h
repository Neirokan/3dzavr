#ifndef ANIMATION_ROTATERELATIVEPOINT_H
#define ANIMATION_ROTATERELATIVEPOINT_H

#include <animation/Animation.h>
#include "components/TransformMatrix.h"

class RotateRelativePoint : public Animation {
private:
    const std::weak_ptr<TransformMatrix> _object;
    const Vec3D _targetPoint;
    const Vec3D _rotationValue;

    void update() override {
        auto obj = _object.lock();

        if (obj == nullptr) {
            stop();
            return;
        }

        obj->rotateRelativePoint(_targetPoint, _rotationValue * dprogress());
    }

public:
    RotateRelativePoint(const std::weak_ptr<TransformMatrix>& object, const Vec3D &targetPoint, const Vec3D &rotationValue,
                        double duration = 1, Animation::LoopOut looped = LoopOut::None,
                        Animation::InterpolationType interpolationType = InterpolationType::Bezier)
            : Animation(duration, looped, interpolationType), _object(object), _targetPoint(targetPoint),
              _rotationValue(rotationValue) {}
};

#endif //ANIMATION_ROTATERELATIVEPOINT_H
