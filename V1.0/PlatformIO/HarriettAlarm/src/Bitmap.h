#ifndef HARRIETT_BITMAP_H
#define HARRIETT_BITMAP_H
class Bitmap {

  public:
  Bitmap(uint8_t* bits, uint16_t width, uint16_t height) :

    _bits(bits), _width(width), _height(height) {}

    uint8_t *bits() {
      return _bits;
    }

    uint16_t w() {
      return _width;
    }

    uint16_t h() {
      return _height;
    }

    protected:
    uint8_t *_bits;
    uint16_t _width;
    uint16_t _height;
};
#endif