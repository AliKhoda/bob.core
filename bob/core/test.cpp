/**
 * @author Manuel Gunther <siebenkopf@googlemail.com>
 * @date Tue Oct 18 11:45:30 MDT 2016
 *
 * @brief Binds configuration information available from bob
 */

#include <bob.blitz/config.h>
#include <bob.blitz/cleanup.h>
#include <bob.extension/documentation.h>

#include <bob.core/logging.h>

#include <boost/shared_array.hpp>

PyDoc_STRVAR(module_docstr,
  "Tests the C++ API of bob.core"
);

struct message_info_t {
  boost::iostreams::stream<bob::core::AutoOutputDevice>* s;
  std::string message;
  bool exit;
  unsigned int ntimes;
  unsigned int thread_id;
};

static void* log_message_inner(void* cookie) {

  message_info_t* mi = (message_info_t*)cookie;

# if PYTHON_LOGGING_DEBUG != 0
  if (PyEval_ThreadsInitialized()) {
    static_log << "(thread " << mi->thread_id << ") Python threads initialized correctly for this thread" << std::endl;
  }
  else {
    static_log << "(thread " << mi->thread_id << ") Python threads NOT INITIALIZED correctly for this thread" << std::endl;
  }
# endif

  for (unsigned int i=0; i<(mi->ntimes); ++i) {

#   if PYTHON_LOGGING_DEBUG != 0
    static_log << "(thread " << mi->thread_id << ") Injecting message `" << mi->message << " (thread " << mi->thread_id << "; iteration " << i << ")'" << std::endl;
#   endif

    *(mi->s) << mi->message
#     if PYTHON_LOGGING_DEBUG != 0
            << " (thread " << mi->thread_id << "; iteration " << i << ")"
#     endif
            << std::endl;

    mi->s->flush();
  }
  if (mi->exit) {
#   if PYTHON_LOGGING_DEBUG != 0
    static_log << "(thread " << mi->thread_id << ") Exiting this thread" << std::endl;
#   endif
    pthread_exit(0);
  }

# if PYTHON_LOGGING_DEBUG != 0
  if (mi->exit) {
    static_log << "(thread " << mi->thread_id << ") Returning 0" << std::endl;
  }
# endif
  return 0;
}

static auto _logmsg_doc = bob::extension::FunctionDoc(
  "_test_log_message",
  "Logs a message into Bob's logging system from C++",
  "This function is bound for testing purposes only and is not part of the Python API for bob.core"
)
.add_prototype("ntimes, stream, message")
.add_parameter("ntimes", "int", "The number of times to print the given message")
.add_parameter("stream", "str", "The stream to use for logging the message. Choose from ``('debug', 'info', 'warn', 'error')")
.add_parameter("message", "str", "The message to be logged")
;
static PyObject* log_message(PyObject*, PyObject* args, PyObject* kwds) {
BOB_TRY
  char** kwlist = _logmsg_doc.kwlist();

  unsigned int ntimes;
  const char* stream;
  const char* message;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Iss",
        kwlist, &ntimes, &stream, &message)) return 0;

  // implements: if stream not in ('debug', 'info', 'warn', 'error')
  boost::iostreams::stream<bob::core::AutoOutputDevice>* s = 0;
  if (strncmp(stream, "debug", 5) == 0) s = &bob::core::debug;
  else if (strncmp(stream, "info", 4) == 0) s = &bob::core::info;
  else if (strncmp(stream, "warn", 4) == 0) s = &bob::core::warn;
  else if (strncmp(stream, "error", 5) == 0) s = &bob::core::error;
  else if (strncmp(stream, "fatal", 5) == 0) s = &bob::core::error;
  else {
    PyErr_Format(PyExc_ValueError, "parameter `stream' must be one of 'debug', 'info', 'warn', 'error' or 'fatal' (synomym for 'error'), not '%s'", stream);
    return 0;
  }

  // do the work for this function
  auto no_gil = PyEval_SaveThread();

  message_info_t mi = {s, message, false, ntimes, 0};
  log_message_inner((void*)&mi);
# if PYTHON_LOGGING_DEBUG != 0
  static_log << "(thread 0) Returning to caller" << std::endl;
# endif

  PyEval_RestoreThread(no_gil);

  Py_RETURN_NONE;
BOB_CATCH_FUNCTION("_log_message", 0)
}


static auto _logmsg_mt_doc = bob::extension::FunctionDoc(
  "_test_log_message_mt",
  "Logs a message into Bob's logging system from C++, in a separate thread",
  "This function is bound for testing purposes only and is not part of the Python API for bob.core"
)
.add_prototype("nthreads, ntimes, stream, message")
.add_parameter("nthreads", "int", "The total number of threads from which to write messages to the logging system using the C++->Python API")
.add_parameter("ntimes", "int", "The number of times to print the given message")
.add_parameter("stream", "str", "The stream to use for logging the message. Choose from ``('debug', 'info', 'warn', 'error')")
.add_parameter("message", "str", "The message to be logged")
;
static PyObject* log_message_mt(PyObject*, PyObject* args, PyObject* kwds) {
BOB_TRY
  char** kwlist = _logmsg_mt_doc.kwlist();

  unsigned int nthreads;
  unsigned int ntimes;
  const char* stream;
  const char* message;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "IIss",
        kwlist, &nthreads, &ntimes, &stream, &message)) return 0;

  // implements: if stream not in ('debug', 'info', 'warn', 'error')
  boost::iostreams::stream<bob::core::AutoOutputDevice>* s = 0;
  if (strncmp(stream, "debug", 5) == 0) s = &bob::core::debug;
  else if (strncmp(stream, "info", 4) == 0) s = &bob::core::info;
  else if (strncmp(stream, "warn", 4) == 0) s = &bob::core::warn;
  else if (strncmp(stream, "error", 5) == 0) s = &bob::core::error;
  else {
    PyErr_Format(PyExc_ValueError, "parameter `stream' must be one of 'debug', 'info', 'warn' or 'error', not '%s'", stream);
    return 0;
  }

  // do the work for this function
  auto no_gil = PyEval_SaveThread();

  boost::shared_array<pthread_t> threads(new pthread_t[nthreads]);
  boost::shared_array<message_info_t> infos(new message_info_t[nthreads]);
  for (unsigned int i=0; i<nthreads; ++i) {
    message_info_t mi = {s, message, true, ntimes, i+1};
    infos[i] = mi;
  }

# if PYTHON_LOGGING_DEBUG != 0
  static_log << "(thread 0) Launching " << nthreads << " thread(s)" << std::endl;
# endif

  for (unsigned int i=0; i<nthreads; ++i) {

#   if PYTHON_LOGGING_DEBUG != 0
    static_log << "(thread 0) Launch thread " << (i+1) << ": `" << message << "'" << std::endl;
#   endif

    pthread_create(&threads[i], NULL, &log_message_inner, (void*)&infos[i]);

#   if PYTHON_LOGGING_DEBUG != 0
    static_log << "(thread 0) thread " << (i+1)
               << " == " << std::hex << threads[i] << std::dec
               << " launched" << std::endl;
#   endif

  }

  void* status;
# if PYTHON_LOGGING_DEBUG != 0
  static_log << "(thread 0) Waiting " << nthreads << " thread(s)" << std::endl;
# endif
  for (unsigned int i=0; i<nthreads; ++i) {
    pthread_join(threads[i], &status);
#   if PYTHON_LOGGING_DEBUG != 0
    static_log << "(thread 0) Waiting on thread " << (i+1) << std::endl;
#   endif
  }
# if PYTHON_LOGGING_DEBUG != 0
  static_log << "(thread 0) Returning to caller" << std::endl;
# endif

  PyEval_RestoreThread(no_gil);

  Py_RETURN_NONE;
BOB_CATCH_FUNCTION("_log_message_mt", 0)
}



static auto _output_disable_doc = bob::extension::FunctionDoc(
  "_test_output_disable",
  "Writes C++ messages with and without being visible"
)
.add_prototype("", "success")
.add_return("success", "bool", "The success of the test")
;
PyObject* output_disable(PyObject*, PyObject* args, PyObject* kwds) {
BOB_TRY
  char** kwlist = _output_disable_doc.kwlist();

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) return 0;

  // redirect std::cout and std::cerr, see http://stackoverflow.com/a/5419388/3301902
  std::stringstream out, err;
  std::streambuf * oldout = std::cout.rdbuf(out.rdbuf());
  std::streambuf * olderr = std::cerr.rdbuf(err.rdbuf());

  bool success = true;

  try {
    bob::core::log_level(bob::core::DEBUG);
    bob::core::debug << "This is a debug message" << std::endl;
    bob::core::info << "This is an info message" << std::endl;
    bob::core::warn << "This is a warning message" << std::endl;
    bob::core::error << "This is an error message" << std::endl;
    success = success && out.str() == "This is a debug message\nThis is an info message\n";
    success = success && err.str() == "This is a warning message\nThis is an error message\n";

    out.str("");
    err.str("");
    bob::core::log_level(bob::core::ERROR);
    bob::core::debug << "This is a debug message" << std::endl;
    bob::core::info << "This is an info message" << std::endl;
    bob::core::warn << "This is a warning message" << std::endl;
    bob::core::error << "This is an error message" << std::endl;
    success = success && out.str() == "";
    success = success && err.str() == "This is an error message\n";

    out.str("");
    err.str("");
    bob::core::log_level(bob::core::DISABLE);
    bob::core::debug << "This is a debug message" << std::endl;
    bob::core::info << "This is an info message" << std::endl;
    bob::core::warn << "This is a warning message" << std::endl;
    bob::core::error << "This is an error message" << std::endl;
    success = success && out.str() == "";
    success = success && err.str() == "";

  } catch(...){
    success = false;
  }

  // make sure that cout and cerr are redirected to their original streams
  std::cout.rdbuf( oldout );
  std::cerr.rdbuf( olderr );

  bob::core::log_level(bob::core::DEBUG);

  if (success)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;

BOB_CATCH_FUNCTION("_test_output_disable", 0)
}



static PyMethodDef module_methods[] = {
    {
      _logmsg_doc.name(),
      (PyCFunction)log_message,
      METH_VARARGS|METH_KEYWORDS,
      _logmsg_doc.doc()
    },
    {
      _logmsg_mt_doc.name(),
      (PyCFunction)log_message_mt,
      METH_VARARGS|METH_KEYWORDS,
      _logmsg_mt_doc.doc()
    },
    {
      _output_disable_doc.name(),
      (PyCFunction)output_disable,
      METH_VARARGS|METH_KEYWORDS,
      _output_disable_doc.doc()
    },
    {0}  /* Sentinel */
};


#if PY_VERSION_HEX >= 0x03000000
static PyModuleDef module_definition = {
  PyModuleDef_HEAD_INIT,
  BOB_EXT_MODULE_NAME,
  module_docstr,
  -1,
  module_methods,
  0, 0, 0, 0
};
#endif

static PyObject* create_module (void) {

# if PY_VERSION_HEX >= 0x03000000
  PyObject* m = PyModule_Create(&module_definition);
  auto m_ = make_xsafe(m);
  const char* ret = "O";
# else
  PyObject* m = Py_InitModule3(BOB_EXT_MODULE_NAME, module_methods, module_docstr);
  const char* ret = "N";
# endif
  if (!m) return 0;

  /* register version numbers and constants */
  if (PyModule_AddStringConstant(m, "module", BOB_EXT_MODULE_VERSION) < 0) return 0;

  return Py_BuildValue(ret, m);
}

PyMODINIT_FUNC BOB_EXT_ENTRY_NAME (void) {
# if PY_VERSION_HEX >= 0x03000000
  return
# endif
    create_module();
}
