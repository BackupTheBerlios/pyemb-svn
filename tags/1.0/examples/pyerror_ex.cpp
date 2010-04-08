// ATTENTION: This code is ment to fail!!
// Provoke an importerror by attempting to import a non-existent module
PySession session(false);
if (!session.ImportModule("IDoNotExist"))
{
	// Get the latest error
	PyError *err = session.GetLastError();
	// Exception is a string decription of the error type
  CString exception = err->GetException();
	// Exception is the value of the exception
  CString excvalue = err->GetExceptionValue();
	// What was happening when the error occured
  CString doingwhat = err->GetDoingWhat();
  // The tracebackvalue is a quick debugging value. It shows
	// the callstack and the error occurence location
  CString traceback = err->GetTraceback();
}

// Provoke an OSError by attempting to create a folder with an illegal name
PyValue *arg = session.BuildPyValue("s","?illegal_folder");
  session.ImportModule("os");
if (!session.CallFunction("os","mkdir",arg))
{
  // This function raises an error message
  session.RaiseErrorMessage();
}
