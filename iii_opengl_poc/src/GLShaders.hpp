#pragma once

static const char* LIT_VERT_SRC = R"(
#version 410
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMat;
out vec3 vNormal;
out vec3 vWorldPos;
void main(){
  vec4 wp = uModel * vec4(aPos,1.0);
  vWorldPos = wp.xyz;
  vNormal = normalize(uNormalMat * aNormal);
  gl_Position = uMVP * vec4(aPos,1.0);
}
)";

static const char* LIT_FRAG_SRC = R"(
#version 410
in vec3 vNormal;
in vec3 vWorldPos;
uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uViewPos;
out vec4 FragColor;
void main(){
  vec3 N = gl_FrontFacing ? normalize(vNormal) : -normalize(vNormal);
  vec3 L = normalize(uLightDir);
  float diff = max(dot(N,L),0.0);
  vec3 V = normalize(uViewPos - vWorldPos);
  vec3 H = normalize(L+V);
  float spec = pow(max(dot(N,H),0.0),32.0);
  vec3 amb = 0.2*uColor;
  vec3 result = amb + diff*0.7*uColor + spec*0.25*vec3(1.0);
  FragColor = vec4(result,1.0);
}
)";

static const char* FLAT_VERT_SRC = R"(
#version 410
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main(){
  gl_Position = uMVP * vec4(aPos,1.0);
  gl_PointSize = 8.0;
}
)";

static const char* FLAT_FRAG_SRC = R"(
#version 410
uniform vec3 uColor;
out vec4 FragColor;
void main(){ FragColor = vec4(uColor,1.0); }
)";
