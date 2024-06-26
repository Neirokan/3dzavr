#ifndef LINALG_MATRIX4X4_H
#define LINALG_MATRIX4X4_H

#include <array>
#include <iostream>

#include <linalg/Vec3D.h>

class Matrix4x4 final {
private:
    std::array<std::array<double, 4>, 4> _arr{};

    [[nodiscard]] bool arePerpendicularAxis() const;
public:
    Matrix4x4() = default;
    explicit Matrix4x4(const std::array<std::array<double, 4>, 4>& matrix) : _arr(matrix) {};

    Matrix4x4 &operator=(const Matrix4x4 &matrix4X4) = default;

    [[nodiscard]] Matrix4x4 operator*(const Matrix4x4 &matrix4X4) const;
    [[nodiscard]] Matrix4x4 operator+(const Matrix4x4 &matrix4X4) const;
    [[nodiscard]] Matrix4x4 operator-(const Matrix4x4 &matrix4X4) const;

    [[nodiscard]] Vec4D operator*(const Vec4D &point4D) const;
    [[nodiscard]] Vec3D operator*(const Vec3D &vec) const;

    friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& matrix4x4);

    [[nodiscard]] Vec3D x() const;
    [[nodiscard]] Vec3D y() const;
    [[nodiscard]] Vec3D z() const;
    [[nodiscard]] Vec3D w() const;

    /*
     * This function is computationally inefficient.
     * It is better to use View() method for 3D applications, where you do not need the inverse of 4x4 matrix,
     * but the inverse of 3x3 matrix + inverse translation.
     */
    [[nodiscard]] Matrix4x4 inverse() const;

    [[nodiscard]] double abs() const;

    // Any useful matrix (static methods)
    Matrix4x4 static Identity();
    Matrix4x4 static Zero();
    Matrix4x4 static Constant(double value);
    Matrix4x4 static Scale(const Vec3D &factor);
    Matrix4x4 static Translation(const Vec3D &v);
    Matrix4x4 static Rotation(const Vec3D &r);
    Matrix4x4 static RotationX(double rx);
    Matrix4x4 static RotationY(double ry);
    Matrix4x4 static RotationZ(double rz);
    Matrix4x4 static Rotation(const Vec3D &v, double rv);
    Matrix4x4 static View(const Matrix4x4 &transformMatrix);
    Matrix4x4 static Projection(double fov = 90.0, double aspect = 1.0, double ZNear = 1.0, double ZFar = 10.0);
    Matrix4x4 static ScreenSpace(uint16_t width, uint16_t height, int shiftX=0, int shiftY=0);
};

#include "Matrix4x4.inl"

#endif //LINALG_MATRIX4X4_H
