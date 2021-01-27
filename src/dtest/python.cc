#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>

#include <dtest/application.hh>
#include <dtest/python.hh>

namespace {

    PyMethodDef dts_methods[] = {
        {
            .ml_name = "cluster",
            .ml_meth = (PyCFunction) dts::python::cluster,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Configure the cluster size, network, peer_network and name."
        },
        {
            .ml_name = "exit_code",
            .ml_meth = (PyCFunction) dts::python::exit_code,
            .ml_flags = METH_VARARGS,
            .ml_doc = "How exit code of child processes is accumulated? "
                "Possible values: 'all', 'master', process no. starting from 1."
        },
        {
            .ml_name = "add_process",
            .ml_meth = (PyCFunction) dts::python::add_process,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Execute the process on the specified cluster node. "
                "Works only before the tests are started. "
        },
        {
            .ml_name = "run_process",
            .ml_meth = (PyCFunction) dts::python::run_process,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Execute the process on the specified cluster node. "
                "Works only after the tests are started. "
                "Call it from any test to run additional processes "
                "(e.g. submit the application to the scheduler)."
        },
        {
            .ml_name = "kill_node",
            .ml_meth = (PyCFunction) dts::python::kill_node,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Kill every process running on the specified node with the signal. "
                "Use this function to simulate cluster node failure. "
                "The signal is configurable."
        },
        {
            .ml_name = "add_test",
            .ml_meth = (PyCFunction) dts::python::add_test,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Add unit test that checks output of all processes. "
                "Dtest colects stderr/stdout output from every process in the cluster and "
                "prepends node name to each line. "
                "Finding specific lines or specific sequence of lines allows to check "
                "events that occur in the application."
        },
        {
            .ml_name = "will_restart",
            .ml_meth = (PyCFunction) dts::python::will_restart,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc =
                "Restart processes after test completion and run tests the second time. "
                "Useful when testing power outage."
        },
        {
            .ml_name = "execution_delay",
            .ml_meth = (PyCFunction) dts::python::execution_delay,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Process execution delay in milliseconds. "
                "The amount of time between execution of the processes on the successive nodes. "
        },
        {
            .ml_name = "run",
            .ml_meth = (PyCFunction) dts::python::run,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Run test suite."
        },
        {
            .ml_name = "fail",
            .ml_meth = (PyCFunction) dts::python::fail,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Fail the current test."
        },
        {
            .ml_name = "expect_event_sequence",
            .ml_meth = (PyCFunction) dts::python::expect_event_sequence,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Check that the specified sequence of events ocurred in the processes. "
                "Events are specified as regular expressions (strings)."
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
        "size",
        "name",
        "network",
        "peer_network",
        nullptr};

    constexpr const char* add_process_keywords[] = {"nodes", "args", nullptr};

    dts::application* python_application = nullptr;
    int python_exit_code = 0;

    inline dts::cluster_node::address_type string_to_network(const char* s) {
        dts::cluster_node::address_type net;
        std::stringstream tmp;
        tmp << s;
        tmp >> net;
        return net;
    }

    dts::cluster_node_bitmap object_to_cluster_node_bitmap(PyObject* py_nodes) {
        std::vector<bool> bits(python_application->cluster().size(), false);
        {
            ::python::object py_sequence = PySequence_Fast(py_nodes, "expected a sequence");
            const int py_sequence_size = PySequence_Size(py_nodes);
            for (int i=0; i<py_sequence_size; ++i) {
                ::python::object item = PySequence_Fast_GET_ITEM(py_nodes, i);
                bits[PyLong_AsSsize_t(PyNumber_Long(item.get()))] = true;
            }
        }
        return dts::cluster_node_bitmap(bits);
    }

    std::vector<std::string> object_to_string_array(PyObject* py_list) {
        ::python::object py_sequence = PySequence_Fast(py_list, "expected a sequence");
        const int py_sequence_size = PySequence_Size(py_list);
        std::vector<std::string> cpp_list;
        cpp_list.reserve(py_sequence_size);
        for (int i=0; i<py_sequence_size; ++i) {
            ::python::object item = PySequence_Fast_GET_ITEM(py_list, i);
            cpp_list.emplace_back(PyUnicode_AsUTF8(PyObject_Str(item.get())));
        }
        return cpp_list;
    }

    PyObject* get_nodes_and_arguments(PyObject* args, PyObject* kwds,
                                      dts::cluster_node_bitmap& cpp_nodes,
                                      sys::argstream& cpp_args) {
        PyObject* py_args = nullptr;
        PyObject* py_nodes = nullptr;
        if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO",
                                         const_cast<char**>(add_process_keywords),
                                         &py_nodes, &py_args)) {
            return nullptr;
        }
        cpp_nodes = object_to_cluster_node_bitmap(py_nodes);
        {
            ::python::object py_sequence = PySequence_Fast(py_args, "expected a sequence");
            const int py_sequence_size = PySequence_Size(py_args);
            for (int i=0; i<py_sequence_size; ++i) {
                ::python::object item = PySequence_Fast_GET_ITEM(py_args, i);
                cpp_args.append(PyUnicode_AsUTF8(PyObject_Str(item.get())));
            }
        }
        Py_RETURN_NONE;
    }

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
    if (!fp) { throw std::system_error(errno, std::generic_category()); }
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
    python_application->terminate();
}

PyObject* dts::python::cluster(PyObject* self, PyObject* args, PyObject* kwds) {
    const char* name = nullptr;
    const char* network = nullptr;
    const char* peer_network = nullptr;
    unsigned long size = 2;
    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "k|sss", const_cast<char**>(cluster_keywords),
        &size, &name, &network, &peer_network)) {
        return nullptr;
    }
    dts::cluster c;
    c.name(name);
    if (network) { c.network(string_to_network(network)); }
    if (peer_network) { c.peer_network(string_to_network(peer_network)); }
    c.generate_nodes(size);
    python_application->cluster(std::move(c));
    Py_RETURN_NONE;
}

PyObject* dts::python::exit_code(PyObject* self, PyObject* args, PyObject* kwds) {
    const char* s = nullptr;
    if (!PyArg_ParseTuple(args, "s", &s)) {
        return nullptr;
    }
    python_application->exit_code(dts::to_exit_code(s));
    Py_RETURN_NONE;
}

PyObject* dts::python::add_process(PyObject* self, PyObject* args, PyObject* kwds) {
    dts::cluster_node_bitmap cpp_nodes;
    sys::argstream cpp_args;
    auto ret = get_nodes_and_arguments(args, kwds, cpp_nodes, cpp_args);
    if (!ret) { return ret; }
    python_application->add_process(std::move(cpp_nodes), std::move(cpp_args));
    Py_RETURN_NONE;
}

PyObject* dts::python::run_process(PyObject* self, PyObject* args, PyObject* kwds) {
    dts::cluster_node_bitmap cpp_nodes;
    sys::argstream cpp_args;
    auto ret = get_nodes_and_arguments(args, kwds, cpp_nodes, cpp_args);
    if (!ret) { return ret; }
    python_application->run_process(std::move(cpp_nodes), std::move(cpp_args));
    Py_RETURN_NONE;
}

PyObject* dts::python::kill_node(PyObject* self, PyObject* args, PyObject* kwds) {
    int value = 0;
    PyObject* py_nodes = nullptr;
    if (!PyArg_ParseTuple(args, "O|k", &py_nodes, &value)) {
        return nullptr;
    }
    auto nodes = object_to_cluster_node_bitmap(py_nodes);
    python_application->kill_process(std::move(nodes), sys::signal(value));
    Py_RETURN_NONE;
}

PyObject* dts::python::add_test(PyObject* self, PyObject* args, PyObject* kwds) {
    const char* description = nullptr;
    PyObject* py_test = nullptr;
    if (!PyArg_ParseTuple(args, "sO", &description, &py_test)) {
        return nullptr;
    }
    ::python::object py_test_copy(py_test);
    py_test_copy.retain();
    python_application->emplace_test(
        description, [py_test_copy] (dts::application&, const dts::string_array& lines) mutable {
            const int n = lines.size();
            PyObject* py_lines = PyList_New(n);
            for (int i=0; i<n; ++i) {
                PyList_SetItem(py_lines, i, PyUnicode_FromString(lines[i].data()));
            }
            PyObject_CallFunctionObjArgs(py_test_copy.get(),
                                         py_lines,
                                         nullptr);
        });
    Py_RETURN_NONE;
}

PyObject* dts::python::will_restart(PyObject* self, PyObject* args, PyObject* kwds) {
    int value = 0;
    if (!PyArg_ParseTuple(args, "p", &value)) {
        return nullptr;
    }
    python_application->will_restart(bool(value));
    Py_RETURN_NONE;
}

PyObject* dts::python::execution_delay(PyObject* self, PyObject* args, PyObject* kwds) {
    unsigned long value = 0;
    if (!PyArg_ParseTuple(args, "k", &value)) { return nullptr; }
    python_application->execution_delay(std::chrono::milliseconds(value));
    Py_RETURN_NONE;
}

PyObject* dts::python::run(PyObject* self, PyObject* args, PyObject* kwds) {
    python_exit_code = dts::run(*python_application);
    return PyLong_FromLong(python_exit_code);
}

void dts::python::application(::dts::application* ptr) {
    python_application = ptr;
}

int dts::python::application_exit_code() {
    return python_exit_code;
}

PyObject* dts::python::fail(PyObject* self, PyObject* args, PyObject* kwds) {
    const char* reason = nullptr;
    if (!PyArg_ParseTuple(args, "s", &reason)) { return nullptr; }
    std::string what;
    what.reserve(std::char_traits<char>::length(reason)+1);
    what += reason;
    what += '\n';
    throw std::runtime_error(std::move(what));
    Py_RETURN_NONE;
}

PyObject* dts::python::expect_event_sequence(PyObject* self, PyObject* args, PyObject* kwds) {
    PyObject* py_lines = nullptr;
    PyObject* py_events = nullptr;
    if (!PyArg_ParseTuple(args, "OO", &py_lines, &py_events)) { return nullptr; }
    dts::expect_event_sequence(object_to_string_array(py_lines),
                               object_to_string_array(py_events));
    Py_RETURN_NONE;
}
