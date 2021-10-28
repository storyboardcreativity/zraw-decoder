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

    // Release ZRAW decoder
    zraw_decoder__free(decoder);




    tinydngwriter::DNGImage dng_image;
    dng_image.SetBigEndian(false);

    dng_image.SetSubfileType(false, false, false);
    dng_image.SetImageWidth(frame_info.width_in_photodiodes);
    dng_image.SetImageLength(frame_info.height_in_photodiodes);
    dng_image.SetRowsPerStrip(frame_info.height_in_photodiodes);
    dng_image.SetSamplesPerPixel(1);
    uint16_t bps[1] = {frame_info.bits_per_photodiode_value};
    dng_image.SetBitsPerSample(1, bps);
    dng_image.SetPlanarConfig(tinydngwriter::PLANARCONFIG_CONTIG);
    dng_image.SetCompression(tinydngwriter::COMPRESSION_NONE);
    dng_image.SetPhotometric(tinydngwriter::PHOTOMETRIC_CFA);
    dng_image.SetXResolution(300.0);
    dng_image.SetYResolution(300.0);
    dng_image.SetResolutionUnit(tinydngwriter::RESUNIT_NONE);
    dng_image.SetImageDescription("[Storyboard Creativity] ZRAW -> DNG converter generated image.");

    std::vector<uint16_t> image_data(frame_info.width_in_photodiodes * frame_info.height_in_photodiodes);
    for (int y = 0; y < frame_info.height_in_photodiodes; ++y)
        for (int x = 0; x < frame_info.width_in_photodiodes; ++x)
            image_data[y * frame_info.width_in_photodiodes + x] = rand();

    dng_image.SetImageData(reinterpret_cast<unsigned char *>(image_data.data()), image_data.size() * sizeof(uint16_t));

    tinydngwriter::DNGWriter dng_writer(false);
    dng_writer.AddImage(&dng_image);

    std::string err;
    dng_writer.WriteToFile(args.output.c_str(), &err);

    return 0;
}