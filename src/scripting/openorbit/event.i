%module(package="openorbit") event

%{
  /* Code for wrapper */
#include "sim/simevent.h"

  struct pyevent {
    bool periodic;
    double period;
    PyObject *func;
    PyObject *data;
  };

  void
  pyeventhandler(void *data)
  {
    struct pyevent *ev = data;

    PyObject *arglist;
    PyObject *result;

    arglist = Py_BuildValue("(o)", ev->data);
    if (arglist == NULL) {
      PyErr_Print();
    }
    result = PyObject_CallObject(ev->func, arglist);
    Py_DECREF(arglist);
    Py_DECREF(result);

    if (!ev->periodic) {
      Py_DECREF(ev->func);
      Py_DECREF(ev->data);
      free(ev);
    } else {
      // TODO: Enqueue based on absolute time
      sim_event_enqueue_relative_s(ev->period, pyeventhandler, ev);
    }
  }

  void
  stackPyEvent(PyObject *func, PyObject *arg)
  {
    struct pyevent *ev = malloc(sizeof(struct pyevent));
    ev->periodic = false;
    ev->func = func;
    ev->data = arg;

    Py_INCREF(func);
    Py_INCREF(arg);

    sim_event_stackpost(pyeventhandler, ev);
  }

  void
  enqAbsEvent(double jd, PyObject *func, PyObject *arg)
  {
    struct pyevent *ev = malloc(sizeof(struct pyevent));
    ev->periodic = false;
    ev->func = func;
    ev->data = arg;

    Py_INCREF(func);
    Py_INCREF(arg);

    sim_event_enqueue_absolute(jd, pyeventhandler, ev);
  }


  void
  enqDeltaEvent_ms(unsigned offset, PyObject *func, PyObject *arg)
  {
    struct pyevent *ev = malloc(sizeof(struct pyevent));
    ev->periodic = false;
    ev->func = func;
    ev->data = arg;

    Py_INCREF(func);
    Py_INCREF(arg);

    sim_event_enqueue_relative_ms(offset, pyeventhandler, ev);
  }

  void
  enqDeltaEvent_s(double offset, PyObject *func, PyObject *arg)
  {
    struct pyevent *ev = malloc(sizeof(struct pyevent));
    ev->periodic = false;
    ev->func = func;
    ev->data = arg;

    Py_INCREF(func);
    Py_INCREF(arg);

    sim_event_enqueue_relative_s(offset, pyeventhandler, ev);
  }

%}

%typemap(in) PyObject * {
        $1 = (PyObject *) $input;
}


%rename(stackPost) stackPyEvent;
void stackPyEvent(PyObject *func, PyObject *arg);

%rename(absPost) enqAbsEvent;
void enqAbsEvent(double jd, PyObject *func, PyObject *arg);

%rename(deltaSecPost) enqDeltaEvent_s;
void enqDeltaEvent_s(double offset, PyObject *func, PyObject *arg);

%rename(deltaMilliSecPost) enqDeltaEvent_ms;
void enqDeltaEvent_ms(unsigned offset, PyObject *func, PyObject *arg);
