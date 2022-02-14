#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "Device.h"

extern int RENDER_MODE;// ���ӭn�令enum�Ӫ��
extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;
extern float gl_zFar;
extern float gl_zNear;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
void lookat(Vec3f pos, Vec3f target, Vec3f up);

struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0; //iface:��i�Ӥ��T����, nthvert:�o���T���β�n�ӳ��I
    virtual bool fragment(Vec3f bar, UI32 &color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, Device &device);
void triangle2(Vec4f* pts, IShader& shader, Device& device);
