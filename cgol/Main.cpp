#include "Conway.h"

int main(int argc, char **argv)
{
    Conway* conway = new Conway("Conway\'s Game of Life");
    conway->initialize();
    conway->run();
    return 0;
}