#include <iostream>

#include <dtest/application.hh>
#include <dtest/python.hh>

int main(int argc, char* argv[]) {
    int ret = 0;
    using namespace python;
    try {
        dts::python::init();
        interpreter_guard g;
        set_arguments(argc, argv);
        load(argv[1]);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        dts::python::terminate();
        ret = 1;
    }
    return ret;
    /*
       int ret = 0;
       dts::application app;
       try {
       app.init(argc, argv);
       ret = run(app);
       } catch (const std::exception& err) {
       std::cerr << err.what() << std::endl;
       app.terminate();
       ret = 1;
       }
       return ret;
       */
}
