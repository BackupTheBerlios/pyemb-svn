/*
MyClass.py:

import urllib

class HTTPReadRaw:

  def __init__(self,url):
    self.site = urllib.urlopen(url)

  def getSource(self):
    return self.site.read()
*/

PySession session;

session.ImportModule("MyClass");

PyValue *args = session.BuildPyValue("s","http://www.google.com");

// Create instance of HTTPReadRaw declared in module MyClass
PyClass *cls = session.NewInstance("MyClass","HTTPReadRaw",args);
if (!cls)
  return;
// Call method getSource on created instance
PyValue *val = cls->CallMethod("getSource");

// Do stuff with result
CString Source;
val->GetValueAsString(Source);
MessageBox(Source);