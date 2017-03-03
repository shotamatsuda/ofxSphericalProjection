//
//  The MIT License
//
//  Copyright (C) 2017 Shota Matsuda
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#include "ofxSphericalProjection.h"

#include <string>

#include "ofConstants.h"
#include "ofFbo.h"
#include "ofRectangle.h"
#include "ofVectorMath.h"

#ifndef STRINGIFY
#define STRINGIFY(A) #A
#endif

namespace ofxsphericalprojection {

Projection::Projection()
  : size(),
    internalFormat(),
    format(),
    type(),
    near(),
    far(),
    top(),
    right(),
    bottom(),
    left(),
    texture(),
    needsUpdatePerspectiveMatrix(),
    lookAtMatrices({
      { static_cast<GLuint>(Face::POSITIVE_X), ofMatrix4x4() },
      { static_cast<GLuint>(Face::NEGATIVE_X), ofMatrix4x4() },
      { static_cast<GLuint>(Face::POSITIVE_Y), ofMatrix4x4() },
      { static_cast<GLuint>(Face::NEGATIVE_Y), ofMatrix4x4() },
      { static_cast<GLuint>(Face::POSITIVE_Z), ofMatrix4x4() },
      { static_cast<GLuint>(Face::NEGATIVE_Z), ofMatrix4x4() },
    }) {
  for (auto& pair : lookAtMatrices) {
    auto& matrix = pair.second;
    switch (pair.first) {
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        matrix.makeLookAtMatrix({}, { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 });
        break;
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        matrix.makeLookAtMatrix({}, { -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 });
        break;
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        matrix.makeLookAtMatrix({}, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 });
        break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        matrix.makeLookAtMatrix({}, { 0.0, -1.0, 0.0 }, { 0.0, 0.0, -1.0 });
        break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        matrix.makeLookAtMatrix({}, { 0.0, 0.0, 1.0 }, { 0.0, -1.0, 0.0 });
        break;
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        matrix.makeLookAtMatrix({}, { 0.0, 0.0, -1.0 }, { 0.0, -1.0, 0.0 });
        break;
      default:
        assert(false);
        break;
    }
  }
}

// MARK: Modifier

void Projection::setup(const Settings& settings) {
  size = settings.size;
  internalFormat = settings.internalFormat;
  format = settings.format;
  type = settings.type;
  near = settings.near;
  far = settings.far;
  top = settings.top;
  right = settings.right;
  bottom = settings.bottom;
  left = settings.left;

  // Projection
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // Textures
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
               internalFormat, size, size, 0,
               format, type, nullptr);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,
               internalFormat, size, size, 0,
               format, type, nullptr);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0,
               internalFormat, size, size, 0,
               format, type, nullptr);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0,
               internalFormat, size, size, 0,
               format, type, nullptr);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0,
               internalFormat, size, size, 0,
               format, type, nullptr);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0,
               internalFormat, size, size, 0,
               format, type, nullptr);

  // Framebuffer
  ofFbo::Settings fboSettings;
  fboSettings.width = size;
  fboSettings.height = size;
  fboSettings.numColorbuffers = 6;
  fboSettings.useDepth = true;
  fboSettings.textureTarget = GL_TEXTURE_2D;
  fbo.allocate(fboSettings);

  // Shader
  shader.setupShaderFromSource(GL_VERTEX_SHADER, getVertexShaderSource());
  shader.setupShaderFromSource(GL_FRAGMENT_SHADER, getFragmentShaderSource());
  shader.bindDefaults();
  shader.linkProgram();

  // Perspective matrix
  perspectiveMatrix.makePerspectiveMatrix(90.0, 1.0, near, far);
}

void Projection::begin(Face face) {
  ofPushView();
  ofSetMatrixMode(OF_MATRIX_PROJECTION);
  ofLoadMatrix(getPerspectiveMatrix());
  ofSetMatrixMode(OF_MATRIX_MODELVIEW);
  ofLoadMatrix(getLookAtMatrix(face));
}

void Projection::end() {
  ofPopView();
}

// MARK: Drawing
void Projection::draw(double x, double y, double width, double height) const {
  bind();
  shader.begin();
  shader.setUniform1f("top", top);
  shader.setUniform1f("right", right);
  shader.setUniform1f("bottom", bottom);
  shader.setUniform1f("left", left);
  fbo.draw(x, y, width, height);
  shader.end();
  unbind();
}

void Projection::draw(const ofRectangle& rect) const {
  draw(rect.x, rect.y, rect.width, rect.height);
}

void Projection::draw(const ofPoint& point, double width, double height) const {
  draw(point.x, point.y, width, height);
}

// MARK: Projection binding

void Projection::bind() const {
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}

void Projection::unbind() const {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
}

// MARK: Matrices

const ofMatrix4x4& Projection::getPerspectiveMatrix() const {
  return perspectiveMatrix;
}

const ofMatrix4x4& Projection::getLookAtMatrix(Face face) const {
  return lookAtMatrices.at(static_cast<GLuint>(face));
}

// MARK: Shader sources

std::string Projection::getVertexShaderSource() const {
  const std::string version = "#version 330\n";
  return version + vertexShaderSource;
}

std::string Projection::getFragmentShaderSource() const {
  const std::string version = "#version 330\n";
  return version + fragmentShaderSource;
}

const char * Projection::vertexShaderSource = STRINGIFY(
  precision highp float;

  uniform mat4 viewMatrix;
  uniform mat4 modelViewMatrix;
  uniform mat4 projectionMatrix;
  uniform mat4 modelViewProjectionMatrix;
  uniform mat4 textureMatrix;

  in vec4 position;
  in vec2 texcoord;
  in vec4 normal;
  in vec4 color;

  out VertexAttrib {
    vec2 texcoord;
  } vertex;

  void main() {
    vertex.texcoord = texcoord;
    gl_Position = modelViewProjectionMatrix * position;
  }
);

const char * Projection::fragmentShaderSource = STRINGIFY(
  precision highp float;

  const float M_PI = 3.1415926535897932384626433832795;
  const float M_PI_2 = 1.5707963267948966192313216916398;

  uniform samplerCube environment;
  uniform float top;
  uniform float right;
  uniform float bottom;
  uniform float left;

  in VertexAttrib {
    vec2 texcoord;
  } vertex;

  out vec4 fragColor;

  void main() {
    float theta = mix(left, right, vertex.texcoord.x);
    float phi = mix(top, bottom, vertex.texcoord.y);
    vec3 ray = vec3(cos(phi) * cos(theta), sin(phi), cos(phi) * sin(theta));
    fragColor = texture(environment, ray);
  }
);

}  // namespace ofxsphericalprojection
