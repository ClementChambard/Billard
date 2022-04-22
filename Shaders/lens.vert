#version 130
precision mediump float;

attribute vec2 vPosition; //Depending who compiles, these variables are not "attribute" but "in". In this version (130) both are accepted. in should be used later
attribute vec2 vUVs;

varying vec2 varyUVs;

void main()
{
	gl_Position = vec4(vPosition,0.5,1.0);
    varyUVs = vUVs;
}
