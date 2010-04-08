#include "pysession.h"
#include "pyclass.h"
#include "pyerror.h"

#include <Python.h>
#include <string>
#include <iostream>
#include <sstream>


namespace PyEmb {
	/** \brief Constructor

  @param autoAlert Automatically display python exceptions

PySession constructor. 
*/
	PySession::PySession(bool autoAlert) {
		m_sysModsLoaded = false;
		m_autoAlert = autoAlert;
		Py_Initialize();
	}

	/** \brief Destructor */
	PySession::~PySession() {
		PyClassArray::iterator it_inst = m_instances.begin();
		for (; it_inst != m_instances.end(); ++it_inst) {
			delete *it_inst;
		}

		emptyResultBuffer();

		PyObjectArray::iterator it_mod = m_modules.begin();
		for (; it_mod != m_modules.end(); ++it_mod) {
			Py_DECREF(*it_mod);
		}

		// Finalize python session
		Py_Finalize();
	}

	/** \brief Manually delete CallFunction() resultbuffer.
EmptyResultBuffer deletes all PyValue's administrated by PySession. Calling this actively
requieres caution, since it deletes the PyValues you may be using currently.
*/
	void PySession::emptyResultBuffer() {
		PyValueArray::iterator it_val = m_values.begin();
		for (; it_val!=m_values.end(); ++it_val) {
			delete *it_val;
		}
		m_values.erase(m_values.begin(),m_values.end());
	}

	/** \brief Import a python module. The python interpreter searches for the module
in the python sys.path.

  @param ModuleName Module to import - MyModule imports MyModule.py

The default sys.path is as follows:
<br>
  PY_HOME = path to TRPyEmbxx.dll <br>
  PY_HOME <br>
  PY_HOME/PyModules<br>
  PY_HOME/PyModules/site-packages <br>
  PY_HOME/PyScripts<br>
  PY_HOME/PyScripts/<running processname> (fx. PY_HOME/PyScripts/TRExport)<br>
<br>
Important modules:<br>
  sys  <br>
  os  <br>
  os.path  <br>
  odbc  <br>
  cx_Oracle  <br>
  <br>
  <a href="http://www.python.org/doc/current/modindex.html">Python module index</a>  <br>
  <a href="http://www.python.org/topics/database/">Python database api</a>  <br>
  <br>
*/
	bool PySession::importModule(const std::string &moduleName) {
		PyObject *pModule = loadModule(moduleName);
		if (pModule) {
			m_modules.push_back(pModule);
			return true;
		}
		return false;
	}

	/** \brief Enable/disable autoalert for the session being. This meens that python exceptions which
occure while importing modules, calling functions, creating class instances etc. will be
displayed immediately.

  @param autoalert True/false enable/disable repectively

*/
	void PySession::setAutoAlertEnabled(bool autoalert) {
		m_autoAlert = autoalert;
	}

	/** \brief Test autoalert enabled/disabled */
	bool PySession::autoAlertEnabled() {
		return m_autoAlert;
	}

	/** \brief Create a new instance of a class. The class must be declared
in one of the imported modules.
See PyClass for information on calling class methods.

  @param ModuleName Python module declaring the class
  @param ClassName Class to instantiate 
  @param Args Constructor arguments (null meens no arguments is passed)

*/
	PyClass* PySession::newInstance(const std::string &moduleName, const std::string &className,PyValue *args) {
		PyObject *pArgs = pyValueToPyObject(args,true);
		PyObject *pInstance = createInstance(moduleName,className,pArgs);
		Py_XDECREF(pArgs);
		if (pInstance) {
			PyClass *pyClassInstance = new PyClass(pInstance,this);
			m_instances.push_back(pyClassInstance);
			return pyClassInstance;
		}
		return NULL;
	}


	/** \brief Add an absolute path to python sys.path.
Only one path at the time

  @param path The absolute path to be added

You can display the current path by calling ShowPath()

*/
	void PySession::addToPyPath(const std::string &path) {
		PyObject *syspath = PySys_GetObject((char *) "path");
		PyObject *pathstr = PyString_FromString(path.c_str());
		PyList_Append(syspath,pathstr);
		Py_XDECREF(pathstr);
	}

	/** \brief Create and display a messagebox containing
the current python sys.path.

  Python sys.path determins in which order the python
  interpreter will search for modules.
  You can add elements to python sys.path by calling
  AddToPyPath().
*/
	void PySession::showPath() {
		PyObject *syspath = PySys_GetObject((char *) "path");
		PyObject *seperator = PyString_FromString(";");
		PyObject *pathstr = _PyString_Join(seperator,syspath);
		Py_XDECREF(seperator);

		std::ostringstream newpath;
		newpath << (const char*) PyString_AsString(pathstr);

		Py_XDECREF(pathstr);
		std::cout << newpath.str();
	}


#define TRACEBACK_FETCH_ERROR(what) {errMsg = what; goto done;}

	char *pyTraceback_AsString(PyObject *exc_tb) {
		char *errMsg = NULL; /* holds a local error message */
		char *result = NULL; /* a valid, allocated result. */
		PyObject *modStringIO = NULL;
		PyObject *modTB = NULL;
		PyObject *obFuncStringIO = NULL;
		PyObject *obStringIO = NULL;
		PyObject *obFuncTB = NULL;
		PyObject *argsTB = NULL;
		PyObject *obResult = NULL;

		/* Import the modules we need - cStringIO and traceback */
		modStringIO = PyImport_ImportModule("cStringIO");
		if (modStringIO==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cant import cStringIO\n");

		modTB = PyImport_ImportModule("traceback");
		if (modTB==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cant import traceback\n");
		/* Construct a cStringIO object */
		obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
		if (obFuncStringIO==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cant find cStringIO.StringIO\n");
		obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
		if (obStringIO==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cStringIO.StringIO() failed\n");
		/* Get the traceback.print_exception function, and call it. */
		obFuncTB = PyObject_GetAttrString(modTB, "print_tb");
		if (obFuncTB==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cant find traceback.print_tb\n");

		argsTB = Py_BuildValue("OOO",
							   exc_tb  ? exc_tb  : Py_None,
							   Py_None,
							   obStringIO);
		if (argsTB==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cant make print_tb arguments\n");

		obResult = PyObject_CallObject(obFuncTB, argsTB);
		if (obResult==NULL)
			TRACEBACK_FETCH_ERROR((char *) "traceback.print_tb() failed\n");
		/* Now call the getvalue() method in the StringIO instance */
		Py_DECREF(obFuncStringIO);
		obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
		if (obFuncStringIO==NULL)
			TRACEBACK_FETCH_ERROR((char *) "cant find getvalue function\n");
		Py_DECREF(obResult);
		obResult = PyObject_CallObject(obFuncStringIO, NULL);
		if (obResult==NULL)
			TRACEBACK_FETCH_ERROR((char *) "getvalue() failed.\n");

		/* And it should be a string all ready to go - duplicate it. */
		if (!PyString_Check(obResult))
			TRACEBACK_FETCH_ERROR((char *) "getvalue() did not return a string\n");

		{ // a temp scope so I can use temp locals.
			char *tempResult = PyString_AsString(obResult);
			result = (char *)PyMem_Malloc(strlen(tempResult)+1);
			if (result==NULL)
				TRACEBACK_FETCH_ERROR((char *) "memory error duplicating the traceback string\n");

			strcpy(result, tempResult);
		} // end of temp scope.
		done:
		/* All finished - first see if we encountered an error */
		if (result==NULL && errMsg != NULL) {
			result = (char *)PyMem_Malloc(strlen(errMsg)+1);
			if (result != NULL)
				/* if it does, not much we can do! */
				strcpy(result, errMsg);
		}
		Py_XDECREF(modStringIO);
		Py_XDECREF(modTB);
		Py_XDECREF(obFuncStringIO);
		Py_XDECREF(obStringIO);
		Py_XDECREF(obFuncTB);
		Py_XDECREF(argsTB);
		Py_XDECREF(obResult);
		return result;
	}

	void PySession::storeError(PyError *error) {
		PyObject *err_type, *err_value, *err_traceback;
		int have_error = PyErr_Occurred() ? 1 : 0;
		if (have_error) {
			PyErr_Fetch(&err_type, &err_value, &err_traceback);
		}
		else {
			return;
		}
		error->setTraceback(pyTraceback_AsString(err_traceback));

		PyObject *temp = PyObject_Str(err_type);
		if (temp) {
			error->setException(PyString_AsString(temp));
			Py_DECREF(temp);
		}
		else {
			error->setException("Can't convert exception to a string!");
		}
		if (err_value != NULL) {
			temp = PyObject_Str(err_value);
			if (temp) {
				error->setExceptionValue(PyString_AsString(temp));
				Py_DECREF(temp);
			}
			else {
				error->setExceptionValue("Can't convert exception value to a string!");
			}
		}
	}

	/** \brief Retrieve a pointer
to the last python exception (error) which has occured.
*/
	PyError *PySession::lastError() {
		return &m_lastError;
	}


	PyObject* PySession::loadModule(const std::string &moduleName) {
		PyObject *pName, *pModule;

		pName = PyString_FromString(moduleName.c_str());
		/* Error checking of pName left out */

		pModule = PyImport_Import(pName);
		if (pModule == NULL) {
			std::ostringstream doingwhat;
			doingwhat << "Importing " << moduleName << std::endl;

			m_lastError.setDoingWhat(doingwhat.str());
			storeError(&m_lastError);
			if (autoAlertEnabled()) {
				raiseErrorMessage();
				showPath();
			}
			return NULL;
		}
		Py_XDECREF(pName);
		return pModule;
	}


	/** \brief This method creates and displays a messagebox
containing information about the latest python exception (error)

This method is called automatically when autoAlert is enabled - 
SetAutoAlertEnabled().
*/
	void PySession::raiseErrorMessage()
	{
		std::ostringstream error;

		error << "Python exception occured while:" << std::endl;
		error << lastError()->doingWhat() << std::endl;
		error << lastError()->traceback() << lastError()->exception() << std::endl;
		error << lastError()->exceptionValue() << std::endl;

		std::cout << error.str();
	}

	PyObject* PySession::loadedModule(const std::string &moduleName) {
		PyObject *pModule=NULL;
		PyObjectArray::iterator it_mod = m_modules.begin();
		for (; it_mod != m_modules.end(); ++it_mod) {
			char *modName = PyModule_GetName(*it_mod);
			if (!strcmp(modName,moduleName.c_str())) {
				pModule = *it_mod;
				break;
			}
		}
		return pModule;
	}

	PyObject* PySession::createInstance(const std::string &moduleName,const std::string &className,PyObject *args) {
		PyObject *pModule, *pDict, *pClass, *pInstance;
		pModule = pDict = pClass = pInstance = NULL;

		pModule = loadedModule(moduleName);
		if (!pModule)
			importModule(moduleName);
		pModule = loadedModule(moduleName);

		if (pModule) {
			// Module found now attempt to make an instance
			pDict = PyModule_GetDict(pModule);                        // Try to load module dictionary
			if (!pDict)
				return NULL;
			else {
				pClass = PyDict_GetItemString(pDict, (char *) className.c_str());     // Get class reference
				if (pClass) {
					pInstance = PyInstance_New(pClass,args,NULL);           // Create it
				}
			}
			if (!pInstance)
			{
				std::ostringstream doingwhat;
				doingwhat << "Creating instance of class " << className << " from module " << moduleName << std::endl;
				m_lastError.setDoingWhat(doingwhat.str());
				storeError(&m_lastError);
				if (autoAlertEnabled())
					raiseErrorMessage();
			}
		}
		return pInstance;
	}

	void PySession::loadSysMods() {
		if (m_sysModsLoaded)
			return;
		if (importModule("sys") &&
			importModule("os"))
			m_sysModsLoaded = true;
	}


	/** \brief Call a python function
from an imported module using PyValue as return type and 
argument passing. Use BuildPyValue() to create arguments.

  @param Module Module containing function
  @param Function Function to be called
  @param Arguments Arguments being passed (NULL meens no arguments)

*/
	PyValue *PySession::callFunction(const std::string &moduleName, const std::string &functionName, PyValue *args) {
		PyValue *result = NULL;
		PyObject *pModule, *pDict, *pFunc;
		PyObject *pArgs, *pValue;
		/* Open a file handle. */

		pModule = loadedModule(moduleName);
		if (!pModule)
			importModule(moduleName);
		pModule = loadedModule(moduleName);

		if (pModule) {
			pDict = PyModule_GetDict(pModule);
			pFunc = PyDict_GetItemString(pDict,(char *) functionName.c_str());

			/* pFunc: Borrowed reference */
			if (pFunc && PyCallable_Check(pFunc)) {
				pArgs = pyValueToPyObject(args,true);
				pValue = PyObject_CallObject(pFunc, pArgs);
				if (pValue != NULL) {
					result = new PyValue(pValue);
					Py_DECREF(pValue);
					m_values.push_back(result);
				}
				else {
					std::ostringstream doingwhat;
					doingwhat << "Calling function " << functionName << " in module " << moduleName << std::endl;
					m_lastError.setDoingWhat(doingwhat.str());
					storeError(&m_lastError);
					if (autoAlertEnabled())
						raiseErrorMessage();
				}
				Py_XDECREF(pArgs);
				/* pDict and pFunc are borrowed and must not be Py_DECREF-ed */
			}
			else {
				//      PyErr_Print();
				std::cerr << "Cannot find function \"" << functionName << "\"" << std::endl;
			}
		}
		return result;
	}
	/** \brief Call python function
Like CallFunction() only it takes a PyObject as argument
PyValueToPyObject() can be used to convert a PyValue to a PyObject.

  @param Module Module containing function
  @param Function Function to be called
  @param pArgs Arguments being passed (NULL meens no arguments)

*/
	PyValue *PySession::callFunctionObj(const std::string &moduleName, const std::string &functionName, PyObject *pArgs) {
		PyValue *result = NULL;
		PyObject *pModule, *pDict, *pFunc;
		PyObject *pValue;
		/* Open a file handle. */

		pModule = loadedModule(moduleName);
		if (!pModule)
			importModule(moduleName);
		pModule = loadedModule(moduleName);

		if (pModule) {
			pDict = PyModule_GetDict(pModule);
			/* pDict is a borrowed reference */
			//pFunc = PyDict_GetItemString(pDict, Function.GetBuffer(Function.GetLength()));
			pFunc = PyDict_GetItemString(pDict,(char*)functionName.c_str());
			/* pFun: Borrowed reference */
			if (pFunc && PyCallable_Check(pFunc)) {
				pValue = PyObject_CallObject(pFunc, pArgs);
				if (pValue != NULL) {
					result = new PyValue(pValue);
					Py_DECREF(pValue);
					m_values.push_back(result);
				}
				else {
					std::ostringstream doingwhat;
					doingwhat << "Calling function " << functionName << " in module " << moduleName << std::endl;
					m_lastError.setDoingWhat(doingwhat.str());
					storeError(&m_lastError);
					if (autoAlertEnabled()) {
						raiseErrorMessage();
					}
				}
				Py_XDECREF(pArgs);
				/* pDict and pFunc are borrowed and must not be Py_DECREF-ed */
			}
			else {
				std::cerr << "Cannot find function \"" << functionName << "\"" << std::endl;
			}
		}
		return result;
	}

	/** \brief PyValue to PyObject conversion
Converts a PyValue instance to a PyObject. Setting forceTuple to true
will force the any PyValue into a tuple PyObject. This is usefull for
calling pythonfunctions since they only accept tupleobjects

  @param inValue PyValue to be converted
  @param forceTuple Force singlevalues into a single element tuple

*/
	PyObject *PySession::pyValueToPyObject(PyValue *inValue,bool forceTuple) {
		PyObject *pTuple,*pValue;
		pTuple = pValue = NULL;
		PyTuple tuple;
		if (!inValue) {
			if (forceTuple) {
				pTuple = PyTuple_New(0);
				return pTuple;
			}
			else {
				return NULL;
			}
		}
		if (inValue->valueType()==PyValue::PyTupleType) {
			tuple = inValue->valueAsTuple();
			pTuple = PyTuple_New(tuple.size());

			for (int i=0;i<tuple.size();i++) {
				PyTuple_SetItem(pTuple,i,pyValueToPyObject(tuple.value(i)));
			}
			return pTuple;
		}
		if (inValue->valueType()==PyValue::PyLongType) {
			long longval;
			longval = inValue->valueAsLong();
			pValue = PyLong_FromLong(longval);
		}
		else if (inValue->valueType()==PyValue::PyDoubleType) {
			double doubval;
			doubval = inValue->valueAsDouble();
			pValue = PyFloat_FromDouble(doubval);
		}
		else if (inValue->valueType()==PyValue::PyStringType)	{
			std::string strval;
			strval = inValue->valueAsString();
			pValue = PyString_FromString(strval.c_str());
		}
		if (forceTuple)	{
			pTuple = PyTuple_New(1);
			PyTuple_SetItem(pTuple,0,pValue);
			return pTuple;
		}
		return pValue;
	}


	/** \brief Build a PyValue object

  @param Format  's' (string) [char *] 
  's#' (string) [char *, int] <br>
 *'z' (string or None) [char *] <br>
 *'z#' (string or None) [char *, int] <br>
 *'i' (integer) [int] 
 *'b' (integer) [char] 
 *'h' (integer) [short int] <br>
 *'l' (integer) [long int] <br>
 *'c' (string of length 1) [char] <br>
 *'d' (float) [double] <br>
 *'f' (float) [float] <br><br>
 *'(items)' (tuple) [matching-items] <br>
 *'[items]' (list) [matching-items] <br>

 *It recognizes a set of format units similar to the ones recognized by PyArg_ParseTuple(),
 *but the arguments (which are input to the function, not output) must not be pointers, just 
 *values. It returns a new Python object, suitable for returning from a C function called from Python. 
 *<br>
 *One difference with PyArg_ParseTuple(): while the latter requires its first argument to be a 
 *tuple (since Python argument lists are always represented as tuples internally), BuildValue() 
 *does not always build a tuple. It builds a tuple only if its format string contains two or more 
 *format units. If the format string is empty, it returns None; if it contains exactly one format unit, 
 *it returns whatever object is described by that format unit. To force it to return a tuple of size 0 
 *or one, parenthesize the format string. <br>
 *Examples (to the left the call, to the right the resulting Python value): 
 * <br>
 *examples:<br>
 *    buildPyValue("")                        None <br>
 *    buildPyValue("i", 123)                  123  <br>
 *    buildPyValue("iii", 123, 456, 789)      (123, 456, 789)<br>
 *    buildPyValue("s", "hello")              'hello'<br>
 *    buildPyValue("ss", "hello", "world")    ('hello', 'world')<br>
 *    buildPyValue("s#", "hello", 4)          'hell'<br>
 *    buildPyValue("()")                      ()<br>
 *    buildPyValue("(i)", 123)                (123,)<br>
 *    buildPyValue("(ii)", 123, 456)          (123, 456)<br>
 *    buildPyValue("(i,i)", 123, 456)         (123, 456)<br>
 *    buildPyValue("[i,i]", 123, 456)         [123, 456]<br>
 *    buildPyValue("{s:i,s:i}",<br>
 *                  "abc", 123, "def", 456)    {'abc': 123, 'def': 456}<br>
 *    buildPyValue("((ii)(ii)) (ii)",<br>
 *                  1, 2, 3, 4, 5, 6)          (((1, 2), (3, 4)), (5, 6))<br>
 */

	PyValue *PySession::buildPyValue(const std::string &format,...) {
		PyValue *retpyval;
		PyObject *val;
		va_list args;
		va_start(args,format);
		val = Py_VaBuildValue((char*)format.c_str(), args);
		if (val) {
			retpyval = new PyValue(val);
			Py_DECREF(val);
			m_values.push_back(retpyval);
		}
		va_end(args);
		return retpyval;
	}

	/** \example buildpyvalue_ex.cpp
 * This example shows how to use BuildPyValue() to create a new PyValue instance.
 * In the example 3 values are build a number, a string and a tuple.
 * Note that m_pySession is a classmember of PySession type.
 */
	/** \example pyclass_ex.cpp
 * This example creates an instance of a simple HTTP reader and retrieves the source
 * google's mainpage
 */
	/** \example callfunction_ex.cpp
 * The example shows how to call a free python function. In this case the function
 * "getcwd()" (get current working directory) from the module "os". "os" is one of the
 * native python modules - see module index on python.org for more information.
 */
	/** \example pyerror_ex.cpp
 * This example demonstrates how to work with python exceptions through the TRPyEmb
 * framework. The PySession is created with autoalert disabled and thereby leaving
 * error notification in in the API users hand.
 */
	/** \example pypath_ex.cpp
 * This example demonstrates how to alter python sys.path
 */
}
