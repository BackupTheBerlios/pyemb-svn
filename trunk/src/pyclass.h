#ifndef PYCLASS_H
#define PYCLASS_H

#include "pyembdef.h"
#include "pyvalue.h"

#pragma warning( disable: 4251 )

struct _object;
typedef _object PyObject;

namespace PyEmb {
	class PySession;

	class PYEMB_DECLSPEC PyClass {

	public:
		PyClass(PyObject *instance,PySession *session);
		~PyClass();
		PyValue *callMethod(const std::string &methodName, PyValue *args=NULL);
		void emptyResultBuffer();

	private:
		PyObject *m_instance;
		PySession *m_session;
		PyValueArray m_resultbuffer;
	};
}

#endif
