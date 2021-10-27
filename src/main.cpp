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

    auto decoder = zraw_decoder__create();
    zraw_decoder__free(decoder);

    return 0;
}