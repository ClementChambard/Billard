#version 130
precision mediump float;

attribute vec3 vPosition; //Depending who compiles, these variables are not "attribute" but "in". In this version (130) both are accepted. in should be used later

void main()
{
	gl_Position = vec4(vPosition,1.0);
}
