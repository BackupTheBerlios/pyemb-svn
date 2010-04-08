  PySession session;
	PyValue *val = session.CallFunction("os","getcwd");
	// Test if function call was successful
	if (!val)
		return;
	// Working with single return value
	CString currentWorkingDir;
	if (val->GetValueType()==PyStringType)
      val->GetValueAsString(currentWorkingDir);
	MessageBox(currentWorkingDir);
