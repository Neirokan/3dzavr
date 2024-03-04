#ifndef OBJECTS_DIRECTIONALLIGHT_H
#define OBJECTS_DIRECTIONALLIGHT_H

#include <objects/Object.h>

class DirectionalLight final : public Object {
private:
    Vec3D _dir;
    Color _color;
public:
    DirectionalLight(const ObjectTag& tag, const Vec3D& direction, const Color& color = Consts::WHITE);
    DirectionalLight(const ObjectTag& tag, const DirectionalLight& directionalLight);

    [[nodiscard]] Color color() const;
    [[nodiscard]] Vec3D direction() const;

    std::shared_ptr<Object> copy(const ObjectTag& tag) const override {
        return std::make_shared<DirectionalLight>(tag, *this);
    }
};


#endif //RAYTRACE3D_DIRECTIONALLIGHT_H
