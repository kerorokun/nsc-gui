#include "msdf_wrapper.hpp"

#define _USE_MATH_DEFINES
#include <cstdio>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <thread>

#include "msdf-atlas-gen.h"

using namespace msdf_atlas;

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define DEFAULT_MITER_LIMIT 1.0
#define DEFAULT_EM_SIZE 64
#define DEFAULT_PIXEL_RANGE 8.0
#define SDF_ERROR_ESTIMATE_PRECISION 19
#define GLYPH_FILL_RULE msdfgen::FILL_NONZERO
#define MCG_MULTIPLIER 6364136223846793005ull



static char toupper(char c) {
    return c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
}

static bool cmpExtension(const char* path, const char* ext) {
    for (const char* a = path + strlen(path) - 1, *b = ext + strlen(ext) - 1; b >= ext; --a, --b)
        if (a < path || toupper(*a) != toupper(*b))
            return false;
    return true;
}

static void loadGlyphs(std::vector<GlyphGeometry>& glyphs, msdfgen::FontHandle* font, const Charset& charset) {
    glyphs.clear();
    glyphs.reserve(charset.size());
    for (unicode_t cp : charset) {
        GlyphGeometry glyph;
        if (glyph.load(font, cp))
            glyphs.push_back((GlyphGeometry&&)glyph);
        else
            printf("Glyph for codepoint 0x%X missing\n", cp);
    }
}

struct Configuration {
    ImageType imageType;
    ImageFormat imageFormat;
    int width, height;
    double emSize;
    double pxRange;
    double angleThreshold;
    double miterLimit;
    unsigned long long coloringSeed;
    GeneratorAttributes generatorAttributes;
    int threadCount;
    const char* arteryFontFilename;
    const char* imageFilename;
    const char* jsonFilename;
    const char* csvFilename;
    const char* shadronPreviewFilename;
    const char* shadronPreviewText;
};

template <typename T, typename S, int N, GeneratorFunction<S, N> GEN_FN>
static bool makeAtlas(const std::vector<GlyphGeometry>& glyphs, msdfgen::FontHandle* font, const Configuration& config) {
    ImmediateAtlasGenerator<S, N, GEN_FN, BitmapAtlasStorage<T, N> > generator(config.width, config.height);
    generator.setAttributes(config.generatorAttributes);
    generator.setThreadCount(config.threadCount);
    generator.generate(glyphs.data(), glyphs.size());
    msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();

    bool success = true;

    if (config.imageFilename) {
        if (saveImage(bitmap, config.imageFormat, config.imageFilename))
            puts("Atlas image file saved.");
        else {
            success = false;
            puts("Failed to save the atlas as an image file.");
        }
    }

    if (config.arteryFontFilename) {
        if (exportArteryFont<float>(font, glyphs.data(), glyphs.size(), config.emSize, config.pxRange, bitmap, config.imageType, config.imageFormat, config.arteryFontFilename))
            puts("Artery Font file generated.");
        else {
            success = false;
            puts("Failed to generate Artery Font file.");
        }
    }

    return success;
}

int MsdfWrapper::load_font(const std::string& path, const std::string &out_img, const std::string &out_csv) {
#define ABORT(msg) { puts(msg); return 1; }

    int result = 0;
    Configuration config = { };
    const char* fontFilename = nullptr;
    const char* charsetFilename = nullptr;
    config.imageType = ImageType::MSDF;
    config.imageFormat = ImageFormat::UNSPECIFIED;
    const char* imageFormatName = nullptr;
    int fixedWidth = -1, fixedHeight = -1;
    config.generatorAttributes.overlapSupport = true;
    config.generatorAttributes.scanlinePass = true;
    config.generatorAttributes.errorCorrectionThreshold = MSDFGEN_DEFAULT_ERROR_CORRECTION_THRESHOLD;
    double minEmSize = 0;
    enum {
        /// Range specified in EMs
        RANGE_EM,
        /// Range specified in output pixels
        RANGE_PIXEL,
    } rangeMode = RANGE_PIXEL;
    double rangeValue = 0;
    TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
    config.angleThreshold = DEFAULT_ANGLE_THRESHOLD;
    config.miterLimit = DEFAULT_MITER_LIMIT;
    config.threadCount = std::max((int)std::thread::hardware_concurrency(), 1);
    config.imageType = ImageType::MSDF;
    config.imageFormat = ImageFormat::BMP;

    // image out
    config.imageFilename = out_img.c_str();
    // csv out
    config.csvFilename = out_csv.c_str();
    atlasSizeConstraint = TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
    fixedWidth = -1, fixedHeight = -1;

    fontFilename = path.c_str();

    // Parse command line
    if (!fontFilename)
        ABORT("No font specified.");
    if (!(config.arteryFontFilename || config.imageFilename || config.jsonFilename || config.csvFilename || config.shadronPreviewFilename)) {
        puts("No output specified.");
        return 0;
    }
    bool layoutOnly = !(config.arteryFontFilename || config.imageFilename);

    // Fix up configuration based on related values
    if (!(config.imageType == ImageType::PSDF || config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF))
        config.miterLimit = 0;
    if (config.emSize > minEmSize)
        minEmSize = config.emSize;
    if (!(fixedWidth > 0 && fixedHeight > 0) && !(minEmSize > 0)) {
        puts("Neither atlas size nor glyph size selected, using default...");
        minEmSize = DEFAULT_EM_SIZE;
    }
    if (!(config.imageType == ImageType::SDF || config.imageType == ImageType::PSDF || config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF)) {
        rangeMode = RANGE_PIXEL;
        rangeValue = (double)(config.imageType == ImageType::SOFT_MASK);
    }
    else if (rangeValue <= 0) {
        rangeMode = RANGE_PIXEL;
        rangeValue = DEFAULT_PIXEL_RANGE;
    }

    // Finalize image format
    ImageFormat imageExtension = ImageFormat::UNSPECIFIED;
    if (config.imageFilename) {
        if (cmpExtension(config.imageFilename, ".png")) imageExtension = ImageFormat::PNG;
        else if (cmpExtension(config.imageFilename, ".bmp")) imageExtension = ImageFormat::BMP;
        else if (cmpExtension(config.imageFilename, ".tif") || cmpExtension(config.imageFilename, ".tiff")) imageExtension = ImageFormat::TIFF;
        else if (cmpExtension(config.imageFilename, ".txt")) imageExtension = ImageFormat::TEXT;
        else if (cmpExtension(config.imageFilename, ".bin")) imageExtension = ImageFormat::BINARY;
    }
    if (config.imageFormat == ImageFormat::UNSPECIFIED) {
        config.imageFormat = ImageFormat::PNG;
        imageFormatName = "png";
        // If image format is not specified and -imageout is the only image output, infer format from its extension
        if (imageExtension != ImageFormat::UNSPECIFIED && !config.arteryFontFilename)
            config.imageFormat = imageExtension;
    }
    if (config.imageType == ImageType::MTSDF && config.imageFormat == ImageFormat::BMP)
        ABORT("Atlas type not compatible with image format. MTSDF requires a format with alpha channel.");
    if (config.arteryFontFilename && !(config.imageFormat == ImageFormat::PNG || config.imageFormat == ImageFormat::BINARY || config.imageFormat == ImageFormat::BINARY_FLOAT)) {
        config.arteryFontFilename = nullptr;
        result = 1;
        puts("Error: Unable to create an Artery Font file with the specified image format!");
        // Recheck whether there is anything else to do
        if (!(config.arteryFontFilename || config.imageFilename || config.jsonFilename || config.csvFilename || config.shadronPreviewFilename))
            return result;
        layoutOnly = !(config.arteryFontFilename || config.imageFilename);
    }
    if (imageExtension != ImageFormat::UNSPECIFIED) {
        // Warn if image format mismatches -imageout extension
        bool mismatch = false;
        switch (config.imageFormat) {
        case ImageFormat::TEXT: case ImageFormat::TEXT_FLOAT:
            mismatch = imageExtension != ImageFormat::TEXT;
            break;
        case ImageFormat::BINARY: case ImageFormat::BINARY_FLOAT: case ImageFormat::BINARY_FLOAT_BE:
            mismatch = imageExtension != ImageFormat::BINARY;
            break;
        default:
            mismatch = imageExtension != config.imageFormat;
        }
        if (mismatch)
            printf("Warning: Output image file extension does not match the image's actual format (%s)!\n", imageFormatName);
    }
    imageFormatName = nullptr; // No longer consistent with imageFormat

    // Load font
    class FontHolder {
        msdfgen::FreetypeHandle* ft;
        msdfgen::FontHandle* font;
    public:
        explicit FontHolder(const char* fontFilename) : ft(nullptr), font(nullptr) {
            if ((ft = msdfgen::initializeFreetype()))
                font = msdfgen::loadFont(ft, fontFilename);
        }
        ~FontHolder() {
            if (ft) {
                if (font)
                    msdfgen::destroyFont(font);
                msdfgen::deinitializeFreetype(ft);
            }
        }
        operator msdfgen::FontHandle* () const {
            return font;
        }
    } font(fontFilename);
    if (!font)
        ABORT("Failed to load specified font file.");
    msdfgen::FontMetrics fontMetrics = { };
    msdfgen::getFontMetrics(fontMetrics, font);
    if (fontMetrics.emSize <= 0)
        fontMetrics.emSize = DEFAULT_EM_SIZE;

    // Load character set
    Charset charset;
    for (unicode_t cp = 0x20; cp < 0x7f; ++cp)
        charset.add(cp);


    if (charset.empty())
        ABORT("No character set loaded.");

    // Load glyphs
    std::vector<GlyphGeometry> glyphs;
    loadGlyphs(glyphs, font, charset);
    printf("Loaded geometry of %d out of %d characters.\n", (int)glyphs.size(), (int)charset.size());

    // Determine final atlas dimensions, scale and range, pack glyphs
    {
        double unitRange = 0, pxRange = 0;
        switch (rangeMode) {
        case RANGE_EM:
            unitRange = rangeValue * fontMetrics.emSize;
            break;
        case RANGE_PIXEL:
            pxRange = rangeValue;
            break;
        }
        bool fixedDimensions = fixedWidth >= 0 && fixedHeight >= 0;
        bool fixedScale = config.emSize > 0;
        TightAtlasPacker atlasPacker;
        if (fixedDimensions)
            atlasPacker.setDimensions(fixedWidth, fixedHeight);
        else
            atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
        atlasPacker.setPadding(config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF ? 0 : -1);
        // TODO: In this case (if padding is -1), the border pixels of each glyph are black, but still computed. For floating-point output, this may play a role.
        if (fixedScale)
            atlasPacker.setScale(config.emSize / fontMetrics.emSize);
        else
            atlasPacker.setMinimumScale(minEmSize / fontMetrics.emSize);
        atlasPacker.setPixelRange(pxRange);
        atlasPacker.setUnitRange(unitRange);
        atlasPacker.setMiterLimit(config.miterLimit);
        if (int remaining = atlasPacker.pack(glyphs.data(), glyphs.size())) {
            if (remaining < 0) {
                ABORT("Failed to pack glyphs into atlas.");
            }
            else {
                printf("Error: Could not fit %d out of %d glyphs into the atlas.\n", remaining, (int)glyphs.size());
                return 1;
            }
        }
        atlasPacker.getDimensions(config.width, config.height);
        if (!(config.width > 0 && config.height > 0))
            ABORT("Unable to determine atlas size.");
        config.emSize = atlasPacker.getScale() * fontMetrics.emSize;
        config.pxRange = atlasPacker.getPixelRange();
        if (!fixedScale)
            printf("Glyph size: %.9g pixels/EM\n", config.emSize);
        if (!fixedDimensions)
            printf("Atlas dimensions: %d x %d\n", config.width, config.height);
    }

    // Generate atlas bitmap
    if (!layoutOnly) {

        // Edge coloring
        if (config.imageType == ImageType::MSDF || config.imageType == ImageType::MTSDF) {
            unsigned long long glyphSeed = config.coloringSeed;
            for (GlyphGeometry& glyph : glyphs) {
                glyphSeed *= MCG_MULTIPLIER;
                glyph.edgeColoring(config.angleThreshold, glyphSeed);
            }
        }

        bool floatingPoint = (
            config.imageFormat == ImageFormat::TIFF ||
            config.imageFormat == ImageFormat::TEXT_FLOAT ||
            config.imageFormat == ImageFormat::BINARY_FLOAT ||
            config.imageFormat == ImageFormat::BINARY_FLOAT_BE
            );

        bool success = false;
        switch (config.imageType) {
        case ImageType::HARD_MASK:
            if (floatingPoint)
                success = makeAtlas<float, float, 1, scanlineGenerator>(glyphs, font, config);
            else
                success = makeAtlas<byte, float, 1, scanlineGenerator>(glyphs, font, config);
            break;
        case ImageType::SOFT_MASK:
        case ImageType::SDF:
            if (floatingPoint)
                success = makeAtlas<float, float, 1, sdfGenerator>(glyphs, font, config);
            else
                success = makeAtlas<byte, float, 1, sdfGenerator>(glyphs, font, config);
            break;
        case ImageType::PSDF:
            if (floatingPoint)
                success = makeAtlas<float, float, 1, psdfGenerator>(glyphs, font, config);
            else
                success = makeAtlas<byte, float, 1, psdfGenerator>(glyphs, font, config);
            break;
        case ImageType::MSDF:
            if (floatingPoint)
                success = makeAtlas<float, float, 3, msdfGenerator>(glyphs, font, config);
            else
                success = makeAtlas<byte, float, 3, msdfGenerator>(glyphs, font, config);
            break;
        case ImageType::MTSDF:
            if (floatingPoint)
                success = makeAtlas<float, float, 4, mtsdfGenerator>(glyphs, font, config);
            else
                success = makeAtlas<byte, float, 4, mtsdfGenerator>(glyphs, font, config);
            break;
        }
        if (!success)
            result = 1;
    }

    if (config.csvFilename) {
        if (exportCSV(glyphs.data(), glyphs.size(), fontMetrics.emSize, config.csvFilename))
            puts("Glyph layout written into CSV file.");
        else {
            result = 1;
            puts("Failed to write CSV output file.");
        }
    }
    if (config.jsonFilename) {
        if (exportJSON(font, glyphs.data(), glyphs.size(), config.emSize, config.pxRange, config.width, config.height, config.imageType, config.jsonFilename))
            puts("Glyph layout and metadata written into JSON file.");
        else {
            result = 1;
            puts("Failed to write JSON output file.");
        }
    }

    if (config.shadronPreviewFilename && config.shadronPreviewText) {
        std::vector<unicode_t> previewText;
        utf8Decode(previewText, config.shadronPreviewText);
        previewText.push_back(0);
        if (generateShadronPreview(font, glyphs.data(), glyphs.size(), config.imageType, config.width, config.height, config.pxRange, previewText.data(), config.imageFilename, config.shadronPreviewFilename))
            puts("Shadron preview script generated.");
        else {
            result = 1;
            puts("Failed to generate Shadron preview file.");
        }
    }

    return result;
}
