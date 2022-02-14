#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "Device.h"

extern int RENDER_MODE;// 應該要改成enum來表示
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
    virtual Vec4f vertex(int iface, int nthvert) = 0; //iface:第i個片三角形, nthvert:這片三角形第n個頂點
    virtual bool fragment(Vec3f bar, UI32 &color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, Device &device);
void triangle2(Vec4f* pts, IShader& shader, Device& device);
