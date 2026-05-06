#include "GLRenderer.hpp"
#include "GLShaders.hpp"
#include "GLMeshes.hpp"
#include <iii/Math.hpp>
#include <iii/Vector.hpp>
#include <cmath>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <variant>

// --- GPUMesh cleanup ---
void GPUMesh::cleanup() {
  if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
  if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
  if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
  indexCount = 0;
}

// --- Constructor / Destructor ---
GLRenderer::GLRenderer() {}
GLRenderer::~GLRenderer() {}

// --- Shader helpers ---
GLuint GLRenderer::compileShader(GLenum type, const char *src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
    std::cerr << "Shader compile error:\n" << log << std::endl;
  }
  return s;
}

GLuint GLRenderer::linkProgram(GLuint vert, GLuint frag) {
  GLuint p = glCreateProgram();
  glAttachShader(p, vert); glAttachShader(p, frag);
  glLinkProgram(p);
  GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[512]; glGetProgramInfoLog(p, 512, nullptr, log);
    std::cerr << "Program link error:\n" << log << std::endl;
  }
  glDeleteShader(vert); glDeleteShader(frag);
  return p;
}

// --- Init / Shutdown ---
void GLRenderer::initGL() {
  m_litProgram = linkProgram(
    compileShader(GL_VERTEX_SHADER, LIT_VERT_SRC),
    compileShader(GL_FRAGMENT_SHADER, LIT_FRAG_SRC));
  m_litLocMVP = glGetUniformLocation(m_litProgram, "uMVP");
  m_litLocModel = glGetUniformLocation(m_litProgram, "uModel");
  m_litLocNormalMat = glGetUniformLocation(m_litProgram, "uNormalMat");
  m_litLocColor = glGetUniformLocation(m_litProgram, "uColor");
  m_litLocLightDir = glGetUniformLocation(m_litProgram, "uLightDir");
  m_litLocViewPos = glGetUniformLocation(m_litProgram, "uViewPos");

  m_flatProgram = linkProgram(
    compileShader(GL_VERTEX_SHADER, FLAT_VERT_SRC),
    compileShader(GL_FRAGMENT_SHADER, FLAT_FRAG_SRC));
  m_flatLocMVP = glGetUniformLocation(m_flatProgram, "uMVP");
  m_flatLocColor = glGetUniformLocation(m_flatProgram, "uColor");

  // Dynamic VAO/VBO for points and lines
  glGenVertexArrays(1, &m_dynVAO);
  glGenBuffers(1, &m_dynVBO);
  glBindVertexArray(m_dynVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_dynVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void GLRenderer::shutdownGL() {
  if (m_litProgram) glDeleteProgram(m_litProgram);
  if (m_flatProgram) glDeleteProgram(m_flatProgram);
  if (m_dynVAO) glDeleteVertexArrays(1, &m_dynVAO);
  if (m_dynVBO) glDeleteBuffers(1, &m_dynVBO);
  for (auto &[k,m] : m_sphereCache) m.cleanup();
  for (auto &[k,m] : m_cylinderCache) m.cleanup();
  for (auto &[k,m] : m_coneCache) m.cleanup();
  for (auto &[k,m] : m_discCache) m.cleanup();
}

// --- Mesh cache ---
GPUMesh &GLRenderer::getOrCreateSphere(int t) {
  auto it = m_sphereCache.find(t);
  if (it != m_sphereCache.end()) return it->second;
  GPUMesh &m = m_sphereCache[t];
  auto d = generateSphereMesh(t);
  uploadMesh(d, m.vao, m.vbo, m.ebo, m.indexCount);
  return m;
}
GPUMesh &GLRenderer::getOrCreateCylinder(int t) {
  auto it = m_cylinderCache.find(t);
  if (it != m_cylinderCache.end()) return it->second;
  GPUMesh &m = m_cylinderCache[t];
  auto d = generateCylinderMesh(t);
  uploadMesh(d, m.vao, m.vbo, m.ebo, m.indexCount);
  return m;
}
GPUMesh &GLRenderer::getOrCreateCone(int t) {
  auto it = m_coneCache.find(t);
  if (it != m_coneCache.end()) return it->second;
  GPUMesh &m = m_coneCache[t];
  auto d = generateConeMesh(t);
  uploadMesh(d, m.vao, m.vbo, m.ebo, m.indexCount);
  return m;
}
GPUMesh &GLRenderer::getOrCreateDisc(int t) {
  auto it = m_discCache.find(t);
  if (it != m_discCache.end()) return it->second;
  GPUMesh &m = m_discCache[t];
  auto d = generateDiscMesh(t);
  uploadMesh(d, m.vao, m.vbo, m.ebo, m.indexCount);
  return m;
}

// --- Matrix math ---
void GLRenderer::mat4Mul(const float *a, const float *b, float *o) {
  for (int c = 0; c < 4; ++c)
    for (int r = 0; r < 4; ++r)
      o[c*4+r] = a[0*4+r]*b[c*4+0] + a[1*4+r]*b[c*4+1] + a[2*4+r]*b[c*4+2] + a[3*4+r]*b[c*4+3];
}

void GLRenderer::normalMat3(const float *m, float *n) {
  // Upper-left 3x3 inverse transpose (for uniform scale, just use upper-left 3x3)
  n[0]=m[0]; n[1]=m[1]; n[2]=m[2];
  n[3]=m[4]; n[4]=m[5]; n[5]=m[6];
  n[6]=m[8]; n[7]=m[9]; n[8]=m[10];
}

void GLRenderer::buildSegmentMatrix(float x1, float y1, float z1,
                                     float x2, float y2, float z2,
                                     float radius, float mat[16]) {
  float dx = x2-x1, dy = y2-y1, dz = z2-z1;
  float len = sqrtf(dx*dx+dy*dy+dz*dz);
  if (len < 1e-8f) { memset(mat, 0, 64); mat[0]=mat[5]=mat[10]=mat[15]=1; return; }
  float ndx=dx/len, ndy=dy/len, ndz=dz/len;
  // Build frame: Y axis = segment direction
  float ux, uy, uz, vx, vy, vz;
  float ax=0,ay=0,az=1;
  if (fabsf(ndy) < 0.9f) { ax=0; ay=1; az=0; } // pick up if not parallel
  // actually let's pick something not parallel to (ndx,ndy,ndz)
  if (fabsf(ndy) > 0.9f) { ax=1; ay=0; az=0; } else { ax=0; ay=1; az=0; }
  // u = cross(dir, arbitrary)
  ux = ndy*az - ndz*ay; uy = ndz*ax - ndx*az; uz = ndx*ay - ndy*ax;
  float ul = sqrtf(ux*ux+uy*uy+uz*uz);
  if (ul > 1e-8f) { ux/=ul; uy/=ul; uz/=ul; }
  // v = cross(dir, u)
  vx = ndy*uz - ndz*uy; vy = ndz*ux - ndx*uz; vz = ndx*uy - ndy*ux;
  // Column-major: col0=U*radius, col1=dir*len, col2=V*radius, col3=origin
  mat[0] = ux*radius;  mat[1] = uy*radius;  mat[2] = uz*radius;  mat[3] = 0;
  mat[4] = ndx*len;    mat[5] = ndy*len;    mat[6] = ndz*len;    mat[7] = 0;
  mat[8] = vx*radius;  mat[9] = vy*radius;  mat[10]= vz*radius;  mat[11]= 0;
  mat[12]= x1;         mat[13]= y1;         mat[14]= z1;         mat[15]= 1;
}

// --- Event handling (unchanged logic) ---
void GLRenderer::onEvent(const iii::Event &e) {
  std::visit([&](auto &&arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, iii::EventCreate>) {
      m_objects.push_back({arg.id, 0,0,0, arg.color.r, arg.color.g, arg.color.b, arg.label});
    } else if constexpr (std::is_same_v<T, iii::EventCreateRelation>) {
      RenderObject obj; obj.id=arg.id; obj.start_id=arg.start_id; obj.end_id=arg.end_id;
      obj.r=arg.color.r; obj.g=arg.color.g; obj.b=arg.color.b; obj.semantic="Line";
      m_objects.push_back(obj);
    } else if constexpr (std::is_same_v<T, iii::EventMove>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.x=(float)arg.x; o.y=(float)arg.y; o.z=(float)arg.z; break; }
    } else if constexpr (std::is_same_v<T, iii::EventDestroy>) {
      std::erase_if(m_objects, [&](const auto &o){ return o.id==arg.id; });
    } else if constexpr (std::is_same_v<T, iii::EventSetSemantic>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.semantic=arg.semantic; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetOrigin>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.origin_id=arg.origin_id; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetVisible>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.visible=arg.visible; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetColor>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.r=arg.color.r; o.g=arg.color.g; o.b=arg.color.b; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetMatrix>) {
      for (auto &o : m_objects) if (o.id==arg.id) { for(int i=0;i<16;++i) o.matrix[i]=arg.m[i]; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetLabel>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.label=arg.label; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetClass>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.className=arg.className; break; }
    } else if constexpr (std::is_same_v<T, iii::EventSetLayer>) {
      for (auto &o : m_objects) if (o.id==arg.id) { o.layer=arg.layer; break; }
    }
  }, e);
}

void GLRenderer::resetObjects() { m_objects.clear(); }

bool GLRenderer::isLayerVisible(const RenderObject &obj,
                                const std::map<std::string, bool> &lv) const {
  if (obj.layer.empty()) return true;
  auto it = lv.find(obj.layer);
  return (it == lv.end()) ? true : it->second;
}

// --- Main render ---
void GLRenderer::render(const float *viewMat, const float *projMat,
                        const float *camPos,
                        const std::map<std::string, bool> &layerVis,
                        const std::map<std::string, ClassStyle> &classStyles) {
  float vp[16]; mat4Mul(projMat, viewMat, vp);

  auto getStyle = [&](const RenderObject &obj) -> const ClassStyle* {
    if (obj.className.empty()) return nullptr;
    auto it = classStyles.find(obj.className);
    if (it == classStyles.end() || it->second.shape == ClassStyle::Default) return nullptr;
    return &it->second;
  };
  auto resolveColor = [](const RenderObject &obj, const ClassStyle *s, float &cr, float &cg, float &cb) {
    if (s && s->overrideColor) { cr=s->colorR; cg=s->colorG; cb=s->colorB; }
    else { cr=obj.r; cg=obj.g; cb=obj.b; }
  };
  auto resolveEndpoints = [&](const RenderObject &obj, float &x1,float &y1,float &z1, float &x2,float &y2,float &z2) -> bool {
    if (obj.semantic=="Vector") {
      x1=y1=z1=0;
      if (obj.origin_id) for (const auto &o:m_objects) if (o.id==obj.origin_id) { x1=o.x; y1=o.y; z1=o.z; break; }
      x2=x1+obj.x; y2=y1+obj.y; z2=z1+obj.z; return true;
    } else if (obj.semantic=="Line") {
      bool f1=false, f2=false;
      for (const auto &p:m_objects) {
        if (p.id==obj.start_id) { x1=p.x; y1=p.y; z1=p.z; f1=true; }
        if (p.id==obj.end_id) { x2=p.x; y2=p.y; z2=p.z; f2=true; }
      }
      return f1&&f2;
    }
    return false;
  };

  // Helper: draw a mesh with model matrix using lit shader
  auto drawLit = [&](GPUMesh &mesh, const float model[16], float cr, float cg, float cb, bool wire) {
    float mvp[16]; mat4Mul(vp, model, mvp);
    float nm[9]; normalMat3(model, nm);
    glUniformMatrix4fv(m_litLocMVP, 1, GL_FALSE, mvp);
    glUniformMatrix4fv(m_litLocModel, 1, GL_FALSE, model);
    glUniformMatrix3fv(m_litLocNormalMat, 1, GL_FALSE, nm);
    glUniform3f(m_litLocColor, cr, cg, cb);
    glBindVertexArray(mesh.vao);
    if (wire) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    if (wire) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  };

  // === Pass 1: Default points ===
  {
    std::vector<float> pts;
    std::vector<float> colors; // stored separately, drawn per-point
    for (const auto &obj : m_objects) {
      if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
      if (obj.semantic != "Point") continue;
      if (getStyle(obj)) continue;
      pts.insert(pts.end(), {obj.x, obj.y, obj.z});
      colors.insert(colors.end(), {obj.r, obj.g, obj.b});
    }
    if (!pts.empty()) {
      glUseProgram(m_flatProgram);
      glEnable(GL_PROGRAM_POINT_SIZE);
      glBindVertexArray(m_dynVAO);
      glBindBuffer(GL_ARRAY_BUFFER, m_dynVBO);
      // Draw each point individually (flat shader has uniform color)
      glBufferData(GL_ARRAY_BUFFER, pts.size()*sizeof(float), pts.data(), GL_DYNAMIC_DRAW);
      // Draw one by one for different colors
      for (size_t i = 0; i < pts.size()/3; ++i) {
        float mvp[16]; mat4Mul(projMat, viewMat, mvp); // identity model
        glUniformMatrix4fv(m_flatLocMVP, 1, GL_FALSE, vp);
        glUniform3f(m_flatLocColor, colors[i*3], colors[i*3+1], colors[i*3+2]);
        glDrawArrays(GL_POINTS, (GLint)i, 1);
      }
    }
  }

  // === Pass 2: Styled points (Sphere) — lit ===
  glUseProgram(m_litProgram);
  glUniform3f(m_litLocLightDir, 0.4f, 0.8f, 0.5f);
  glUniform3f(m_litLocViewPos, camPos[0], camPos[1], camPos[2]);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic != "Point") continue;
    auto *s = getStyle(obj);
    if (!s || s->shape != ClassStyle::Sphere) continue;
    float cr,cg,cb; resolveColor(obj, s, cr, cg, cb);
    float model[16]; memset(model,0,64);
    model[0]=s->radius; model[5]=s->radius; model[10]=s->radius; model[15]=1;
    model[12]=obj.x; model[13]=obj.y; model[14]=obj.z;
    auto &mesh = getOrCreateSphere(s->tessellation);
    drawLit(mesh, model, cr, cg, cb, s->wireframe);
  }

  // === Pass 3: Default lines ===
  {
    glUseProgram(m_flatProgram);
    glBindVertexArray(m_dynVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_dynVBO);
    for (const auto &obj : m_objects) {
      if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
      if (obj.semantic != "Vector" && obj.semantic != "Line") continue;
      if (getStyle(obj)) continue;
      float x1,y1,z1,x2,y2,z2;
      if (!resolveEndpoints(obj, x1,y1,z1, x2,y2,z2)) continue;
      float verts[] = {x1,y1,z1, x2,y2,z2};
      glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
      glUniformMatrix4fv(m_flatLocMVP, 1, GL_FALSE, vp);
      glUniform3f(m_flatLocColor, obj.r, obj.g, obj.b);
      glDrawArrays(GL_LINES, 0, 2);
    }
  }

  // === Pass 4: Styled lines/vectors (Cylinder, Arrow) — lit ===
  glUseProgram(m_litProgram);
  glUniform3f(m_litLocLightDir, 0.4f, 0.8f, 0.5f);
  glUniform3f(m_litLocViewPos, camPos[0], camPos[1], camPos[2]);
  for (const auto &obj : m_objects) {
    if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
    if (obj.semantic != "Vector" && obj.semantic != "Line") continue;
    auto *s = getStyle(obj); if (!s) continue;
    float x1,y1,z1,x2,y2,z2;
    if (!resolveEndpoints(obj, x1,y1,z1, x2,y2,z2)) continue;
    float cr,cg,cb; resolveColor(obj, s, cr, cg, cb);

    if (s->shape == ClassStyle::Cylinder) {
      float model[16];
      buildSegmentMatrix(x1,y1,z1, x2,y2,z2, s->cylinderRadius, model);
      auto &mesh = getOrCreateCylinder(s->tessellation);
      drawLit(mesh, model, cr, cg, cb, s->wireframe);
      if (s->endcaps) {
        float dx=x2-x1, dy=y2-y1, dz=z2-z1;
        float l=sqrtf(dx*dx+dy*dy+dz*dz);
        if (l > 1e-8f) {
          // Bottom disc
          float dm[16]; buildSegmentMatrix(x1,y1,z1, x1-dx/l*0.001f,y1-dy/l*0.001f,z1-dz/l*0.001f, s->cylinderRadius, dm);
          dm[4]=dx/l*0.001f; dm[5]=dy/l*0.001f; dm[6]=dz/l*0.001f; // tiny Y
          auto &disc = getOrCreateDisc(s->tessellation);
          drawLit(disc, dm, cr, cg, cb, false);
          // Top disc
          buildSegmentMatrix(x2,y2,z2, x2+dx/l*0.001f,y2+dy/l*0.001f,z2+dz/l*0.001f, s->cylinderRadius, dm);
          dm[4]=dx/l*0.001f; dm[5]=dy/l*0.001f; dm[6]=dz/l*0.001f;
          drawLit(disc, dm, cr, cg, cb, false);
        }
      }
    } else if (s->shape == ClassStyle::Arrow) {
      float dx=x2-x1, dy=y2-y1, dz=z2-z1;
      float len=sqrtf(dx*dx+dy*dy+dz*dz);
      if (len < 1e-8f) continue;
      float ndx=dx/len, ndy=dy/len, ndz=dz/len;
      float bodyLen = len - s->coneLength;
      if (bodyLen < 0) bodyLen = len * 0.5f;
      float bx2=x1+ndx*bodyLen, by2=y1+ndy*bodyLen, bz2=z1+ndz*bodyLen;
      // Shaft
      float model[16];
      buildSegmentMatrix(x1,y1,z1, bx2,by2,bz2, s->cylinderRadius, model);
      drawLit(getOrCreateCylinder(s->tessellation), model, cr, cg, cb, s->wireframe);
      // Cone head
      buildSegmentMatrix(bx2,by2,bz2, x2,y2,z2, s->coneRadius, model);
      drawLit(getOrCreateCone(s->tessellation), model, cr, cg, cb, s->wireframe);
      // Base disc at cone
      float dm[16]; buildSegmentMatrix(bx2,by2,bz2, bx2-ndx*0.001f,by2-ndy*0.001f,bz2-ndz*0.001f, s->coneRadius, dm);
      dm[4]=-ndx*0.001f; dm[5]=-ndy*0.001f; dm[6]=-ndz*0.001f;
      drawLit(getOrCreateDisc(s->tessellation), dm, cr, cg, cb, false);
      if (s->endcaps) {
        buildSegmentMatrix(x1,y1,z1, x1-ndx*0.001f,y1-ndy*0.001f,z1-ndz*0.001f, s->cylinderRadius, dm);
        dm[4]=-ndx*0.001f; dm[5]=-ndy*0.001f; dm[6]=-ndz*0.001f;
        drawLit(getOrCreateDisc(s->tessellation), dm, cr, cg, cb, false);
      }
    }
  }

  // === Pass 5: Frames ===
  {
    glUseProgram(m_flatProgram);
    glBindVertexArray(m_dynVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_dynVBO);
    for (const auto &obj : m_objects) {
      if (!obj.visible || !isLayerVisible(obj, layerVis)) continue;
      if (obj.semantic != "Frame") continue;
      double tx=obj.matrix[12], ty=obj.matrix[13], tz=obj.matrix[14];
      float cols[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
      int axes[3] = {0, 4, 8};
      for (int a = 0; a < 3; ++a) {
        float v[6] = {(float)tx,(float)ty,(float)tz,
          (float)(tx+obj.matrix[axes[a]]),(float)(ty+obj.matrix[axes[a]+1]),(float)(tz+obj.matrix[axes[a]+2])};
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);
        glUniformMatrix4fv(m_flatLocMVP, 1, GL_FALSE, vp);
        glUniform3f(m_flatLocColor, cols[a][0], cols[a][1], cols[a][2]);
        glDrawArrays(GL_LINES, 0, 2);
      }
    }
  }
  glUseProgram(0);
}

// --- Queries ---
std::vector<std::string> GLRenderer::getLayers() const {
  std::vector<std::string> r;
  for (const auto &o : m_objects) {
    if (!o.layer.empty()) {
      bool f=false; for (const auto &l:r) if (l==o.layer) { f=true; break; }
      if (!f) r.push_back(o.layer);
    }
  }
  return r;
}
std::vector<std::string> GLRenderer::getClasses() const {
  std::vector<std::string> r;
  for (const auto &o : m_objects) {
    if (!o.className.empty()) {
      bool f=false; for (const auto &c:r) if (c==o.className) { f=true; break; }
      if (!f) r.push_back(o.className);
    }
  }
  return r;
}
