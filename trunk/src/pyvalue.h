#ifndef PYVALUE_H
#define PYVALUE_H

#include "pyembdef.h"

#include <vector>
#include <map>
#include <string>

#pragma warning( disable: 4251 )

struct _object;
typedef _object PyObject;

namespace PyEmb {
	class PyClass;
	class PyTuple;
	class PyDict;

	class PYEMB_DECLSPEC PyValue {

	public:
		enum ValueType {PyNullType,PyLongType,PyDoubleType,PyStringType,PyUnicodeType,PyTupleType,PyDictType};

		PyValue(PyObject *pValue);
		PyValue();
		PyValue(long value) {m_tuple = NULL;m_dict = NULL;setValueAsLong(value);}
		PyValue(double value) {m_tuple = NULL;m_dict = NULL;setValueAsDouble(value);}
		PyValue(const std::string &value) {m_tuple = NULL;m_dict = NULL;setValueAsString(value);}
		PyValue(const PyTuple &value) {m_tuple = NULL;m_dict = NULL;setValueAsTuple(value);}
		PyValue(const PyDict &value) {m_tuple = NULL;m_dict = NULL;setValueAsDict(value);}
		PyValue(const PyValue &value);
		~PyValue();
		bool operator<(const PyValue &other) const;
		PyValue &operator=(const PyValue &other);
		void deepCopy(const PyValue &value);
		long valueAsLong(bool *ok=NULL) const;
		double valueAsDouble(bool *ok=NULL) const;
		const std::string &valueAsString(bool *ok=NULL) const;
		const PyTuple &valueAsTuple(bool *ok=NULL) const;
		const PyDict &valueAsDict(bool *ok=NULL) const;
		ValueType valueType() const {return m_valueType;}
		void setValueAsLong(long value);
		void setValueAsDouble(double value);
		void setValueAsString(const std::string &value);
		void setValueAsTuple(const PyTuple &value);
		void setValueAsDict(const PyDict &value);
		std::string str() const;
		static PyValue *buildPyValue(const char * Format,...);

	private:
		void valueRelease();
		void setValue_FromPyObject(PyObject *pValue);
		long m_longVal;
		std::string m_stringVal;
		double m_doubleVal;
		PyTuple *m_tuple;
		PyDict *m_dict;
		ValueType m_valueType;
	};


	typedef std::vector<PyValue*> PyValueArray;

	class PYEMB_DECLSPEC PyTuple {

	public:
		PyTuple();
		PyTuple(PyObject *pTuple);
		PyTuple(const PyTuple &tuple);
		~PyTuple();
		bool operator<(const PyTuple &tuple) const;
		PyTuple &operator=(const PyTuple &tuple);
		void deepCopy(const PyTuple &tuple);
		PyValue *value(int index);
		const PyValue &value(int index) const;
		void addValue(const PyValue &val);
		void removeValue(int index);
		int size() const {return m_valueArray.size();}
		std::string str() const;

	private:
		PyValueArray m_valueArray;
	};


	typedef std::map<PyValue,PyValue *> PyValueMap;

	class PYEMB_DECLSPEC PyDict {

	public:
		PyDict();
		PyDict(PyObject *pDict);
		PyDict(const PyDict &dict);
		~PyDict();
		PyDict &operator=(const PyDict &tuple);
		void deepCopy(const PyDict &dict);
		PyValue *value(const PyValue &key);
		const PyValue &value(const PyValue &key) const;
		void setValue(const PyValue &key, const PyValue &val);
		void removeValue(const PyValue &key);
		int size() {return m_valueMap.size();}
		std::string str() const;

	private:
		PyValueMap m_valueMap;
	};

}

#endif
