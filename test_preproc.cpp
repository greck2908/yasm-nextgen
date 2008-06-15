#include <cstdlib>
#include <iostream>
#include <sstream>

#include <libyasmx/errwarn.h>
#include <libyasmx/errwarns.h>
#include <libyasmx/registry.h>
#include <libyasmx/linemap.h>
#include <libyasmx/preproc.h>

using namespace yasm;

YASM_STATIC_MODULE_REF(preproc, raw)

int
main()
{
    std::auto_ptr<Preprocessor> preproc = load_module<Preprocessor>("raw");
    std::string instr("test text");
    std::istringstream iss(instr);
    Linemap linemap;
    Errwarns errwarns;
    preproc->init(iss, "<string>", linemap, errwarns);

    std::string outstr;
    if (!preproc->get_line(outstr))
        return EXIT_FAILURE;

    std::cout << outstr << std::endl;

    if (outstr != instr)
        return EXIT_FAILURE;

    if (preproc->get_line(outstr))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
