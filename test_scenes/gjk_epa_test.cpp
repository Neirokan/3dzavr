#include <iostream>

#include <Engine.h>
#include <io/Screen.h>
#include <utils/WorldEditor.h>
#include <components/lighting/PointLight.h>

class Test final : public Engine {
private:
    std::shared_ptr<WorldEditor> _worldEditor;

    void start() override {
        _worldEditor = std::make_shared<WorldEditor>(screen, world, camera);

        auto transparentYellow = std::make_shared<Material>(MaterialTag("yellow"),
                                                            nullptr, Color(255, 200, 170, 100));

        auto cube1 = std::make_shared<Object>(ObjectTag("cube_1"));
        cube1->addComponent<RigidObject>()->setCollision(true);

        auto boundFn = std::bind(&Test::collisionHandler, this,
                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        cube1->getComponent<RigidObject>()->setCollisionCallBack(boundFn);

        cube1->addComponent<TriangleMesh>(TriangleMesh::Cube())->setMaterial(transparentYellow);

        //cube1->getComponent<TransformMatrix>()->translate(Vec3D(1, 0, 3));
        cube1->getComponent<TransformMatrix>()->translate(Vec3D(-0.1, 0.45, 3.4));

        world->add(cube1);

        auto cube2 = std::make_shared<Object>(ObjectTag("cube_2"));
        cube2->addComponent<RigidObject>();
        cube2->addComponent<TriangleMesh>(TriangleMesh::Cube())->setMaterial(transparentYellow);

        cube2->getComponent<TransformMatrix>()->translate(Vec3D(-1, 0, 3));
        world->add(cube2);

        auto dirLight = std::make_shared<Object>(ObjectTag("Directional Light 1"));
        dirLight->addComponent<DirectionalLight>(Vec3D(1, -1, 1), Color::WHITE, 1.5);
        world->add(dirLight);

        camera->transformMatrix()->translateToPoint(Vec3D(3.3, 2.5, 0.2));
        camera->transformMatrix()->rotateRelativeItself(Vec3D(0, -1, 0));
        camera->transformMatrix()->rotateLeft(0.4);
    };

    void update() override {
        screen->setTitle("3dzavr, " + std::to_string(Time::fps()) + "fps");

        _worldEditor->update();

        if(Keyboard::isKeyTapped(SDLK_TAB)) {
            setDebugInfo(!showDebugInfo());
        }
    };

    void collisionHandler(const CollisionPoint& point,
                          std::shared_ptr<RigidObject> obj1,
                          std::shared_ptr<RigidObject> obj2) {
        // Support vectors on cubes
        int i = 0;
        for(const auto& edge: {point.edge1, point.edge2, point.edge3}) {
            auto vec = Object(ObjectTag("cube_v" + std::to_string(i++)));
            vec.addComponent<TriangleMesh>(TriangleMesh::ArrowTo(edge.p1, edge.p2, 0.01));
            world->replace(vec);
        }

        auto redMaterial = std::make_shared<Material>(MaterialTag("red"),
                                                      nullptr, Color::RED);
        auto greenMaterial = std::make_shared<Material>(MaterialTag("green"),
                                                        nullptr, Color::GREEN);

        // Support points
        i = 0;
        for(const auto& p : {point.edge1, point.edge2, point.edge3}) {
            auto cubeEdge1 = Object(ObjectTag("cube_edge_" + std::to_string(i++)));
            cubeEdge1.addComponent<TriangleMesh>(TriangleMesh::Cube(0.1))->setMaterial(redMaterial);
            cubeEdge1.getComponent<TransformMatrix>()->translateToPoint(p.p1);
            world->replace(cubeEdge1);

            auto cubeEdge2 = Object(ObjectTag("cube_edge_" + std::to_string(i++)));
            cubeEdge2.addComponent<TriangleMesh>(TriangleMesh::Cube(0.1))->setMaterial(greenMaterial);
            cubeEdge2.getComponent<TransformMatrix>()->translateToPoint(p.p2);
            world->replace(cubeEdge2);

            auto cubeEdgeEPA = Object(ObjectTag("cube_edge_EPA_" + std::to_string(i++)));
            cubeEdgeEPA.addComponent<TriangleMesh>(TriangleMesh::Cube(0.1))->setMaterial(redMaterial);
            cubeEdgeEPA.getComponent<TransformMatrix>()->translateToPoint(p.support);
            world->replace(cubeEdgeEPA);
        }

        // EPA polytope edges
        i = 0;
        for(const auto& edge: point.polytope) {
            auto edgeObj = Object(ObjectTag("edge_EPA_" + std::to_string(i++)));
            edgeObj.addComponent<TriangleMesh>(TriangleMesh::ArrowTo(Vec3D(0), edge.support, 0.01));
            world->replace(edgeObj);
        }

        auto blueMaterial = std::make_shared<Material>(MaterialTag("blue"),
                                                       nullptr, Color::BLUE);


        auto collisionP1 = Object(ObjectTag("collisionP1"));
        collisionP1.addComponent<TriangleMesh>(TriangleMesh::Cube(0.1))->setMaterial(blueMaterial);
        collisionP1.getComponent<TransformMatrix>()->translateToPoint(point.point);
        world->replace(collisionP1);
    }

public:
    Test() = default;
};

int main(int argc, char *argv[]) {
    Test test;
    test.create();
    return 0;
}
