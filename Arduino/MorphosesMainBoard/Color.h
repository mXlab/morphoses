#ifndef COLOR_H_
#define COLOR_H_

#include <PlaquetteLib.h>

#include "hsv_rgb_convert.h"

class Color {
public:

  // RGB/HSV values.
  uint16_t _rh;
  uint8_t  _gs, _bv;
  bool _isRgb;

  // Constructor.
  Color(bool rgb=true);
  Color(int rh, int gs, int bv, bool rgb=true);

  // Factory methods.
  static Color rgb(int r, int g, int b);
  static Color hsv(int h, int s, int v);
  static Color lerp(Color from, Color to, float t);

  void copyFrom(const Color& o) {
    *this = isRgb() ? o.toRgb() : o.toHsv();
  }
  
  int r() const;
  int g() const;
  int b() const;

  void setR(int r);
  void setG(int g);
  void setB(int b);

  void setRgb(int r, int g, int b);

  int h() const;
  int s() const;
  int v() const;

  void setH(int h);
  void setS(int s);
  void setV(int v);

  void setHsv(int h, int s, int v);

  bool isRgb() const { return _isRgb; }
  // Warning: this may involve a conversion that might change the actual color.
  void setIsRgb(bool isRgb);

  void clear();

  Color toRgb() const { return Color::rgb(r(), g(), b()); }
  Color toHsv() const { return Color::hsv(h(), s(), v()); }

private:
  int _hsvToRgbComponent(int c) const;
  int _rgbToHsvComponent(int c) const;
};


#endif /* COLOR_H_ */
