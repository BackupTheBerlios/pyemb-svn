PROJECTS = pyemb

OBJECTS_DIR = obj_$(if $(DEBUG),debug,release)
DESTDIR = bin_$(if $(DEBUG),debug,release)
pyemb_TARGETS = $(DESTDIR)/pyemb_$(PYEMB_VERSION).dll

$(pyemb_TARGETS)_SOURCES = \
    src/pysession.cpp \
    src/pyerror.cpp \
    src/pyvalue.cpp \
    src/pyclass.cpp

$(pyemb_TARGETS)_HEADERS = \
	src/pyembdef.h \
    src/cdebug.h \
    src/pysession.h \
    src/pyerror.h \
    src/pyvalue.h \
    src/pyclass.h

CXXFLAGS += /DPYEMB_DLL