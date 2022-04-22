#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used

varying vec3 varyNormal; //Sometimes we use "out" instead of "varying". "out" should be used in later version of GLSL.
varying vec3 varyCamSpacePos;
varying vec2 varyUVs;

uniform vec3 uMatK;
uniform vec3 uMatCol;
uniform float uMatAlpha;
uniform vec3 uLigCol;
uniform vec3 uLigPos;

uniform sampler2D uTex;


//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

void main()
{
    vec3 matCol = texture(uTex, varyUVs).xyz * uMatCol;
    vec3 Ambiant = uMatK.x * matCol * uLigCol;
    vec3 ligDir = normalize(varyCamSpacePos - uLigPos);
    vec3 Diffuse = uMatK.y * max(0, dot(varyNormal, -ligDir)) * matCol * uLigCol;
    vec3 Specular = uMatK.z * pow((max(0, dot(reflect(ligDir, varyNormal), vec3(0,0,1)))),uMatAlpha) * uLigCol;
    gl_FragColor = vec4(Ambiant + Diffuse + Specular, 1.0);
}
