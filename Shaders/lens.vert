#version 130
precision mediump float;

attribute vec3 vPosition;
attribute vec2 vUV;

varying vec2 varyUVs;

void main()
{
	gl_Position = vec4(vPosition,1.0);
    varyUVs = vUV;
}
