#version 130
precision mediump float;

attribute vec3 vPosition; //Depending who compiles, these variables are not "attribute" but "in". In this version (130) both are accepted. in should be used later
attribute vec3 vNormal;
attribute vec2 vUVs;

varying vec3 varyNormal; //Depending who compiles, these variables are not "varying" but "out". In this version (130) both are accepted. out should be used later
varying vec2 varyUVs;
varying vec3 varyCamSpacePos;

uniform mat4 uMV;
uniform mat4 uP;
uniform mat4 uV;


//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

void main()
{
    //We need to put vPosition as a vec4. Because vPosition is a vec3, we need one more value (w) which is here 1.0. Hence x and y go from -w to w hence -1 to +1. Premultiply this variable if you want to transform the position.
    vec4 cameraCoordsPos = uMV * vec4(vPosition, 1.0);
	gl_Position = uP * cameraCoordsPos;
    varyCamSpacePos = cameraCoordsPos.xyz;
    varyNormal = normalize((uMV * vec4(vNormal, 0.0)).xyz);
    varyUVs = vUVs;
}
