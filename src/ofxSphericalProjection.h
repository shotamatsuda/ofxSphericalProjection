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

#pragma once

#include <string>
#include <unordered_map>

#include "ofConstants.h"
#include "ofFbo.h"
#include "ofGraphics.h"
#include "ofRectangle.h"
#include "ofShader.h"
#include "ofVectorMath.h"

#include "ofxSphericalProjectionFace.h"
#include "ofxSphericalProjectionSetting.h"

namespace ofxsphericalprojection {

class Projection final {
 public:
  using Face = Face;
  using Settings = Settings;

 public:
  Projection();

  // Disallow copy semantics
  Projection(const Projection&) = delete;
  Projection& operator=(const Projection&) = delete;

  // Move semantics
  Projection(Projection&&) = default;
  Projection& operator=(Projection&&) = default;

  // Modifier
  void setup(const Settings& settings);
  void bind() const;
  void unbind() const;

  // Drawing
  template <class Function>
  void render(Function callback);
  void begin(Face face);
  void end();
  void draw(double x, double y, double width, double height) const;
  void draw(const ofRectangle& rect) const;
  void draw(const ofPoint& point, double width, double height) const;

  // Matrices
  const ofMatrix4x4& getPerspectiveMatrix() const;
  const ofMatrix4x4& getLookAtMatrix(Face face) const;

  // Parameters
  double getTop() const;
  void setTop(double value);
  double getRight() const;
  void setRight(double value);
  double getBottom() const;
  void setBottom(double value);
  double getLeft() const;
  void setLeft(double value);

  // Shader sources
  std::string getVertexShaderSource() const;
  std::string getFragmentShaderSource() const;

 private:
  // Parameters
  int size;
  GLuint internalFormat;
  GLuint format;
  GLuint type;
  double near;
  double far;
  double top;
  double right;
  double bottom;
  double left;

  // Compositions
  GLuint texture;
  ofFbo fbo;
  ofShader shader;

  // Internal states
  bool needsUpdatePerspectiveMatrix;
  ofMatrix4x4 perspectiveMatrix;
  std::unordered_map<GLuint, ofMatrix4x4> lookAtMatrices;

  // Shader sources
  static const char *vertexShaderSource;
  static const char *fragmentShaderSource;
};

// MARK: -

template <class Function>
inline void Projection::render(Function callback) {
  for (const auto& pair : lookAtMatrices) {
    const auto face = pair.first;
    const auto& result = callback(static_cast<Face>(face));
    fbo.begin();
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        face, texture, 0);
    result.draw(ofPoint());
    fbo.end();
  }
}

// MARK: Parameters

inline double Projection::getTop() const {
  return top;
}

inline void Projection::setTop(double value) {
  top = value;
}

inline double Projection::getRight() const {
  return right;
}

inline void Projection::setRight(double value) {
  right = value;
}

inline double Projection::getBottom() const {
  return bottom;
}

inline void Projection::setBottom(double value) {
  bottom = value;
}

inline double Projection::getLeft() const {
  return left;
}

inline void Projection::setLeft(double value) {
  left = value;
}

}  // namespace ofxsphericalprojection

using ofxSphericalProjection = ofxsphericalprojection::Projection;
