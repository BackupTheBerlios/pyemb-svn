#ifndef PYSESSION_H
#define PYSESSION_H

#include "pyembdef.h"
#include "pyerror.h"
#include "pyvalue.h"
#include <vector>
#include <string>

#pragma warning( disable: 4251 )

struct _object;
typedef _object PyObject;

namespace PyEmb {
	class PyClass;
	extern char *pyTraceback_AsString(PyObject *exc_tb);

	typedef std::vector<PyObject*> PyObjectArray;
	typedef std::vector<PyClass*> PyClassArray;

	/** \class PySession
 PySession is the top of the framework this is where you import module,
 call functions, instantiate class objects etc. Almost all PySession methods return pointers these
 may not be deleted by the calling instance since PySession implements a simple Garbagecollector.<br>
 <br>
 GARBAGE COLLECTION <br>
 Methods returning instance pointers should never be deleted from the calling entity. These instances
 are deleted by the PySession instance on destruction. However, if you are working with very large return
 values you can clear the value buffer manually by calling EmptyResultBuffer() and thereby freeing memory
 <br><br>
*/

	class PYEMB_DECLSPEC PySession
	{
		friend class PyClass;
	public:
		PySession(bool autoAlert=true);
		~PySession();
		void addToPyPath(const std::string &path);
		bool importModule(const std::string &moduleName);
		PyClass *newInstance(const std::string &moduleName, const std::string &className,PyValue *args=NULL); // Garbage collection
		PyValue *callFunction(const std::string &moduleName, const std::string & functionName, PyValue *args=NULL);  // Garbage collection
		PyValue *callFunctionObj(const std::string &moduleName, const std::string &functionName, PyObject *args=NULL);  // Garbage collection
		void emptyResultBuffer();
		PyError *lastError();
		PyValue *buildPyValue(const std::string &format,...);  // Garbage collection
		bool autoAlertEnabled();
		void setAutoAlertEnabled(bool autoalert);
		void raiseErrorMessage();
		void showPath();
		PyObject *pyValueToPyObject(PyValue *value, bool forceTuple=false); // Must be DECREF'ed to prevent Memoryleaking

	private:
		void storeError(PyError *error);
		PyObject *loadModule(const std::string &moduleName);
		PyObject *createInstance(const std::string &moduleName,const std::string &className,PyObject *args);
		PyObject *loadedModule(const std::string &moduleName);
		void loadSysMods();

		PyObjectArray m_modules;
		PyClassArray m_instances;
		PyValueArray m_values;
		bool m_sysModsLoaded;
		PyError m_lastError;
		bool m_autoAlert;
	};
}

#endif
