#include <cstdio>
#include <iostream>
#include <vector>

#include <dtest/application.hh>
#include <dtest/python.hh>

namespace {

    PyMethodDef dts_methods[] = {
        {
            .ml_name = "hello",
            .ml_meth = (PyCFunction) dts::python::hello,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Hello world!"
        },
        {
            .ml_name = "cluster",
            .ml_meth = (PyCFunction) dts::python::cluster,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Configure cluster"
        },
        {
            .ml_name = "exit_code",
            .ml_meth = (PyCFunction) dts::python::exit_code,
            .ml_flags = METH_VARARGS,
            .ml_doc = "How to interpret exit code?"
        },
        {
            .ml_name = "add_process",
            .ml_meth = (PyCFunction) dts::python::add_process,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Executed the process on the specified cluster node"
        },
        {
            .ml_name = "run",
            .ml_meth = (PyCFunction) dts::python::run,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Run test suite."
        },
        {nullptr, nullptr, 0, nullptr}
    };

    PyModuleDef dts_module = {
        PyModuleDef_HEAD_INIT,
        .m_name = "dtest",
        .m_doc = "Distributed unit test framework.",
        .m_size = -1,
        .m_methods = dts_methods
    };

    PyMODINIT_FUNC dts_init() {
        return PyModule_Create(&dts_module);
    }

    constexpr const char* cluster_keywords[] = {
        "name",
        "size",
        //"network",
        //"peer_network",
        nullptr};

    constexpr const char* add_process_keywords[] = {"nodes", "args", nullptr};

    dts::application python_application;

}

void python::set_arguments(int argc, char** argv) {
    std::vector<pointer<wchar_t>> tmp;
    tmp.reserve(argc-1);
    std::vector<wchar_t*> args;
    args.reserve(argc-1);
    for (int i=1; i<argc; ++i) {
        tmp.emplace_back(Py_DecodeLocale(argv[i], nullptr));
        args.emplace_back(tmp.back().get());
    }
    ::PySys_SetArgvEx(args.size(), args.data(), 0);
}

void python::load(const char* filename) {
    auto fp = std::fopen(filename, "rb");
    ::PyCompilerFlags flags{};
    ::PyRun_SimpleFileExFlags(fp, filename, true, &flags);
}

void python::add_builtin_module(const char* name, PyObject* (*init)()) {
    PyImport_AppendInittab(name, init);
}

void dts::python::init() {
    ::python::add_builtin_module("dtest", &dts_init);
}

void dts::python::terminate() {
    python_application.terminate();
}

PyObject* dts::python::hello(PyObject* self, PyObject* args, PyObject* kwds) {
    std::cout << "Hello world!" << std::endl;
    Py_RETURN_NONE;
}

PyObject* dts::python::cluster(PyObject* self, PyObject* args, PyObject* kwds) {
    const char* name = nullptr;
    unsigned long size = 2;
    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sk", const_cast<char**>(cluster_keywords),
        &name, &size)) {
        return nullptr;
    }
    dts::cluster c;
    c.name(name);
    c.generate_nodes(size);
    python_application.cluster(std::move(c));
    Py_RETURN_NONE;
}

PyObject* dts::python::exit_code(PyObject* self, PyObject* args, PyObject* kwds) {
    const char* s = nullptr;
    if (!PyArg_ParseTuple(args, "|s", &s)) {
        return nullptr;
    }
    python_application.exit_code(dts::to_exit_code(s));
    Py_RETURN_NONE;
}

PyObject* dts::python::add_process(PyObject* self, PyObject* args, PyObject* kwds) {
    PyObject* py_args = nullptr;
    PyObject* py_nodes = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO",
                                     const_cast<char**>(add_process_keywords),
                                     &py_nodes, &py_args)) {
        return nullptr;
    }
    std::vector<bool> bits(python_application.cluster().size(), false);
    {
        ::python::object py_sequence = PySequence_Fast(py_nodes, "expected a sequence");
        const int py_sequence_size = PySequence_Size(py_nodes);
        for (int i=0; i<py_sequence_size; ++i) {
            ::python::object item = PySequence_Fast_GET_ITEM(py_nodes, i);
            bits[PyLong_AsSsize_t(PyNumber_Long(item.get()))] = true;
        }
    }
    dts::cluster_node_bitmap cpp_nodes(bits);
    sys::argstream cpp_args;
    {
        ::python::object py_sequence = PySequence_Fast(py_args, "expected a sequence");
        const int py_sequence_size = PySequence_Size(py_args);
        for (int i=0; i<py_sequence_size; ++i) {
            ::python::object item = PySequence_Fast_GET_ITEM(py_args, i);
            cpp_args.append(PyUnicode_AsUTF8(PyObject_Str(item.get())));
        }
    }
    python_application.add_process(std::move(cpp_nodes), std::move(cpp_args));
    Py_RETURN_NONE;
}

PyObject* dts::python::run(PyObject* self, PyObject* args, PyObject* kwds) {
    return PyLong_FromLong(dts::run(python_application));
}
