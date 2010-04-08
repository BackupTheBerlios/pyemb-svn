#ifndef PYERROR_H
#define PYERROR_H

#include "pyembdef.h"
#include <string>

#pragma warning( disable: 4251 )

namespace PyEmb {
	class PYEMB_DECLSPEC PyError {

	public:
		PyError();
		~PyError();
		void setTraceback(const std::string &tb);
		void setDoingWhat(const std::string &action);
		void setException(const std::string &exc);
		void setExceptionValue(const std::string &excval);
		const std::string &traceback() const;
		const std::string &doingWhat() const;
		const std::string &exception() const;
		const std::string &exceptionValue() const;

	private:
		std::string m_exception;
		std::string m_excvalue;
		std::string m_traceback;
		std::string m_doingWhat;
	};
}

#endif
