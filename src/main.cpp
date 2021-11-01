#include <fstream>
#include <vector>

#include <libzraw.h>
#include <quick_arg_parser.hpp>

#define TINY_DNG_WRITER_IMPLEMENTATION
#include <examples/dngwriter/tiny_dng_writer.h>

struct Args : MainArguments<Args>
{
    using MainArguments<Args>::MainArguments;

    std::string input = nonstandardOption("-input", 'i', "Input ZRAW frame").validator(
        [] (std::string input)
        {
            return input != "";
        }
    );

    std::string output = nonstandardOption("-output", 'o', "Output DNG frame").validator(
        [] (std::string output)
        {
            return output != "";
        }
    );

    bool verbose = option("verbose", 'v', "Verbose") = false;
};

int main(int argc, char** argv)
{
    Args args {argc, argv};
    if (args.verbose)
    {
        std::cout << "Input file: " << args.input << std::endl;
        std::cout << "Output file: " << args.output << std::endl;
    }

    // Open file and get size
    std::ifstream file(args.input, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read ZRAW file to buffer
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
        return -1;

    // Create ZRAW decoder
    auto decoder = zraw_decoder__create();

    // Read frame from buffer
    auto reading_state = zraw_decoder__read_hisi_frame(decoder, buffer.data(), buffer.size());

    zraw_frame_info_t frame_info;
    auto info_state = zraw_decoder__get_hisi_frame_info(decoder, frame_info);

    // Decode frame
    auto decompression_state = zraw_decoder__decompress_hisi_frame(decoder);

    tinydngwriter::DNGImage dng_image;
    dng_image.SetBigEndian(false);

    dng_image.SetSubfileType(false, false, false);
    dng_image.SetImageWidth(frame_info.width_in_photodiodes);
    dng_image.SetImageLength(frame_info.height_in_photodiodes);
    dng_image.SetRowsPerStrip(frame_info.height_in_photodiodes);
    dng_image.SetSamplesPerPixel(1);

    // Bits Per Photodiode value
    uint16_t bps[1] = {(uint16_t)frame_info.bits_per_photodiode_value};
    dng_image.SetBitsPerSample(1, bps);

    dng_image.SetPlanarConfig(tinydngwriter::PLANARCONFIG_CONTIG);
    dng_image.SetCompression(tinydngwriter::COMPRESSION_NONE);
    dng_image.SetPhotometric(tinydngwriter::PHOTOMETRIC_CFA);
    dng_image.SetXResolution(300.0);
    dng_image.SetYResolution(300.0);
    dng_image.SetOrientation(tinydngwriter::ORIENTATION_TOPLEFT);
    dng_image.SetResolutionUnit(tinydngwriter::RESUNIT_NONE);
    dng_image.SetImageDescription("[Storyboard Creativity] ZRAW -> DNG converter generated image.");

    dng_image.SetUniqueCameraModel("Z CAM E2");

    // CM1
    double matrix1[] =
    {
        0.9784, -0.4995, 0.003,
        -0.3625, 1.1454, 0.2475,
        -0.0961, 0.2097, 0.6377
    };
    dng_image.SetColorMatrix1(3, matrix1);
    dng_image.SetCalibrationIlluminant1(17);

    // CM2
    double matrix2[] =
    {
        0.6770, -0.1895, -0.0744,
        -0.5232, 1.3145, 0.2303,
        -0.1664, 0.2691, 0.5703
    };
    dng_image.SetColorMatrix2(3, matrix2);
    dng_image.SetCalibrationIlluminant2(21);
    
    // We set analog WB as neutral
    double analog_balance[] = {1.0, 1.0, 1.0};
    dng_image.SetAnalogBalance(3, analog_balance);

    // We set post WB according to awb(or wb) gains
    double rgain = frame_info.awb_gain_r;
    double ggain = frame_info.awb_gain_g;
    double bgain = frame_info.awb_gain_b;
    double wbalance[3] =
    {
        (1.0 * ggain / rgain),
        (1.0),
        (1.0 * ggain / bgain)
    };
    dng_image.SetAsShotNeutral(3, wbalance);

    // Black Levels
    dng_image.SetBlackLevelRepeatDim(2, 2);
    dng_image.SetBlackLevel(4, frame_info.cfa_black_levels);

    dng_image.SetDNGVersion(1, 2, 0, 0);

    dng_image.SetCFARepeatPatternDim(2, 2);

    uint8_t cfa_pattern[4] = {0, 1, 1, 2};
    dng_image.SetCFAPattern(4, cfa_pattern);

    double white_levels[1] = {(double)((1 << frame_info.bits_per_photodiode_value) - 1)};
    dng_image.SetWhiteLevelRational(1, white_levels);

    // Get CFA data
    std::vector<uint16_t> image_data(frame_info.width_in_photodiodes * frame_info.height_in_photodiodes);
    auto cfa_state = zraw_decoder__get_decompressed_CFA(decoder, image_data.data(), image_data.size() * sizeof(uint16_t));

    // Release ZRAW decoder
    zraw_decoder__free(decoder);
   
    dng_image.SetImageDataPacked(image_data.data(), image_data.size(), frame_info.bits_per_photodiode_value, true);

    tinydngwriter::DNGWriter dng_writer(false);
    dng_writer.AddImage(&dng_image);

    std::string err;
    dng_writer.WriteToFile(args.output.c_str(), &err);

    return 0;
}