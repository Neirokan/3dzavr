#include <cmath>

#include "Texture.h"
#include <utils/math.h>

#include <iostream>

Texture::Texture(const FilePath &filename) {
    _texture.emplace_back(filename);
    while (_texture.back().width() * _texture.back().height() != 1) {
        _texture.emplace_back(_texture.back().downSampled());
    }

    //Check does the texture have the transparent pixels
    for(int x = 0; x < width(); x++) {
        for(int y = 0; y < height(); y++) {
            auto pix = get_pixel(x, y);
            if(pix.a() != 255) {
                _isTransparent = true;
                return;
            }
        }
    }
}

Color Texture::get_pixel(uint16_t x, uint16_t y) const {
    return _texture.front().get_pixel(x, y);
}

Color Texture::get_pixel_from_UV(const Vec2D &uv) const {
    return _texture.front().get_pixel_from_UV(uv);
}

const Image& Texture::get_sample(double area) const {
    uint64_t limit = 1ULL << (_texture.size() - 1);
    uint16_t K;

    if (area < 2) {
        K = 0;
    } else if (area < limit) {
        K = static_cast<uint16_t>(log2_u64(area)) - 1;
    } else {
        K = _texture.size() - 1;
    }
    return _texture[K];
}

Color Texture::get_pixel_from_UV(const Vec2D &uv, double area) const {
    return get_sample(area).get_pixel_from_UV(uv);
}
