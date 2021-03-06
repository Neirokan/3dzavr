//
// Created by Иван Ильин on 13.01.2021.
//

#include "Triangle.h"

Triangle::Triangle () {
    p[0] = {0,0,0,1};
    p[1] = {0,0,0,1};
    p[2] = {0,0,0,1};
}

Triangle::Triangle(const Point4D& p1, const Point4D& p2, const Point4D& p3) {
    p[0] = p1;
    p[1] = p2;
    p[2] = p3;
}

Triangle Triangle::operator*(const Matrix4x4 &matrix4X4) const {
    return Triangle(*this) *= matrix4X4;
}

Triangle &Triangle::operator*=(const Matrix4x4 &matrix4X4) {
    p[0] = matrix4X4 * p[0];
    p[1] = matrix4X4 * p[1];
    p[2] = matrix4X4 * p[2];

    return *this;
}

Point4D Triangle::norm() const {

    Point4D v1 = p[1] - p[0];
    Point4D v2 = p[2] - p[0];

    return v1.cross3D(v2).normalize();
}

Point4D Triangle::operator[](int i) const {
    return p[i];
}

Point4D &Triangle::operator[](int i) {
    return p[i];
}

Point4D Triangle::pos() const {
    return (p[0] + p[1] + p[2])/3.0;
}

Triangle::Triangle(const Triangle &triangle) {
    clip = triangle.clip;
    color = triangle.color;
    p[0] = triangle[0];
    p[1] = triangle[1];
    p[2] = triangle[2];
}
