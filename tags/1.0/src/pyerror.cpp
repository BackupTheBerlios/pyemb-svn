#include "pyerror.h"

namespace PyEmb {
	PyError::PyError() {
	}

	PyError::~PyError() {
	}


	void PyError::setTraceback(const std::string &tb) {
		m_traceback = tb;
	}

	void PyError::setDoingWhat(const std::string &action) {
		m_doingWhat = action;
	}

	void PyError::setException(const std::string &exc) {
		m_exception = exc;
	}

	void PyError::setExceptionValue(const std::string &excval) {
		m_excvalue = excval;
	}

	const std::string &PyError::traceback() const {
		return m_traceback;
	}

	const std::string &PyError::doingWhat() const {
		return m_doingWhat;
	}

	const std::string &PyError::exception() const {
		return m_exception;
	}

	const std::string &PyError::exceptionValue() const {
		return m_excvalue;
	}
}
