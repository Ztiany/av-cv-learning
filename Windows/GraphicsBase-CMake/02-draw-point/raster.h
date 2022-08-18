#ifndef GRAPHICSBASE_CMAKE_RASTER_H
#define GRAPHICSBASE_CMAKE_RASTER_H

#include "../libs/CELLMath.hpp"

namespace CELL {

    class Raster {
    public:
        Rgba _buffer[256][256];
    public:
        Raster();

        ~Raster();

    public:
        void clear() {
            memset(_buffer, 0, sizeof(_buffer));
        }

        void drawPoint(int x, int y, Rgba color, int ptSize) {
            switch (ptSize) {
                case 1: {
                    setPixel(x, y, color);
                    break;
                }
                case 2: {
                    setPixel(x + 0, y + 0, color);
                    setPixel(x + 1, y + 0, color);
                    setPixel(x + 0, y + 1, color);
                    setPixel(x + 1, y + 1, color);
                    break;
                }
                case 3: {
                    setPixel(x - 1, y - 1, color);
                    setPixel(x + 0, y - 1, color);
                    setPixel(x + 1, y - 1, color);

                    setPixel(x - 1, y + 0, color);
                    setPixel(x + 0, y + 0, color);
                    setPixel(x + 1, y + 0, color);

                    setPixel(x - 1, y + 1, color);
                    setPixel(x + 0, y + 1, color);
                    setPixel(x + 1, y + 1, color);

                    break;
                }
                default: {
                    break;
                }
            }
        }

        void setPixel(int x, int y, Rgba color) {
            if (x < 0 || y < 0 || x >= 256 || y >= 256) {
                return;
            }
            _buffer[x][y] = color;
        }
    };

}

#endif //GRAPHICSBASE_CMAKE_RASTER_H
