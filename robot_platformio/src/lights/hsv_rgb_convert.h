/*
 * Color.h
 *
 *  Created on: Aug 11, 2010
 *      Author: tats
 */

#ifndef ARDUINO_MORPHOSE_PLATFORMIO_SRC_LIGHTS_HSV_RGB_CONVERT_H_
#define ARDUINO_MORPHOSE_PLATFORMIO_SRC_LIGHTS_HSV_RGB_CONVERT_H_

// From Arduino
#ifndef MIN
#define MIN min
#endif

#ifndef MAX
#define MAX max
#endif

#ifndef ROUND
#define ROUND round
#endif

#define _FAST_HSV_RGB_CONVERT 1

// # Basic colorspace convert functions (from the Gimp gimpcolorspace.h) ####

/*  int functions  */

/**
 * rgb_to_hsv_int
 * @red: The red channel value, returns the Hue channel
 * @green: The green channel value, returns the Saturation channel
 * @blue: The blue channel value, returns the Value channel
 *
 * The arguments are pointers to int representing channel values in
 * the RGB colorspace, and the values pointed to are all in the range
 * [0, 255].
 *
 * The function changes the arguments to point to the HSV value
 * corresponding, with the returned values in the following
 * ranges: H [0, 360], S [0, 255], V [0, 255].
 **/
inline void
rgb_to_hsv_int(int *red         /* returns hue        */,
                int *green       /* returns saturation */,
                int *blue        /* returns value      */) {
  float  r, g, b;
  float  h, s, v;
  float  minimum;
  float  delta;
  Serial.println("here");
  r = *red;
  g = *green;
  b = *blue;

  if (r > g) {
    v = MAX(r, b);
    minimum = MIN(g, b);
  } else {
    v = MAX(g, b);
    minimum = MIN(r, b);
  }

  delta = v - minimum;

  if (v == 0.0) {
    s = 0.0;
  } else {
    s = delta / v;
  }
  if (s == 0.0) {
    h = 0.0;
  } else {
    if (r == v) {
      h = 60.0 * (g - b) / delta;
    } else if (g == v) {
      h = 120 + 60.0 * (b - r) / delta;
    } else {
      h = 240 + 60.0 * (r - g) / delta;
    }

    if (h < 0.0) h += 360.0;
    if (h > 360.0) h -= 360.0;
  }

  *red   = ROUND(h);
  *green = ROUND(s * 255.0);
  *blue  = ROUND(v);
}

/**
 * hsv_to_rgb_int
 * @hue: The hue channel, returns the red channel
 * @saturation: The saturation channel, returns the green channel
 * @value: The value channel, returns the blue channel
 *
 * The arguments are pointers to int, with the values pointed to in the
 * following ranges:  H [0, 360], S [0, 255], V [0, 255].
 *
 * The function changes the arguments to point to the RGB value
 * corresponding, with the returned values all in the range [0, 255].
 **/
inline void
hsv_to_rgb_int(int *hue         /* returns red        */,
                int *saturation  /* returns green      */,
                int *value       /* returns blue       */) {
#if _FAST_HSV_RGB_CONVERT
  // Adapted from: web.mit.edu/storborg/Public/hsvtorgb.c
  unsigned int h, s, v;
  unsigned long fpart;
  unsigned int region;
  unsigned char p, q, t;

  // Grayscale.
  if (*saturation == 0) {
    *hue        = *value;
    *saturation = *value;
    //    *value      = *value;
  } else {
    h = *hue;
    s = *saturation;
    v = *value;

    // Transfer hue in 0..5 make hue 0-5 */
    region = *hue / 60;
    region = min(region, 5U);  // avoid to fall in region 6 if hue is 360

    /* find remainder part, make it from 0-65535 */
    fpart = (h - (region * 60));
    fpart = (fpart == 59 ? 65535U : fpart * 1111);

    /* calculate temp vars, doing integer multiplication */
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 16))) >> 8;
    t = (v * (255 - ((s * (65535U - fpart)) >> 16))) >> 8;

    /* assign temp vars based on color cone region */
    switch (region) {
      case 0:
        *hue = v; *saturation = t; *value = p; break;
      case 1:
        *hue = q; *saturation = v; *value = p; break;
      case 2:
        *hue = p; *saturation = v; *value = t; break;
      case 3:
        *hue = p; *saturation = q; *value = v; break;
      case 4:
        *hue = t; *saturation = p; *value = v; break;
      default:
        *hue = v; *saturation = p; *value = q; break;
    }
  }
#else
  // From Gimp source
  float h, s, v, h_temp;
  float f, p, q, t;
  int i;

  if (*saturation == 0) {
    *hue        = *value;
    *saturation = *value;
    //    *value      = *value;
  } else {
    h = *hue;
    s = *saturation / 255.0;
    v = *value      / 255.0;

    if (h == 360)
      h_temp = 0;
    else
      h_temp = h;

    h_temp = h_temp / 60.0;
    i = (int) floor(h_temp);
    f = h_temp - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));

    switch (i) {
    case 0:
      *hue        = ROUND(v * 255.0);
      *saturation = ROUND(t * 255.0);
      *value      = ROUND(p * 255.0);
      break;

    case 1:
      *hue        = ROUND(q * 255.0);
      *saturation = ROUND(v * 255.0);
      *value      = ROUND(p * 255.0);
      break;

    case 2:
      *hue        = ROUND(p * 255.0);
      *saturation = ROUND(v * 255.0);
      *value      = ROUND(t * 255.0);
      break;

    case 3:
      *hue        = ROUND(p * 255.0);
      *saturation = ROUND(q * 255.0);
      *value      = ROUND(v * 255.0);
      break;

    case 4:
      *hue        = ROUND(t * 255.0);
      *saturation = ROUND(p * 255.0);
      *value      = ROUND(v * 255.0);
      break;

    case 5:
      *hue        = ROUND(v * 255.0);
      *saturation = ROUND(p * 255.0);
      *value      = ROUND(q * 255.0);
      break;
    }
  }
#endif
}

#endif  // ARDUINO_MORPHOSE_PLATFORMIO_SRC_LIGHTS_HSV_RGB_CONVERT_H_

