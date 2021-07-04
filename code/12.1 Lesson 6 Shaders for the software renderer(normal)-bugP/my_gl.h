#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "Device.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, UI32 &color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, Device &device);
void triangle2(Vec4f* pts, IShader& shader, Device& device);
