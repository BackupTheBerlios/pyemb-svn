#include "pyvalue.h"
#include "cdebug.h"

#include <Python.h>
#include <sstream>
#include <iostream>

namespace PyEmb {

	PyValue pyNullValue = PyValue();

	PyValue::PyValue(PyObject *pValue) {

		CDEBUG << "PvValue create: " << this << std::endl;
		m_valueType = PyNullType;
		m_tuple = NULL;
		m_dict = NULL;
		setValue_FromPyObject(pValue);
	}

	PyValue::PyValue(const PyValue &value) {
		CDEBUG << "PvValue create: " << this << std::endl;
		m_tuple = NULL;
		m_dict= NULL;
		deepCopy(value);
	}

	PyValue::PyValue() {
		CDEBUG << "PvValue create: " << this << std::endl;
		m_valueType = PyNullType;
		m_tuple = NULL;
		m_dict = NULL;
	}


	PyValue::~PyValue() {
		CDEBUG << "PvValue delete: " << this << std::endl;
		valueRelease();
	}

	void PyValue::deepCopy(const PyValue &value) {
		valueRelease();
		m_valueType = value.valueType();
		if (value.valueType() == PyLongType) {
			setValueAsLong(value.valueAsLong());
		}
		if (value.valueType() == PyDoubleType) {
			setValueAsDouble(value.valueAsDouble());
		}
		if (value.valueType() == PyStringType) {
			setValueAsString(value.valueAsString());
		}
		if (value.valueType() == PyTupleType) {
			setValueAsTuple(value.valueAsTuple());
		}
		if (value.valueType() == PyDictType) {
			setValueAsDict(value.valueAsDict());
		}
	}

	long PyValue::valueAsLong(bool *ok) const {
		bool success = false;
		if (m_valueType == PyLongType) {
			success = true;
		}
		if (ok) {
			*ok = success;
		}
		return m_longVal;
	}

	double PyValue::valueAsDouble(bool *ok) const {
		bool success = false;
		if (m_valueType == PyDoubleType) {
			success = true;
		}
		if (ok) {
			*ok = success;
		}
		return m_doubleVal;
	}

	const std::string &PyValue::valueAsString(bool *ok) const {
		bool success = false;
		if (m_valueType == PyStringType) {
			success = true;
		}
		if (ok) {
			*ok = success;
		}
		return m_stringVal;
	}

	const PyTuple &PyValue::valueAsTuple(bool *ok) const {
		bool success = false;
		if (m_valueType == PyTupleType) {
			success = true;
		}
		if (ok) {
			*ok = success;
		}
		return *m_tuple;
	}

	const PyDict &PyValue::valueAsDict(bool *ok) const {
		bool success = false;
		if (m_valueType == PyDictType) {
			success = true;
		}
		if (ok) {
			*ok = success;
		}
		return *m_dict;
	}

	void PyValue::setValueAsLong(long value) {
		valueRelease();
		m_valueType = PyLongType;
		m_longVal = value;
	}

	void PyValue::setValueAsDouble(double value) {
		valueRelease();
		m_valueType = PyDoubleType;
		m_doubleVal = value;
	}

	void PyValue::setValueAsString(const std::string &value) {
		valueRelease();
		m_valueType = PyStringType;
		m_stringVal = value;
	}

	void PyValue::setValueAsTuple(const PyTuple &value) {
		valueRelease();
		m_valueType = PyTupleType;
		m_tuple = new PyTuple(value);
	}

	void PyValue::setValueAsDict(const PyDict &value) {
		valueRelease();
		m_valueType = PyDictType;
		m_dict = new PyDict(value);
	}

	void PyValue::setValue_FromPyObject(PyObject *pValue) {
		bool decRefList = false;
		valueRelease();
		if (pValue) {
			if (PyList_Check(pValue)) {
				decRefList = true;
				pValue = PyList_AsTuple(pValue);
			}
			else if (PyUnicode_Check(pValue)) {
				m_valueType = PyUnicodeType;
			}
			else if (PyFloat_Check(pValue)) {
				m_doubleVal = PyFloat_AsDouble(pValue);
				m_valueType = PyDoubleType;
			}
			else if (PyInt_Check(pValue)) {
				m_longVal = PyInt_AsLong(pValue);
				m_valueType = PyLongType;
			}
			else if (PyLong_Check(pValue)) {
				m_longVal = PyLong_AsLong(pValue);
				m_valueType = PyLongType;
			}
			else if (PyString_Check(pValue)) {
				setValueAsString(PyString_AsString(pValue));
				m_valueType = PyStringType;
			}
			else if (PyTuple_Check(pValue)) {
				m_tuple = new PyTuple(pValue);
				m_valueType = PyTupleType;
			}
			else if (PyDict_Check(pValue)) {
				m_dict = new PyDict(pValue);
				m_valueType = PyDictType;
			}

			if (decRefList) {
				Py_XDECREF(pValue);
			}
		}
	}

	void PyValue::valueRelease() {
		m_valueType = PyNullType;
		if (m_tuple) {
			delete m_tuple;
			m_tuple = NULL;
		}
		if (m_dict) {
			delete m_dict;
			m_dict = NULL;
		}
	}

	std::string &replaceInStdString(std::string &s, const std::string &sub,const std::string &other) {
		assert(!sub.empty());
		size_t b = 0;
		for (;;) {
			b = s.find(sub, b);
			if (b == s.npos) break;
			s.replace(b, sub.size(), other);
			b += other.size();
		}
		return s;
	}

	PyValue &PyValue::operator=(const PyValue &other) {
		CDEBUG << "PvValue operator=: " << this << std::endl;
		deepCopy(other);
		return *this;
	}

	bool PyValue::operator<(const PyValue &other) const {
		if (valueType()<other.valueType()) {
			return true;
		}
		else if (valueType()==PyLongType && m_longVal<other.valueAsLong()) {
			return true;
		}
		else if (valueType()==PyDoubleType && m_doubleVal<other.valueAsDouble()) {
			return true;
		}
		else if (valueType()==PyStringType && m_stringVal<other.valueAsString()) {
			return true;
		}
		return false;
	}

	std::string PyValue::str() const {
		CDEBUG << "printing: " << this << std::endl;
		std::ostringstream strstream;
		if (valueType() == PyLongType) {
			strstream << m_longVal;
		}
		if (valueType() == PyDoubleType) {
			strstream << m_doubleVal;
		}
		if (valueType() == PyStringType) {
			std::string tmp = m_stringVal;
			replaceInStdString(tmp,"\n","\\n");
			strstream << "'" << tmp << "'";
		}
		if (valueType() == PyTupleType) {
			strstream << m_tuple->str();
		}
		if (valueType() == PyDictType) {
			strstream << m_dict->str();
		}
		if (valueType() == PyNullType) {
			strstream << "None";
		}
		return strstream.str();

	}


	/////////////////////////////
	//  PyTuple
	/////////////////////////////


	PyTuple::PyTuple() {
		CDEBUG << "PvTuple create: " << this << std::endl;
	}

	PyTuple::PyTuple(PyObject *pTuple) {
		CDEBUG << "PvTuple create: " << this << std::endl;
		if (!PyTuple_Check(pTuple)) {
			return;
		}
		for (int i=0;i<PyTuple_Size(pTuple);i++) {
			PyValue *newVal = new PyValue(PyTuple_GetItem(pTuple,i));
			m_valueArray.push_back(newVal);
		}
	}


	PyTuple::PyTuple(const PyTuple &tuple) {
		CDEBUG << "PyTuple create: " << this << std::endl;
		deepCopy(tuple);
	}

	PyTuple &PyTuple::operator=(const PyTuple &other) {
		CDEBUG << "PyTuple operator=: " << this << std::endl;
		deepCopy(other);
		return *this;
	}

	bool PyTuple::operator<(const PyTuple &other) const {
		if (size() < other.size()) {
			return true;
		}
		else if (size() > other.size()) {
			return false;
		}
		PyValueArray::const_iterator it_val = m_valueArray.begin();
		PyValueArray::const_iterator it_val_other = other.m_valueArray.begin();
		for (; it_val!=m_valueArray.end(); ++it_val,++it_val_other) {
			if (*(*it_val) < *(*it_val_other)) {
				return true;
			}
		}
		return false;
	}

	void PyTuple::deepCopy(const PyTuple &tuple) {
		PyValueArray::const_iterator it_val = tuple.m_valueArray.begin();
		for (; it_val!=tuple.m_valueArray.end(); ++it_val) {
			PyValue *newVal = new PyValue(*(*it_val));
			m_valueArray.push_back(newVal);
		}
	}

	PyTuple::~PyTuple() {
		CDEBUG << "PvTuple delete: " << this << std::endl;
		PyValueArray::iterator it_val = m_valueArray.begin();
		for (; it_val!=m_valueArray.end(); ++it_val) {
			delete *it_val;
		}
	}

	PyValue* PyTuple::value(int index) {
		if (index >= (int) m_valueArray.size()) {
			return NULL;
		}
		int cnt = 0;
		PyValueArray::iterator it_val = m_valueArray.begin();
		for (; it_val!=m_valueArray.end(); ++it_val) {
			if (cnt==index) {
				return *it_val;
			}
			cnt++;
		}
		return NULL;
	}

	const PyValue &PyTuple::value(int index) const {
		if (index >= (int) m_valueArray.size()) {
			return pyNullValue;
		}
		int cnt = 0;
		PyValueArray::const_iterator it_val = m_valueArray.begin();
		for (; it_val!=m_valueArray.end(); ++it_val) {
			if (cnt==index) {
				return *(*it_val);
			}
			cnt++;
		}
		return pyNullValue;
	}

	void PyTuple::addValue(const PyValue &val) {
		m_valueArray.push_back(new PyValue(val));
	}

	void PyTuple::removeValue(int index) {
		if (index >= (int) m_valueArray.size()) {
			return;
		}
		int cnt = 0;
		PyValueArray::iterator it_val = m_valueArray.begin();
		for (; it_val!=m_valueArray.end(); ++it_val) {
			if (cnt==index) {
				delete *it_val;
				m_valueArray.erase(it_val);
			}
			cnt++;
		}
	}

	std::string PyTuple::str() const {
		std::ostringstream strstream;
		strstream << '(';
		bool first = true;
		PyValueArray::const_iterator it_val = m_valueArray.begin();
		for (; it_val!=m_valueArray.end(); ++it_val) {
			if (!first) {
				strstream << ",";
			}
			strstream << (*it_val)->str();
			first = false;
		}
		if (m_valueArray.size()==1) {
			strstream << ",";
		}
		strstream << ')';
		return strstream.str();
	}



	PyDict::PyDict() {

	}

	PyDict::PyDict(PyObject *pDict){
		CDEBUG << "PyDict create: " << this << std::endl;
		if (!PyDict_Check(pDict)) {
			return;
		}
		PyObject *items = PyDict_Items(pDict);
		if (!PyList_Check(items)) {
			return;
		}

		for (int i=0;i<PyList_Size(items);i++) {
			PyObject *item = PyList_GetItem(items,i);
			if (!PyTuple_Check(item)) {
				return;
			}
			PyValue key(PyTuple_GetItem(item, 0));
			PyValue *val = new PyValue(PyTuple_GetItem(item, 1));
			m_valueMap[key] = val;
		}
		Py_XDECREF(items);
	}

	PyDict::PyDict(const PyDict &dict) {
		deepCopy(dict);
	}

	PyDict::~PyDict() {
		PyValueMap::iterator it = m_valueMap.begin();
		for (; it!=m_valueMap.end(); ++it) {
			delete it->second;
		}
	}

	PyDict &PyDict::operator=(const PyDict &other) {
		CDEBUG << "PyDict operator=: " << this << std::endl;
		deepCopy(other);
		return *this;
	}

	void PyDict::deepCopy(const PyDict &dict) {
		PyValueMap::const_iterator it = dict.m_valueMap.begin();
		for (; it!=dict.m_valueMap.end(); ++it) {
			m_valueMap[it->first] = new PyValue(*(it->second));
		}
	}

	PyValue *PyDict::value(const PyValue &key) {
		PyValueMap::iterator it = m_valueMap.find(key);
		if (it!=m_valueMap.end()) {
			return it->second;
		}
		return NULL;
	}

	const PyValue &PyDict::value(const PyValue &key) const {
		PyValueMap::const_iterator it = m_valueMap.find(key);
		if (it!=m_valueMap.end()) {
			return *(it->second);
		}
		return pyNullValue;
	}

	void PyDict::setValue(const PyValue &key, const PyValue &val) {
		m_valueMap[key] = new PyValue(val);
	}

	void PyDict::removeValue(const PyValue &key) {
		PyValueMap::iterator it = m_valueMap.find(key);
		if (it!=m_valueMap.end()) {
			delete it->second;
			m_valueMap.erase(it);
		}
	}

	std::string PyDict::str() const {
		std::ostringstream strstream;
		strstream << '{';
		bool first = true;
		PyValueMap::const_iterator it_val = m_valueMap.begin();
		for (; it_val!=m_valueMap.end(); ++it_val) {
			if (!first) {
				strstream << ",";
			}
			strstream << it_val->first.str() << ":" << it_val->second->str();
			first = false;
		}
		strstream << '}';
		return strstream.str();
	}
}
