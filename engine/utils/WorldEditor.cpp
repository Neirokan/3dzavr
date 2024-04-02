#include <utils/ResourceManager.h>
#include <utils/WorldEditor.h>
#include <utils/Log.h>
#include <io/Keyboard.h>
#include <io/Mouse.h>

static const char button_map[256] = {
        [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
        [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
        [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
        [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
        [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
        [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
        [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
        [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
        [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
        [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
        [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};

int textWidth(mu_Font font, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    auto f = ResourceManager::loadFont(Consts::DEFAULT_FONT_FILENAME);
    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) { continue; }
        res += f->getTextSize(std::string(1, *p), 12).first;;
    }
    return res;
}

int textHeight(mu_Font font) {
    return 15;
}

WorldEditor::WorldEditor(std::shared_ptr<Screen> screen, std::shared_ptr<World> world, std::shared_ptr<Camera> camera):
        _screen(screen), _world(world), _camera(camera) {
    _cameraController = std::make_shared<ObjectController>(_camera);
    _selectedObjectBounds = std::make_shared<LineMesh>(LineMesh::Cube(ObjectTag("Cube_frame")));
    _selectedObjectBounds->setVisible(false);
    _world->add(_selectedObjectBounds);

    _ctx = std::make_unique<mu_Context>();
    mu_init(_ctx.get());
    _ctx->text_width = textWidth;
    _ctx->text_height = textHeight;

    //_ctx.get()->style->colors[MU_COLOR_WINDOWBG] = {50, 50, 50, 200};

    // For debug
    _redCube = std::make_shared<TriangleMesh>(TriangleMesh::Cube(ObjectTag("RedCube"), 0.1));
    _redCube->setVisible(_objInFocus);

#ifndef NDEBUG
    _world->add(_redCube);
#endif
}

void WorldEditor::handleInputEvents() {
    auto pos = Mouse::getMousePosition() / Consts::SCREEN_SCALE;
    mu_input_mousemove(_ctx.get(), (int) pos.x(), (int) pos.y());
    mu_input_scroll(_ctx.get(), 0, (int) Mouse::getMouseScroll().y() * -30);
    // Mouse buttons events:
    if (Mouse::isButtonDown()) {
        auto buttons = Mouse::buttonsDown();
        for (const auto &b: buttons) {
            mu_input_mousedown(_ctx.get(), (int) pos.x(), (int) pos.y(), button_map[b & 0xff]);
        }
    }
    if (Mouse::isButtonUp()) {
        auto buttons = Mouse::buttonsUp();
        for (const auto &b: buttons) {
            mu_input_mouseup(_ctx.get(), (int) pos.x(), (int) pos.y(), button_map[b & 0xff]);
        }
    }
    // Keyboard keys events
    if (Keyboard::isKeyDown()) {
        auto keys = Keyboard::keysDown();
        for (const auto &k: keys) {
            mu_input_keydown(_ctx.get(), key_map[k & 0xff]);
        }
    }
    if (Keyboard::isKeyUp()) {
        auto keys = Keyboard::keysUp();
        for (const auto &k: keys) {
            mu_input_keyup(_ctx.get(), key_map[k & 0xff]);
        }
    }
    mu_input_text(_ctx.get(), Keyboard::inputText().c_str());
}

void WorldEditor::drawUiIcon(int id, uint16_t x, uint16_t y) {
    switch (id) {
        case MU_ICON_CLOSE:
            _screen->drawLine(x+9, y+7, x+16, y+14, Color::WHITE_GRAY, 2);
            _screen->drawLine(x+9, y+14, x+16, y+7, Color::WHITE_GRAY, 2);
            break;
        case MU_ICON_CHECK:
            _screen->drawLine(x+7, y+14, x+4, y+11, Color::WHITE_GRAY, 2);
            _screen->drawLine(x+7, y+14, x+15, y+6, Color::WHITE_GRAY, 2);
            break;
        case MU_ICON_COLLAPSED:
            _screen->drawLine(x+7, y+14, x+12, y+10, Color::WHITE_GRAY, 2);
            _screen->drawLine(x+7, y +6, x+12, y+10, Color::WHITE_GRAY, 2);
            break;
        case MU_ICON_EXPANDED:
            _screen->drawLine(x+6, y+8, x+10, y+12, Color::WHITE_GRAY, 2);
            _screen->drawLine(x+14, y+8, x+10, y+12, Color::WHITE_GRAY, 2);
            break;
        default:
            break;
    }
}

void WorldEditor::processFrame() {
    mu_begin(_ctx.get());

    controlPanel();

    mu_end(_ctx.get());
}

void WorldEditor::renderGui() {
    _screen->setDepthTest(false);
    mu_Command *cmd = nullptr;
    while (mu_next_command(_ctx.get(), &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT:
                _screen->drawText(cmd->text.str, cmd->text.pos.x, cmd->text.pos.y,
                          Color(cmd->text.color.r, cmd->text.color.g, cmd->text.color.b, cmd->text.color.a));

                break;
            case MU_COMMAND_RECT:
                _screen->drawRectangle(cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w, cmd->rect.rect.h,
                               Color(cmd->rect.color.r, cmd->rect.color.g, cmd->rect.color.b, cmd->rect.color.a));
                break;
            case MU_COMMAND_ICON:
                drawUiIcon(cmd->icon.id, cmd->icon.rect.x, cmd->icon.rect.y);
                break;
        }
    }
    _screen->setDepthTest(true);
}

void WorldEditor::controlPanel() {
    auto ctx = _ctx.get();

    if (mu_begin_window(ctx, "Control Panel", mu_rect(Consts::STANDARD_SCREEN_WIDTH, 0,
                                                      Consts::STANDARD_EDITOR_WIDTH-Consts::STANDARD_SCREEN_WIDTH, Consts::STANDARD_SCREEN_HEIGHT))) {

        objectEditor();

        if (mu_begin_treenode(ctx, "World tree")) {
            groupTree(_world->objects());

            mu_end_treenode(ctx);
        }

        renderSettings();


        mu_layout_row(ctx, 1, (int[]) { 210 }, 20);
        mu_text(ctx, "Press 'q' to enter/quit free camera mode");
        mu_end_window(ctx);
    }
}

void WorldEditor::groupTree(const std::shared_ptr<Group> &group) {
    auto ctx = _ctx.get();

    if (mu_begin_treenode(ctx, group->name().str().c_str())) {

        mu_layout_row(ctx, 1, (int[]) { 200 }, 0);
        if (group != _world->objects() && mu_button_ex(ctx, "Select the whole group", MU_ICON_CHECK, 1)) {
            _selectedObject = group;
        }

        mu_text(ctx, "The content of the group");
        for(const auto& [objTag, obj] : *group) {
            std::shared_ptr<Group> subGroup = std::dynamic_pointer_cast<Group>(obj);
            if(subGroup) {
                // We need to recursively continue to draw subgroup
                groupTree(subGroup);
            } else {
                mu_layout_row(ctx, 1, (int[]) { 200 }, 0);
                if (mu_button(ctx, objTag.str().c_str())) {
                    _selectedObject = obj;
                }
            }
        }

        mu_end_treenode(ctx);
    }
}

void WorldEditor::objectEditor() {
    auto ctx = _ctx.get();

    if (_selectedObject && mu_header_ex(ctx, "Object editor", MU_OPT_EXPANDED)) {
        mu_text(ctx, ("Object name: " + _selectedObject->name().str()).c_str());


        auto pos = _selectedObject->fullPosition();
        mu_text(ctx, ("X / Y / Z: " + std::to_string(pos.x()) + " / " + std::to_string(pos.y()) + " / " + std::to_string(pos.z()) ).c_str());


        mu_text(ctx, "Transform object:");

        mu_layout_row(ctx, 3, (int[]) { 60, 60, 60 }, 0);
        if (mu_button(ctx, "Translate")) { _operation = 0; }
        if (mu_button(ctx, "Scale")) { _operation = 1; }
        if (mu_button(ctx, "Rotate")) { _operation = 2; }

        float x = 0;
        float y = 0;
        float z = 0;

        switch (_operation) {
            case 0:
                mu_layout_begin_column(ctx);
                mu_layout_row(ctx, 2, (int[]) { 70, 150 }, 0);
                mu_label(ctx, "Translate X:");   mu_slider(ctx, &x, -30, 30);
                mu_label(ctx, "Translate Y:"); mu_slider(ctx, &y, -30, 30);
                mu_label(ctx, "Translate Z:");  mu_slider(ctx, &z, -30, 30);
                mu_layout_end_column(ctx);
                _selectedObject->translate(Vec3D(x, y, z)*Time::deltaTime());
                break;
            case 1:
                mu_layout_begin_column(ctx);
                mu_layout_row(ctx, 2, (int[]) { 70, 150 }, 0);
                mu_label(ctx, "Scale X:");   mu_slider(ctx, &x, -3, 3);
                mu_label(ctx, "Scale Y:"); mu_slider(ctx, &y, -3, 3);
                mu_label(ctx, "Scale Z:");  mu_slider(ctx, &z, -3, 3);
                mu_layout_end_column(ctx);
                _selectedObject->scaleInside(Vec3D(1 + x*Time::deltaTime(),
                                                   1 + y*Time::deltaTime(),
                                                   1 + z*Time::deltaTime()));
                break;
            case 2:
                mu_layout_begin_column(ctx);
                mu_layout_row(ctx, 2, (int[]) { 70, 150 }, 0);
                mu_label(ctx, "Rotate X:");   mu_slider(ctx, &x, -3, 3);
                mu_label(ctx, "Rotate Y:"); mu_slider(ctx, &y, -3, 3);
                mu_label(ctx, "Rotate Z:");  mu_slider(ctx, &z, -3, 3);
                mu_layout_end_column(ctx);
                _selectedObject->rotateRelativePoint(_selectedObject->position(), Vec3D(x, y, z)*Time::deltaTime());
                break;
        }

        mu_layout_row(ctx, 2, (int[]) { 120, 120}, 0);
        if (mu_button(ctx, "Undo Transform")) {
            _selectedObject->transform(_selectedObject->invModel());
        }
        if (mu_button(ctx, "Delete object")) {
            _world->remove(_selectedObject->name());
            _selectedObject = nullptr;
        }
    }
}

void WorldEditor::renderSettings() {
    auto ctx = _ctx.get();

    if (mu_begin_treenode(ctx, "Render Settings")) {

        mu_checkbox(ctx, "Transparent objects", &_enableTransparency);
        mu_checkbox(ctx, "Borders of triangles", &_enableTriangleBorders);
        mu_checkbox(ctx, "Texturing", &_enableTexturing);
        mu_checkbox(ctx, "Texture antialiasing (mipmapping)", &_enableMipmapping);
        mu_checkbox(ctx, "Depth test", &_enableDepthTest);

        if (mu_begin_treenode(ctx, "Lighting")) {
            mu_checkbox(ctx, "Lighting", &_enableLighting);
            mu_checkbox(ctx, "Exact lighting", &_enableTrueLighting);

            mu_layout_begin_column(ctx);
            mu_layout_row(ctx, 2, (int[]) { 70, 150 }, 0);
            mu_label(ctx, "LOD near");   mu_slider(ctx, &_lightingLODNearDistance, 0, 100);
            mu_label(ctx, "LOD far"); mu_slider(ctx, &_lightingLODFarDistance, _lightingLODNearDistance, _lightingLODNearDistance+100);
            mu_layout_end_column(ctx);

            mu_end_treenode(ctx);
        }

        mu_end_treenode(ctx);
    }
}

void WorldEditor::updateScreenSettings() {
    _screen->setLighting(_enableLighting);
    _screen->setTrueLighting(_enableTrueLighting);
    _screen->setTransparency(_enableTransparency);
    _screen->setTriangleBorders(_enableTriangleBorders);
    _screen->setTexturing(_enableTexturing);
    _screen->setMipmapping(_enableMipmapping);
    _screen->setDepthTest(_enableDepthTest);
    _screen->setLightingLODNearDistance(_lightingLODNearDistance);
    _screen->setLightingLODFarDistance(_lightingLODFarDistance);
}

void WorldEditor::updateControllers() {

    if(_isControllerActive) {
        _screen->drawStrokeRectangle(Consts::STANDARD_SCREEN_WIDTH/2-1, Consts::STANDARD_SCREEN_HEIGHT/2-7, 1, 14, Color::BLACK);
        _screen->drawStrokeRectangle(Consts::STANDARD_SCREEN_WIDTH/2-7, Consts::STANDARD_SCREEN_HEIGHT/2-1, 14, 1, Color::BLACK);
    }

    if(_isControllerActive) {
        if(_selectedObject) {
            _objController->update();
        } else {
            _cameraController->update();
        }
    }

#ifndef NDEBUG
    auto debugRayCast = _world->rayCast(_camera->position(), _camera->position() + _camera->lookAt(), {_redCube->name()});
    _objInFocus = debugRayCast.intersected;
    _redCube->setVisible(_objInFocus);
    if(_objInFocus) {
        _redCube->translateToPoint(debugRayCast.pointOfIntersection);
    }
#endif

    if(_isControllerActive) {
        // select object:
        if (Keyboard::isKeyTapped(SDLK_o) || Mouse::isButtonTapped(SDL_BUTTON_LEFT)) {
            auto rayCast = _world->rayCast(_camera->position(), _camera->position() + _camera->lookAt(), {_redCube->name()});

            if(rayCast.intersected) {
                _selectedObject = rayCast.obj;
                _objController = std::make_shared<ObjectController>(_selectedObject);
                Log::log("Object " + rayCast.obj->name().str() + " selected.");
            }
        }

        // deselect object:
        if (Keyboard::isKeyTapped(SDLK_p)) {
            _selectedObject.reset();
        }
    }

    if(Keyboard::isKeyTapped(SDLK_F1)) {
        _screen->makeScreenShot().save2png(FilePath("screenshot_" + Time::getLocalTimeInfo("%F_%H-%M-%S") + ".png"));
    }

    if(Keyboard::isKeyTapped(SDLK_F2)) {
        if(!_isRecording) {
            _screen->startRender();
        } else{
            _screen->stopRender();
        }
        _isRecording = !_isRecording;
    }

    auto meshedSelectedObj = std::dynamic_pointer_cast<TriangleMesh>(_selectedObject);
    if(meshedSelectedObj) {
        _selectedObjectBounds->setVisible(true);
        _selectedObjectBounds->undoTransformations();
        auto Mbounds = meshedSelectedObj->bounds()*_selectedObject->fullModel();
        _selectedObjectBounds->scale(Mbounds.extents*2 + Vec3D::EPS());
        _selectedObjectBounds->translate(Mbounds.center);
    } else {
        _selectedObjectBounds->setVisible(false);
    }

    if(Keyboard::isKeyTapped(SDLK_q)) {
        if(!_isControllerActive && _selectedObject) {
            _objController = std::make_shared<ObjectController>(_selectedObject);
        }

        _isControllerActive = !_isControllerActive;

        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(_isControllerActive));
        SDL_ShowCursor(!_isControllerActive);
    }

    if (_selectedObject && _isControllerActive) {
        // object scale x:
        if (Keyboard::isKeyPressed(SDLK_UP)) {
            _selectedObject->scaleInside(Vec3D(1 + Time::deltaTime(), 1, 1));
        }
        // object scale y:
        if (Keyboard::isKeyPressed(SDLK_DOWN)) {
            _selectedObject->scaleInside(Vec3D(1, 1 + Time::deltaTime(), 1));
        }
        // object scale z:
        if (Keyboard::isKeyPressed(SDLK_LEFT)) {
            _selectedObject->scaleInside(Vec3D(1, 1, 1 + Time::deltaTime()));
        }

        // undo transformations
        if (Keyboard::isKeyPressed(SDLK_u)) {
            _selectedObject->undoTransformations();
        }

        // delete object
        if (Keyboard::isKeyPressed(SDLK_DELETE)) {
            _world->remove(_selectedObject->name());
            _selectedObject.reset();
        }
    }
}

void WorldEditor::update() {

    if(!_isControllerActive) {
        handleInputEvents();
    }

    processFrame();

    updateScreenSettings();
    updateControllers();

    renderGui();
}