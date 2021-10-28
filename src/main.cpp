#include <fstream>
#include <vector>

#include <libzraw.h>
#include <quick_arg_parser.hpp>

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
    auto state = zraw_decoder__read_hisi_frame(decoder, buffer.data(), buffer.size());

    // Decode frame
    auto decompression_state = zraw_decoder__decompress_hisi_frame(decoder);

    // Release ZRAW decoder
    zraw_decoder__free(decoder);

    return 0;
}