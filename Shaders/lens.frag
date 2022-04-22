#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used

varying vec2 varyUVs;
uniform sampler2D uTex;
uniform float uAlpha;

void main()
{
    vec4 col = texture(uTex, varyUVs);
    col.a *= uAlpha;
    gl_FragColor = col;
}
