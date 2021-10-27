#include <libzraw.h>

int main(int argc, char** argv)
{
    auto decoder = zraw_decoder__create();
    zraw_decoder__free(decoder);

    return 0;
}