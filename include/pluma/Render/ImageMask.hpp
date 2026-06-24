#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>

namespace pluma {

class ImageMask {
public:
    ImageMask() : width_(0), height_(0) {}
    ImageMask(int w, int h) : width_(w), height_(h), rows_(h) {}

    void addSegment(int y, int start_x, int end_x) {
        if (y >= 0 && y < height_) {
            rows_[y].push_back({start_x, end_x});
        }
    }

    // Queries if a rectangular area [x1, x2] x [y1, y2] intersects any solid pixels.
    // Coordinates must be in image pixel space.
    bool intersectsRect(int y1, int y2, int x1, int x2) const {
        int start_y = std::max(0, y1);
        int end_y = std::min(height_ - 1, y2);
        
        for (int y = start_y; y <= end_y; ++y) {
            for (const auto& seg : rows_[y]) {
                if (x1 <= seg.second && x2 >= seg.first) {
                    return true; // Overlaps
                }
            }
        }
        return false;
    }
    
    // Finds the next transparent gap for a rect of size (w, h) starting at (x, y).
    // Checks the vertical range [y, y+h-1].
    // Returns the new x position. If no gap fits within the image width, returns width_.
    int findGap(int y1, int y2, int x, int w) const {
        int start_y = std::max(0, y1);
        int end_y = std::min(height_ - 1, y2);
        
        int current_x = x;
        bool fits = false;
        
        while (!fits) {
            // Early exit: si ya superamos el ancho de la imagen, no hay gap disponible
            if (current_x >= width_) {
                return width_;
            }
            fits = true;
            for (int y = start_y; y <= end_y; ++y) {
                for (const auto& seg : rows_[y]) {
                    if (current_x <= seg.second && current_x + w - 1 >= seg.first) {
                        // Collision! Push current_x past this segment
                        current_x = seg.second + 1;
                        fits = false;
                        break;
                    }
                }
                if (!fits) break;
            }
        }
        
        return current_x;
    }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

private:
    int width_;
    int height_;
    std::vector<std::vector<std::pair<int, int>>> rows_;
};

} // namespace pluma
