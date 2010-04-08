  PySession session;
  // Build longtype 8394
  PyValue *val1 = session.BuildPyValue("i",8394);
  // Build stringtype "A string"
  PyValue *val2 = session.BuildPyValue("s","A string");
  // Build tupletype (1,"str1",3.45,"str2)
  PyValue *val3 = session.BuildPyValue("isfs",1,"str1",3.45,"str2");
  // Build multidimentional tuple ((1,"str1"),(3.45,"str2))
  PyValue *val4 = session.BuildPyValue("((is)(fs))",1,"str1",3.45,"str2");
