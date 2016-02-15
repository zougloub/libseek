//
// Created by Dmitry Sorokin on 2/15/16.
//

#include <Python.h>
#include "seek.hpp"

static LibSeek::Imager *imgr;
static LibSeek::Frame *frm;
static bool initialized = false;
static unsigned int length = 0;
static float *absolute_data;
static float _K = 0.02; // Experimental values, looks realistic
static float _C = -618;

// TODO: get_frame absolute should accept convertation coeff?

void
parse_coef_args(PyObject *args) {
    PyObject *p1 = nullptr;
    PyObject *p2 = nullptr;
    // TODO: can be a memleak here?
    PyArg_UnpackTuple(args, "ref", 0, 2, &p1, &p2);
    if (p1 != nullptr && PyFloat_Check(p1)) {
        _K = PyFloat_AsDouble(p1);
    }
    if (p2 != nullptr && PyFloat_Check(p2)) {
        _C = PyFloat_AsDouble(p2);
    }
}

bool
is_absolute(PyObject *args) {
    PyObject *p1 = nullptr;
    // TODO: can be a memleak here?
    PyArg_UnpackTuple(args, "ref", 0, 1, &p1);
    if (p1 != nullptr && PyBool_Check(p1)) {
        return p1 == Py_True;
    }
    return false;
}

static PyObject *
init(PyObject *self, PyObject *args) {
    if (initialized) {
        return nullptr;
    }
    parse_coef_args(args);
    // TODO: throw exceptions
    imgr = new LibSeek::Imager();
    frm = new LibSeek::Frame();
    imgr->init();
    imgr->frame_init(*frm);
    length = frm->width()*frm->height();
    absolute_data = new float[length];
    initialized = true;
    return Py_None;
}

static bool check_init() {
    if (initialized)
        return true;
    init(nullptr, nullptr);
    return initialized;
}

static PyObject *
deinit(PyObject *self, PyObject *args) {
    check_init(); // TODO: throw exception if unitialized
    delete frm;
    delete imgr;
    delete absolute_data;
    return Py_None;
}

const uint16_t *
_get_frame_data() {
    imgr->frame_acquire(*frm);
    return frm->data();
}

const float *
_convert_to_absolute(const uint16_t *data) {
    for (unsigned int i = 0; i < length; i++) {
        absolute_data[i] = data[i]*_K + _C;
    }
    return absolute_data;
}

static PyObject *
_value_builder(const float value) {
    return PyFloat_FromDouble(value);
}

static PyObject *
_value_builder(const uint16_t value) {
    return PyLong_FromLong(value);
}

template <typename T>
static PyObject *
_construct_flat_frame (T *data) {
    PyObject *res = PyList_New(length);

    for (unsigned int i = 0; i < length; i++) {
        PyList_SetItem(res, i, _value_builder(data[i]));
    }
    return res;
}

template <typename T>
static PyObject *
_construct_frame (T *data) {
    PyObject *res = PyList_New(frm->height());
    unsigned int offset = 0;

    for (unsigned int i = 0; i < frm->height(); i++) {
        PyObject *row = PyList_New(frm->width());
        PyList_SetItem(res, i, row);
        for (unsigned int j = 0; j < frm->width(); j++) {
            PyList_SetItem(row, j, _value_builder(data[offset]));
            offset++;
        }
    }
    return res;
}

static PyObject *
get_frame_flat(PyObject *self, PyObject *args) {
    check_init();

    auto data = _get_frame_data();
    return is_absolute(args) ?
        _construct_flat_frame(_convert_to_absolute(data)) :
        _construct_flat_frame(data);
}

static PyObject *
get_frame(PyObject *self, PyObject *args) {
    check_init();

    auto data = _get_frame_data();
    return is_absolute(args) ?
           _construct_frame(_convert_to_absolute(data)) :
           _construct_frame(data);
}

static PyObject *
get_dimensions(PyObject *self, PyObject *args) {
    check_init();
    PyObject *res = PyTuple_New(2);
    PyTuple_SetItem(res, 0, PyLong_FromLong(frm->width()));
    PyTuple_SetItem(res, 1, PyLong_FromLong(frm->height()));
    return res;
}


static PyMethodDef libseekMethods[] = {
        {"init",  init, METH_VARARGS,
                "Initialize device."},
        {"deinit",  deinit, METH_NOARGS,
                "Deinitialize device."},
        {"get_frame_flat",  get_frame, METH_VARARGS,
                "Get flattened frame from device."},
        {"get_frame",  get_frame, METH_VARARGS,
                "Get frame from device."},
        {"get_dimensions",  get_dimensions, METH_NOARGS,
                "Get frame width and height."},
        {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef pylibseekmodule = {
        PyModuleDef_HEAD_INIT,
        "libseek_python",   /* name of module */
        NULL, /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
        libseekMethods
};

PyMODINIT_FUNC
PyInit_pylibseek(void) {
    return PyModule_Create(&pylibseekmodule);
}