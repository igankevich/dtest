#include <iostream>
#include <string>

#include <dtest/application.hh>
#include <dtest/config.hh>
#include <dtest/python.hh>

void usage() {
    std::cout << "usage: dtest-python [-h] [--help] [--version] python-script\n";
}

void parse_arguments(int argc, char** argv) {
    if (argc != 2) { usage(); std::exit(1); }
    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help") { usage(); std::exit(0); }
    if (arg1 == "--version") { std::cout << DTEST_VERSION << '\n'; std::exit(0); }
}

int main(int argc, char* argv[]) {
    int ret = 0;
    parse_arguments(argc, argv);
    using namespace python;
    try {
        dts::python::init();
        interpreter_guard g;
        dts::application app;
        dts::python::application(&app);
        set_arguments(argc, argv);
        load(argv[1]);
        ret = dts::python::application_exit_code();
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        dts::python::terminate();
        ret = 1;
    }
    return ret;
}
