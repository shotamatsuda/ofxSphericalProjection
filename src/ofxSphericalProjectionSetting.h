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

namespace ofxsphericalprojection {

struct Settings {
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
  Settings();
};

// Comparison
bool operator==(const Settings& a, const Settings& b);
bool operator!=(const Settings& a, const Settings& b);

// MARK: -

inline Settings::Settings()
  : size(),
    internalFormat(GL_RGB),
    format(GL_RGB),
    type(GL_UNSIGNED_BYTE),
    near(0.1),
    far(1024.0),
    top(M_PI_2),
    right(M_PI),
    bottom(-M_PI_2),
    left(-M_PI) {}

// MARK: Comparison

inline bool operator==(const Settings& a, const Settings& b) {
  return (a.size == b.size &&
          a.internalFormat == b.internalFormat &&
          a.format == b.format &&
          a.type == b.type &&
          a.near == b.near &&
          a.far == b.far &&
          a.top == b.top &&
          a.right == b.right &&
          a.bottom == b.bottom &&
          a.left == b.left);
}

inline bool operator!=(const Settings& a, const Settings& b) {
  return !(a == b);
}

}  // namespace ofxsphericalprojection

using ofxSphericalProjectionSettings = ofxsphericalprojection::Settings;
