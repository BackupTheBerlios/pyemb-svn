#include <Python.h>
#include "pyclass.h"
#include "pysession.h"

#include <sstream>

namespace PyEmb {
	PyClass::PyClass(PyObject *instance,PySession *session) {
		m_session = session;
		m_instance=instance;
	}

	PyClass::~PyClass() {
		emptyResultBuffer();
		Py_DECREF(m_instance);
	}


	/**
Call classmethod <i>Method</i> passing arguments through <i>Args</i>
Use PySession::BuildPyValue() to create the argument.
*/
	PyValue *PyClass::callMethod(const std::string &methodName, PyValue *args) {

		PyObject *pValue,*pArgs,*pFunc = 0;
		PyValue *result=NULL;
		pFunc = PyObject_GetAttrString(m_instance, methodName.c_str());
		if (pFunc == NULL) {
			PyErr_SetString(PyExc_AttributeError, methodName.c_str());
			return 0;
		}

		if (!PyCallable_Check(pFunc)) {
			return 0;
		}

		pArgs = m_session->pyValueToPyObject(args,true);

		pValue = PyObject_Call(pFunc,pArgs, NULL);
		if (pArgs) {
			Py_DECREF(pArgs);
		}
		if (pValue) {
			result = new PyValue(pValue);
			Py_DECREF(pValue);
			m_resultbuffer.push_back(result);
			return result;
		}
		else {
			std::ostringstream doingwhat;
			doingwhat << "Calling method" << methodName << std::endl;
			m_session->m_lastError.setDoingWhat(doingwhat.str());
			m_session->storeError(&m_session->m_lastError);
			if (m_session->autoAlertEnabled()) {
				m_session->raiseErrorMessage();
			}
			return result;
		}
	}

	void PyClass::emptyResultBuffer() {
		PyValueArray::iterator it_val = m_resultbuffer.begin();
		for (; it_val != m_resultbuffer.end(); ++it_val) {
			delete *it_val;
		}
		m_resultbuffer.erase(m_resultbuffer.begin(),m_resultbuffer.end());
	}
}
