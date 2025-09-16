#ifndef COLORMAP_H
#define COLORMAP_H

static int cmap_red[] = {0, 49, 107, 158, 198, 230, 253, 253, 253, 49, 116, 161, 199, 117, 158, 188, 218, 99, 150, 189, 217};
static int cmap_green[] = {0, 130, 174, 202, 219, 85, 141, 174, 208, 163, 196, 217, 233, 107, 154, 189, 218, 99, 150, 189, 217};
static int cmap_blue[] = {0, 189, 214, 225, 239, 13, 60, 107, 162, 84, 118, 155, 192, 177, 200, 220, 235, 99, 150, 189, 217};

enum class ChannelOrder {
    H_W_C, C_H_W, Auto
};
enum class ColorMode {
    Grayscale, Colormap, RGB, BGR, Seismic
};

enum class ContrastMode {
    Array, Canvas
};

inline unsigned long index_in_vector(ChannelOrder channelOrder, int n, int x, int y, int channel, int width, int height, int num_channels) {
    if (channelOrder == ChannelOrder::C_H_W) {
        return static_cast<unsigned long>((y * width + x) + (channel * width * height) + (n * num_channels * width * height));
    } else {
        return static_cast<unsigned long> ((y * width + x) * num_channels + channel + (n * num_channels * width * height));
    }
}


#endif // COLORMAP_H
