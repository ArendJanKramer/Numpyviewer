#ifndef COLORMAP_H
#define COLORMAP_H

static int cmap_red[] = {0, 49, 107, 158, 198, 230, 253, 253, 253, 49, 116, 161, 199, 117, 158, 188, 218, 99, 150, 189, 217};
static int cmap_green[] = {0, 130, 174, 202, 219, 85, 141, 174, 208, 163, 196, 217, 233, 107, 154, 189, 218, 99, 150, 189, 217};
static int cmap_blue[] = {0, 189, 214, 225, 239, 13, 60, 107, 162, 84, 118, 155, 192, 177, 200, 220, 235, 99, 150, 189, 217};

inline unsigned long index_in_vector(bool channelsfirst, int x, int y, int channel, int width, int height, int num_channels)
{
    if (channelsfirst){
        return static_cast<unsigned long>((y*width+x)+(channel * width * height));
    }else{
        return static_cast<unsigned long>((x*height+y)*num_channels+channel);
    }

}

#endif // COLORMAP_H
