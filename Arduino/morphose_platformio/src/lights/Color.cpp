#include "Color.h"

Color::Color(bool rgb) : _rh(0), _gs(0), _bv(0), _isRgb(rgb) {}
Color::Color(int rh, int gs, int bv, bool rgb) : _rh(rh), _gs(gs), _bv(bv), _isRgb(rgb) {
}

Color Color::rgb(int r, int g, int b) {
  return Color(r, g, b, true);
}

Color Color::hsv(int h, int s, int v) {
  return Color(h, s, v, false);
}

Color Color::lerp(Color from, Color to, float t) {
  Color result;

  // Make sure to operate in the same space.
  to.setIsRgb(from.isRgb());

  // Keep t in [0, 1].
  t = constrain(t, 0, 1);
  if (from.isRgb()) {
    float r = pq::mapFrom01(t, from.r(), to.r());
    float g = pq::mapFrom01(t, from.g(), to.g());
    float b = pq::mapFrom01(t, from.b(), to.b());
    result.setRgb(round(r), round(g), round(b));
  } else {
    float r = pq::mapFrom01(t, from.r(), to.r());
    float g = pq::mapFrom01(t, from.g(), to.g());
    float b = pq::mapFrom01(t, from.b(), to.b());
    result.setRgb(round(r), round(g), round(b));
  }

  return result;
}

int Color::r() const {
  return ( _isRgb ? _rh : _hsvToRgbComponent(0) );
}

int Color::g() const {
  return ( _isRgb ? _gs : _hsvToRgbComponent(1) );
}

int Color::b() const {
  return ( _isRgb ? _bv : _hsvToRgbComponent(2) );
}

void Color::setR(int r) {
  setRgb(r, g(), b());
}

void Color::setG(int g) {
  setRgb(r(), g, b());
}

void Color::setB(int b) {
  setRgb(r(), g(), b);
}

void Color::setRgb(int r, int g, int b) {

  if (_isRgb) {
    _rh = r; _gs = g; _bv = b;
  } else {
    rgb_to_hsv_int(&r, &g, &b);
    setHsv(r, g, b);
  }
}

int Color::h() const {
  return ( !_isRgb ? _rh : _rgbToHsvComponent(0) );
}

int Color::s() const {
  return ( !_isRgb ? _gs : _rgbToHsvComponent(1) );
}

int Color::v() const {
  return ( !_isRgb ? _bv : _rgbToHsvComponent(2) );
}

void Color::setH(int h) {
  setHsv(h, s(), v());
}

void Color::setS(int s) {
  setHsv(h(), s, v());
}

void Color::setV(int v) {
  setHsv(h(), s(), v);
}

void Color::setHsv(int h, int s, int v) {
  if (!_isRgb) {
    _rh = h; _gs = s; _bv = v;
  } else {
    hsv_to_rgb_int(&h, &s, &v);
    setRgb(h, s, v);
  }
}

void Color::setIsRgb(bool isRgb) {
  if (isRgb != _isRgb) {
    int data[3] = { _rh, _gs, _bv };
    // HSV -> RGB.
    if (isRgb)
      hsv_to_rgb_int(&data[0], &data[1], &data[2]);
    else
      rgb_to_hsv_int(&data[0], &data[1], &data[2]);
    _rh = data[0]; _gs = data[1]; _bv = data[2];
    _isRgb = isRgb;
  }
}

void Color::clear() {
  if (_isRgb)
    setRgb(0, 0, 0);
  else
    setV(0);
}

int Color::_hsvToRgbComponent(int c) const {
  int data[3] = { h(), s(), v() };
  hsv_to_rgb_int(&data[0], &data[1], &data[2]);
  return data[c];
}

int Color::_rgbToHsvComponent(int c) const {
  int data[3] = { r(), g(), b() };
  rgb_to_hsv_int(&data[0], &data[1], &data[2]);
  return data[c];
}
