#include "envi_parser.h"

inline bool fileExists(const std::string &name) {
    ifstream f(name.c_str());
    return f.good();
}

template<typename TIdx>
vector<TIdx> envi_parser::makeFloatCube(string cubepath, string darkrefpath, string whiterefpath, UInt16 width, UInt16 bands, UInt16 *cubeHeight, TIdx pixelGain) {

    if (!fileExists(cubepath)) {
        printf("Can't find the cube file. Make sure you point at the correct location.\n");
        return vector<TIdx>();
    }
    if (!fileExists(darkrefpath)) {
        printf("Can't find the dark reference file. Make sure you point at the correct location.\n");
        return vector<TIdx>();
    }
    if (!fileExists(whiterefpath)) {
        printf("Can't find the white reference file. Make sure you point at the correct location.\n");
        return vector<TIdx>();
    }

    // Setup matrices
    vector<UInt16> image;
    vector<UInt16> darkref;
    vector<UInt16> whiteref;

    // Read files into 3d arrays
    size_t imageSize = readRawENVI(image, cubepath, width, bands);
    size_t darkSize = readRawENVI(darkref, darkrefpath, width, bands);
    size_t whiteSize = readRawENVI(whiteref, whiterefpath, width, bands);

    auto _cubeHeight = static_cast<UInt16>(imageSize / (bands * width));
    auto blackHeight = static_cast<UInt16>(darkSize / (bands * width));
    auto whiteHeight = static_cast<UInt16>(whiteSize / (bands * width));

    *cubeHeight = _cubeHeight;
    vector<TIdx> imaged(image.begin(), image.end());
    vector<TIdx> darkrefd(darkref.begin(), darkref.end());
    vector<TIdx> whiterefd(whiteref.begin(), whiteref.end());

    // Normalize image cube with dark ref and white ref
    normalizeENVI(imaged, darkrefd, whiterefd, (TIdx) bands, envi_parser::elBandInterleaveByLine,
                  (TIdx) width, (TIdx) _cubeHeight, (TIdx) whiteHeight, (TIdx) blackHeight);


    if (pixelGain != 1.0f)
        std::transform(imaged.begin(), imaged.end(), imaged.begin(), std::bind1st(std::multiplies<TIdx>(), pixelGain));

    // Convert matrix to a tiled version, 1 wide and 224 long
    // This way, the first width*cubeHeight pixels are band 1, etc.
    // When we convert it to numpy, this will be right automatically
    return BILToTiled(imaged, (short) width, _cubeHeight, (short) bands);
}

template<typename TIdx>
vector<TIdx> envi_parser::BILToTiled(vector<TIdx> bil, UInt16 tileWidth, UInt16 tileHeight, UInt16 bandCount) {

    if ((size_t) bil.size() % (size_t) (bandCount * tileWidth * tileHeight) != 0)
        throw std::runtime_error("Source image height is not dividable by the number of bands.");

    vector<TIdx> tiled(tileWidth * tileHeight * bandCount);

    int srcy = 0;
    for (int lineCnt = 0; lineCnt < tileHeight; lineCnt++) {
        for (int bandCnt = 0; bandCnt < bandCount; bandCnt++) {
            const int desty = (bandCnt * tileHeight) + lineCnt;

            TIdx *dstPtr = &tiled[desty * tileWidth];
            const TIdx *srcPtr = &bil[srcy * tileWidth];

            memcpy(dstPtr, srcPtr, tileWidth * sizeof(TIdx));
            srcy++;
        }
    }
    return tiled;
}

template<typename T>
size_t inline envi_parser::readToVector(const string &filename, vector<T> &buf) {
    std::ifstream ifs(filename, ios::binary);
    if (!ifs) throw std::runtime_error("Cannot open file: " + filename);
    ifs.seekg(0, std::ios::end);
    auto size = (size_t) ifs.tellg();
    ifs.seekg(0);
    buf.resize(size / sizeof(T));
    ifs.read(static_cast<char *>(static_cast<void *>(buf.data())), size);

    if (!ifs) throw std::runtime_error("Not all bytes read");
    return buf.size();
}

template<typename TIdx>
size_t envi_parser::readRawENVI(vector<TIdx> &dst, string &filename, TIdx width, TIdx bands) {
    vector<TIdx> buf;
    auto size = (size_t) readToVector(filename, buf);

    if (size % (width * bands) != 0)
        throw std::runtime_error("Cannot deduce height form file size : %s");

    dst = buf;
    return size;
}

template<typename TIdx>
void envi_parser::normalizeENVI(vector<TIdx> &cube, const vector<TIdx> &black, const vector<TIdx> &white, TIdx bands,
                                enviLayout cubeLayout, TIdx cubeCols, TIdx cubeRows, TIdx whiteHeight, TIdx blackHeight) {
    if (cubeLayout != elBandInterleaveByLine)
        throw std::runtime_error("Specified cubeLayout not supported");

    vector<TIdx> whiteRef(bands * cubeCols * whiteHeight);
    vector<TIdx> blackRef(bands * cubeCols * blackHeight);
    //Take average for white
    for (UInt16 y = 0; y < whiteHeight; y++) {
        for (UInt16 b = 0; b < bands; b++) {
            TIdx *dstPtr = &whiteRef[b * cubeCols];
            const TIdx *srcPtr = &white.at((y * bands + b) * cubeCols);
            for (TIdx px = 0; px < cubeCols; px++, dstPtr++, srcPtr++)
                *dstPtr += *srcPtr;
        }
    }
    //Take average for black
    for (UInt16 y = 0; y < blackHeight; y++) {
        for (UInt16 b = 0; b < bands; b++) {
            TIdx *dstPtr = &blackRef[b * cubeCols];
            const TIdx *srcPtr = &black[(y * bands + b) * cubeCols];
            for (TIdx px = 0; px < cubeCols; px++, dstPtr++, srcPtr++)
                *dstPtr += *srcPtr;
        }
    }
    //Average
    for (UInt16 b = 0; b < bands; b++) {
        TIdx *ptrWhite = &whiteRef[b * cubeCols];
        TIdx *ptrBlack = &blackRef[b * cubeCols];
        for (UInt16 px = 0; px < cubeCols; px++, ptrWhite++, ptrBlack++) {
            *ptrWhite /= whiteHeight;
            *ptrBlack /= blackHeight;
        }
    }
    //Normalise
    for (UInt16 y = 0; y < cubeRows; y++) {
        for (UInt16 b = 0; b < bands; b++) {
            const TIdx *ptrWhite = &whiteRef[b * cubeCols];
            const TIdx *ptrBlack = &blackRef[b * cubeCols];
            TIdx *ptrDst = &cube[(y * bands + b) * cubeCols];
            for (TIdx x = 0; x < cubeCols; x++, ptrWhite++, ptrBlack++, ptrDst++)
                *ptrDst = (*ptrDst - *ptrBlack) / (*ptrWhite - *ptrBlack);
        }
    }
}

// Process a capture
bool envi_parser::processCapture(const string &cubepath, const string &darkrefpath, const string &whiterefpath, string destinationpath, UInt16 width, UInt16 bands, float pixelGain) {
    try {

        printf(" -> Writing numpy array with data...\n");

        UInt16 cubeHeight;
        vector<float> tiled = makeFloatCube<float>(cubepath, darkrefpath, whiterefpath, width, bands, &cubeHeight, pixelGain);

        if (tiled.empty()) {
            printf(" -> Conversion failed, no tiles returned\n\n");
            return false;
        }

        cnpy::npy_save(std::move(destinationpath), &tiled[0], {bands, cubeHeight, width}, "w");

        printf(" -> Conversion done\n");
        return true;

    } catch (const std::exception &e) {
        cout << "STD" << e.what();
    }
    return false;
}

// Process a capture
bool envi_parser::processNormalizedCapture(const string &cubepath, const string &darkrefpath, const string &whiterefpath, string destinationpath, UInt16 width, UInt16 bands) {
    try {

        printf(" -> Writing normalized numpy array with data...\n");

        float pixelGain = 1.0f;

        UInt16 cubeHeight;
        typedef float outputType;

        vector<outputType> tiled = makeFloatCube<float>(cubepath, darkrefpath, whiterefpath, width, bands, &cubeHeight, pixelGain);

        if (tiled.size() >= (size_t) (width * cubeHeight * 2lu)) {
            vector<outputType> normalized(static_cast<unsigned long>(width * cubeHeight * (bands - 1)));

            for (UInt16 b = 0; b < (bands - 1); b++) {
                unsigned int size = width * cubeHeight;
                unsigned int offSet = b * size;
                vector<outputType> bandA(tiled.begin() + offSet, tiled.begin() + offSet + size);
                vector<outputType> bandB(tiled.begin() + offSet + size, tiled.begin() + offSet + 2 * size);
                std::transform(bandB.begin(), bandB.end(), bandA.begin(), normalized.begin() + offSet, std::divides<outputType>());
            }

            cnpy::npy_save(std::move(destinationpath), &normalized[0], {bands - 1lu, cubeHeight, width}, "w");
        } else {
            printf(" -> Conversion failed, no tiles returned\n\n");
            return false;
        }

        printf(" -> Conversion done\n");

        return true;

    } catch (const std::exception &e) {
        cout << "STD" << e.what();
    }
    return false;
}
