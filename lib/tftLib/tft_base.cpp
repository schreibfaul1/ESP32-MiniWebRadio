#include "tft_base.h"

#include <utility>

uint16_t TFT_Base::logicalWidth() const {
    if (m_rotation & 1) return m_v_res;
    return m_h_res;
}

uint16_t TFT_Base::logicalHeight() const {
    if (m_rotation & 1) return m_h_res;
    return m_v_res;
}

bool TFT_Base::renderRGB565(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t* rgb, const uint8_t* alpha) {
    if (!rgb || w == 0 || h == 0) return false;

    int32_t minX = logicalWidth();
    int32_t minY = logicalHeight();
    int32_t maxX = -1;
    int32_t maxY = -1;

    for (uint16_t row = 0; row < h; ++row) {
        for (uint16_t col = 0; col < w; ++col) {
            int32_t srcX = x + col;
            int32_t srcY = y + row;

            int32_t dstX, dstY;
            mapRotation(m_rotation, srcX, srcY, dstX, dstY);

            if (dstX < 0 || dstY < 0) continue;
            if (dstX >= m_h_res || dstY >= m_v_res) continue;

            const size_t fbIndex = dstY * m_h_res + dstX;
            const size_t srcIndex = row * w + col;

            uint16_t newColor = rgb[srcIndex];

            if (alpha) {
                uint8_t a = alpha[srcIndex];

                if (a == 0) continue;

                if (a < 255) {
                    uint16_t old = m_framebuffer[0][fbIndex];
                    uint16_t invA = 255 - a;

                    uint8_t r = ((newColor >> 11) & 0x1F) << 3;
                    uint8_t g = ((newColor >> 5) & 0x3F) << 2;
                    uint8_t b = (newColor & 0x1F) << 3;

                    uint8_t oldR = ((old >> 11) & 0x1F) << 3;
                    uint8_t oldG = ((old >> 5) & 0x3F) << 2;
                    uint8_t oldB = (old & 0x1F) << 3;

                    r = (r * a + oldR * invA + 128) >> 8;
                    g = (g * a + oldG * invA + 128) >> 8;
                    b = (b * a + oldB * invA + 128) >> 8;

                    newColor = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                }
            }

            m_framebuffer[0][fbIndex] = newColor;

            if (dstX < minX) minX = dstX;
            if (dstY < minY) minY = dstY;
            if (dstX > maxX) maxX = dstX;
            if (dstY > maxY) maxY = dstY;
        }
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }

    return true;
}

void TFT_Base::mapRotation(uint8_t rot, int32_t srcX, int32_t srcY, int32_t& dstX, int32_t& dstY) const {
    switch (rot & 3) {
        case 0:
            dstX = srcX;
            dstY = srcY;
            break;

        case 1:
            dstX = logicalHeight() - 1 - srcY;
            dstY = srcX;
            break;

        case 2:
            dstX = logicalWidth() - 1 - srcX;
            dstY = logicalHeight() - 1 - srcY;
            break;

        case 3:
            dstX = srcY;
            dstY = logicalWidth() - 1 - srcX;
            break;
    }
}

void TFT_Base::drawRectLogicalFromFB(uint8_t fb, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    int32_t px[4], py[4];
    int32_t sx[4] = {x, x + w - 1, x, x + w - 1};
    int32_t sy[4] = {y, y, y + h - 1, y + h - 1};

    int32_t minX = m_h_res;
    int32_t minY = m_v_res;
    int32_t maxX = -1;
    int32_t maxY = -1;

    for (int i = 0; i < 4; i++) {
        mapRotation(m_rotation, sx[i], sy[i], px[i], py[i]);

        if (px[i] < minX) minX = px[i];
        if (py[i] < minY) minY = py[i];
        if (px[i] > maxX) maxX = px[i];
        if (py[i] > maxY) maxY = py[i];
    }

    panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[fb]);
}

bool TFT_Base::copyFramebuffer(uint8_t source, uint8_t destination, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if (w == 0 || h == 0) return false;

    uint16_t lw = logicalWidth();
    uint16_t lh = logicalHeight();

    if (x >= lw || y >= lh) return false;
    if (x + w > lw) w = lw - x;
    if (y + h > lh) h = lh - y;

    for (uint16_t row = 0; row < h; ++row) {
        for (uint16_t col = 0; col < w; ++col) {
            int32_t physX, physY;
            mapRotation(m_rotation, x + col, y + row, physX, physY);

            if (physX < 0 || physY < 0 || physX >= m_h_res || physY >= m_v_res) continue;

            m_framebuffer[destination][physY * m_h_res + physX] = m_framebuffer[source][physY * m_h_res + physX];
        }
    }

    if (destination == 0) drawRectLogicalFromFB(0, x, y, w, h);

    return true;
}

void TFT_Base::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    if (x >= logicalWidth() || y >= logicalHeight()) return;
    if (x + w < 0 || y + h < 0) return;

    uint16_t lineBuffer[w];
    for (int16_t i = 0; i < w; ++i) lineBuffer[i] = color;

    for (int16_t row = 0; row < h; ++row) {
        renderRGB565(x, y + row, w, 1, lineBuffer, nullptr);
    }
}

void TFT_Base::fillScreen(uint16_t color) {
    fillRect(0, 0, logicalWidth(), logicalHeight(), color);
}

void TFT_Base::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    while (true) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, x0, y0, rotX, rotY);

        if (rotX >= 0 && rotX < m_h_res && rotY >= 0 && rotY < m_v_res) {
            m_framebuffer[0][rotY * m_h_res + rotX] = color;

            if (rotX < minX) minX = rotX;
            if (rotY < minY) minY = rotY;
            if (rotX > maxX) maxX = rotX;
            if (rotY > maxY) maxY = rotY;
        }

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}

void TFT_Base::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

void TFT_Base::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto drawSpan = [&](int16_t xs, int16_t xe, int16_t y) {
        if (xs > xe) std::swap(xs, xe);

        for (int16_t x = xs; x <= xe; ++x) {
            int32_t rotX, rotY;
            mapRotation(m_rotation, x, y, rotX, rotY);

            if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) continue;

            m_framebuffer[0][rotY * m_h_res + rotX] = color;

            if (rotX < minX) minX = rotX;
            if (rotY < minY) minY = rotY;
            if (rotX > maxX) maxX = rotX;
            if (rotY > maxY) maxY = rotY;
        }
    };

    if (y1 == y2) {
        for (int16_t y = y0; y <= y1; ++y) {
            int16_t xa = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawSpan(xa, xb, y);
        }
    } else if (y0 == y1) {
        for (int16_t y = y0; y <= y2; ++y) {
            int16_t xa = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            int16_t xb = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            drawSpan(xa, xb, y);
        }
    } else {
        for (int16_t y = y0; y <= y1; ++y) {
            int16_t xa = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawSpan(xa, xb, y);
        }

        for (int16_t y = y1; y <= y2; ++y) {
            int16_t xa = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            int16_t xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawSpan(xa, xb, y);
        }
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}

void TFT_Base::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (w == 0 || h == 0) return;

    drawLine(x, y, x + w - 1, y, color);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    drawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    drawLine(x, y + h - 1, x, y, color);
}

void TFT_Base::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (w <= 0 || h <= 0 || r <= 0) return;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    drawLine(x + r, y, x + w - r - 1, y, color);
    drawLine(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);
    drawLine(x, y + r, x, y + h - r - 1, color);
    drawLine(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t cx = 0;
    int16_t cy = r;

    while (cx <= cy) {
        auto plot = [&](int16_t px, int16_t py) {
            int32_t rotX, rotY;
            mapRotation(m_rotation, px, py, rotX, rotY);

            if (rotX >= 0 && rotX < m_h_res && rotY >= 0 && rotY < m_v_res) {
                m_framebuffer[0][rotY * m_h_res + rotX] = color;
            }
        };

        plot(x + w - r - 1 + cx, y + r - cy);
        plot(x + w - r - 1 + cy, y + r - cx);
        plot(x + w - r - 1 + cx, y + h - r - 1 + cy);
        plot(x + w - r - 1 + cy, y + h - r - 1 + cx);
        plot(x + r - cx, y + h - r - 1 + cy);
        plot(x + r - cy, y + h - r - 1 + cx);
        plot(x + r - cx, y + r - cy);
        plot(x + r - cy, y + r - cx);

        if (f >= 0) {
            cy--;
            ddF_y += 2;
            f += ddF_y;
        }

        cx++;
        ddF_x += 2;
        f += ddF_x;
    }

    int32_t minX = m_h_res;
    int32_t minY = m_v_res;
    int32_t maxX = -1;
    int32_t maxY = -1;

    auto includeCorner = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    includeCorner(x, y);
    includeCorner(x + w - 1, y);
    includeCorner(x, y + h - 1);
    includeCorner(x + w - 1, y + h - 1);

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}

void TFT_Base::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (w <= 0 || h <= 0) return;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto plot = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) return;

        m_framebuffer[0][rotY * m_h_res + rotX] = color;

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    auto drawSpan = [&](int16_t xs, int16_t xe, int16_t py) {
        if (xs > xe) std::swap(xs, xe);
        for (int16_t px = xs; px <= xe; ++px) plot(px, py);
    };

    for (int16_t py = y + r; py < y + h - r; ++py) drawSpan(x, x + w - 1, py);

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t cx = 0;
    int16_t cy = r;

    while (cx <= cy) {
        drawSpan(x + r - cx, x + w - r - 1 + cx, y + r - cy);
        drawSpan(x + r - cy, x + w - r - 1 + cy, y + r - cx);
        drawSpan(x + r - cx, x + w - r - 1 + cx, y + h - r - 1 + cy);
        drawSpan(x + r - cy, x + w - r - 1 + cy, y + h - r - 1 + cx);

        if (f >= 0) {
            cy--;
            ddF_y += 2;
            f += ddF_y;
        }

        cx++;
        ddF_x += 2;
        f += ddF_x;
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}

void TFT_Base::drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    if (r <= 0) return;

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto plot = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) return;

        m_framebuffer[0][rotY * m_h_res + rotX] = color;

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    plot(cx, cy + r);
    plot(cx, cy - r);
    plot(cx + r, cy);
    plot(cx - r, cy);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        plot(cx + x, cy + y);
        plot(cx - x, cy + y);
        plot(cx + x, cy - y);
        plot(cx - x, cy - y);
        plot(cx + y, cy + x);
        plot(cx - y, cy + x);
        plot(cx + y, cy - x);
        plot(cx - y, cy - x);
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}

void TFT_Base::fillCircle(int16_t cx, int16_t cy, uint16_t r, uint16_t color) {
    if (r == 0) return;

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto plot = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) return;

        m_framebuffer[0][rotY * m_h_res + rotX] = color;

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    auto drawSpan = [&](int16_t xs, int16_t xe, int16_t py) {
        if (xs > xe) std::swap(xs, xe);
        for (int16_t px = xs; px <= xe; ++px) plot(px, py);
    };

    drawSpan(cx, cx, cy - r);
    drawSpan(cx, cx, cy + r);

    while (x <= y) {
        drawSpan(cx - x, cx + x, cy + y);
        drawSpan(cx - x, cx + x, cy - y);
        drawSpan(cx - y, cx + y, cy + x);
        drawSpan(cx - y, cx + y, cy - x);

        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
