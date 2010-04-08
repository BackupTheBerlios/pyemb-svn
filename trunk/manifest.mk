MANIFEST_VERSION := 1
MVBUILD_VERSION := release_1-0-3

MODULE_LOCAL := development/components/pyemb
TAG_FILE := $(dir $(lastword $(MAKEFILE_LIST)))/CVS/Tag
VERSION_LOCAL := $(if $(wildcard $(TAG_FILE)),$(shell cut -c 2- $(TAG_FILE) | cut -d _ -f 2- | tr [:upper:] [:lower:]),head)

PYEMB_VERSION := $(VERSION_LOCAL)

PYTHON_MARKER = release_2-6-4

$(MODULE_LOCAL).$(VERSION_LOCAL)_DEPS := \
	development/thirdparty/python.$(PYTHON_MARKER)
	
$(MODULE_LOCAL).$(VERSION_LOCAL)_INCLUDEPATHS := include
$(MODULE_LOCAL).$(VERSION_LOCAL)_LIBS := bin_$(if $(DEBUG),debug,release)/pyemb_$(VERSION_LOCAL).lib
$(MODULE_LOCAL).$(VERSION_LOCAL)_DLLS = \
	$(MIKROV_ROOT)/development/components/pyemb_$(PYEMB_VERSION)/bin_$(if $(DEBUG),debug,release)/pyemb_$(PYEMB_VERSION).dll \
	$(MIKROV_ROOT)/development/components/pyemb_$(PYEMB_VERSION)/bin_$(if $(DEBUG),debug,release)/pyemb_$(PYEMB_VERSION).dll.manifest

