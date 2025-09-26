#version 450 core
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec4 vPosition;
layout(std140, binding = 0) uniform mvpMatrix {     //binding = 0=> binding point
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} u_mvp;



void main(void)
{
    //code
    gl_Position = u_mvp.projectionMatrix * u_mvp.viewMatrix * u_mvp.modelMatrix * vPosition;
}

