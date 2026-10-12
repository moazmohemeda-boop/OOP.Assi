#ifndef FILTERS_H
#define FILTERS_H

#include "Image_Class.h"

Image grayscale(Image img);
Image blackAndWhite(Image img);
Image invertColors(Image img);
Image addFrame(Image img, int thickness, int r, int g, int b);
Image flipImage(Image img, bool horizontal);          // true = horizontal, false = vertical
Image rotateImage(Image img, int degrees);            // 90 / 180 / 270 clockwise
Image adjustBrightness(Image img, int percent);       // -100 .. +100
Image resizeImage(Image img, int newW, int newH);
Image mergeImages(Image a, Image b);
Image detectEdges(Image img);
Image cropImage(Image img, int x, int y, int w, int h);
Image strongBlur(Image img);
Image wanoSunlight(Image img);
Image oldTVEffect(Image img);
Image purpleTint(Image img);
Image infraredEffect(Image img);
Image skewImage(Image img, double angleDegrees);      // -60 .. +60
Image oilPainting(Image img);

#endif