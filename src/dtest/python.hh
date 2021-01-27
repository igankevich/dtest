#ifndef DTEST_PYTHON_HH
#define DTEST_PYTHON_HH

#include <Python.h>

#include <memory>

namespace python {

    class interpreter_guard {
    public:
        inline explicit interpreter_guard(bool init_signal_handlers=false) noexcept {
            ::Py_InitializeEx(init_signal_handlers ? 1 : 0);
        }
        inline ~interpreter_guard() noexcept { ::Py_FinalizeEx(); }
        interpreter_guard(const interpreter_guard&) = delete;
        interpreter_guard& operator=(const interpreter_guard&) = delete;
        interpreter_guard(interpreter_guard&&) = delete;
        interpreter_guard& operator=(interpreter_guard&&) = delete;
    };

    class gil_guard {
    private:
        ::PyGILState_STATE _state;
    public:
        inline gil_guard() noexcept: _state(::PyGILState_Ensure()) {}
        inline ~gil_guard() noexcept { ::PyGILState_Release(this->_state); }
        gil_guard(const gil_guard&) = delete;
        gil_guard& operator=(const gil_guard&) = delete;
        gil_guard(gil_guard&&) = delete;
        gil_guard& operator=(gil_guard&&) = delete;
    };

    struct python_pointer_deleter {
        inline void operator()(void* ptr) { ::PyMem_RawFree(ptr); }
    };

    template <class T>
    using pointer = std::unique_ptr<T,python_pointer_deleter>;

    void set_arguments(int argc, char** argv);
    void load(const char* filename);
    void add_builtin_module(const char* name, PyObject* (*init)());

    class object {

    private:
        PyObject* _ptr{};

    public:
        object() noexcept = default;
        inline object(PyObject* ptr) noexcept: _ptr(ptr) {}
        inline ~object() noexcept { release(); }
        inline object(const object& rhs) noexcept: _ptr(rhs._ptr) { retain(); }
        inline object& operator=(const object& rhs) noexcept { object tmp(rhs); swap(tmp); return *this; }
        inline object(object&& rhs) noexcept: _ptr(rhs._ptr) { rhs._ptr = nullptr; }
        inline object& operator=(object&& rhs) noexcept { swap(rhs); return *this; };
        inline void swap(object& rhs) noexcept { std::swap(this->_ptr, rhs._ptr); }
        inline explicit operator bool() const noexcept { return this->_ptr != nullptr; }
        inline bool operator !() const noexcept { return !this->operator bool(); }
        inline PyObject* get() noexcept { return this->_ptr; }
        inline const PyObject* get() const noexcept { return this->_ptr; }
        inline operator PyObject*() noexcept { return get(); }
        inline operator const PyObject*() const noexcept { return get(); }
        inline Py_ssize_t size() const noexcept { return Py_SIZE(this->_ptr); }
        inline const PyTypeObject* type() const noexcept { return Py_TYPE(this->_ptr); }
        inline PyTypeObject* type() noexcept { return Py_TYPE(this->_ptr); }
        inline Py_ssize_t reference_count() const noexcept { return Py_REFCNT(this->_ptr); }
        inline void clear() noexcept { Py_CLEAR(this->_ptr); }
        inline void retain() noexcept { Py_XINCREF(this->_ptr); }
        inline void release() noexcept { Py_XDECREF(this->_ptr); }
        //inline void reference_count(Py_ssize_t n) noexcept { Py_SET_REFCNT(this->_ptr, n); }
    };

    inline void swap(object& a, object& b) { a.swap(b); }

}

namespace dts {
    namespace python {
        void init();
        void terminate();
        void application(::dts::application* ptr);
        int application_exit_code();
        PyObject* cluster(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* exit_code(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* add_process(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* run_process(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* kill_node(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* add_test(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* will_restart(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* execution_delay(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* run(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* fail(PyObject* self, PyObject* args, PyObject* kwds);
        PyObject* expect_event_sequence(PyObject* self, PyObject* args, PyObject* kwds);
    }
}

#endif // vim:filetype=cpp
