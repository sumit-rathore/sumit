

# Warning: This is an automatically generated file, do not edit!
# ICRON: Hacked in $PROJECT.  This will have to re-added if this makefile is ever re-generated
PROJECT ?= Release


srcdir=.
top_srcdir=.

include $(top_srcdir)/config.make

ifeq ($(CONFIG),RELEASE)
ASSEMBLY_COMPILER_COMMAND = gmcs
ASSEMBLY_COMPILER_FLAGS =  -noconfig -codepage:utf8 -warn:4 -optimize+
ASSEMBLY = bin/$(PROJECT)/LogReader.exe
ASSEMBLY_MDB = 
COMPILE_TARGET = exe
PROJECT_REFERENCES = 
BUILD_DIR = bin/$(PROJECT)/


endif

AL=al2
SATELLITE_ASSEMBLY_NAME=$(notdir $(basename $(ASSEMBLY))).resources.dll

BINARIES = \
	$(LOGREADER) $(LOGREADER_SCRIPT)


RESGEN=resgen2

LOGREADER = $(BUILD_DIR)/LogReader.exe
LOGREADER_SCRIPT = $(BUILD_DIR)/logreader

FILES = \
	LogReader.cs \
	LogReader.Designer.cs \
	Program.cs \
	Properties/AssemblyInfo.cs \
	Properties/Resources.Designer.cs \
	Properties/Settings.Designer.cs 

DATA_FILES = 

RESOURCES = \
	LogReader.resx,LogReader.LogReader.resources \
	Properties/Resources.resx,LogReader.Properties.Resources.resources 

EXTRAS = \
	Properties/Settings.settings \
	Resources/icron_fav.ico \
	logreader.in 

REFERENCES =  \
	System \
	System.Core \
	System.Data \
	System.Drawing \
	System.Windows.Forms \
	System.Xml

DLL_REFERENCES = 

CLEANFILES = $(BINARIES) 

#Targets
all-local: $(ASSEMBLY) $(BINARIES)  $(top_srcdir)/config.make


$(LOGREADER_SCRIPT):
    $(eval $(call emit-deploy-wrapper,LOGREADER_SCRIPT,logreader,x))


$(eval $(call emit_resgen_targets))
$(build_xamlg_list): %.xaml.g.cs: %.xaml
	xamlg '$<'


$(ASSEMBLY_MDB): $(ASSEMBLY)
$(ASSEMBLY): $(build_sources) $(build_resources) $(build_datafiles) $(DLL_REFERENCES) $(PROJECT_REFERENCES) $(build_xamlg_list) $(build_satellite_assembly_list)
	$(MAKE) pre-all-local-hook prefix=$(prefix)
	mkdir -p $(shell dirname $(ASSEMBLY))
	$(MAKE) $(CONFIG)_BeforeBuild
	$(ASSEMBLY_COMPILER_COMMAND) $(ASSEMBLY_COMPILER_FLAGS) -out:$(ASSEMBLY) -target:$(COMPILE_TARGET) $(build_sources_embed) $(build_resources_embed) $(build_references_ref)
	$(MAKE) $(CONFIG)_AfterBuild
	$(MAKE) post-all-local-hook prefix=$(prefix)

install-local: $(ASSEMBLY) $(ASSEMBLY_MDB)
	$(MAKE) pre-install-local-hook prefix=$(prefix)
	$(MAKE) install-satellite-assemblies prefix=$(prefix)
	mkdir -p '$(DESTDIR)$(libdir)/$(PACKAGE)'
	$(call cp,$(ASSEMBLY),$(DESTDIR)$(libdir)/$(PACKAGE))
	$(call cp,$(ASSEMBLY_MDB),$(DESTDIR)$(libdir)/$(PACKAGE))
	mkdir -p '$(DESTDIR)$(bindir)'
	$(call cp,$(LOGREADER),$(DESTDIR)$(bindir))
	$(MAKE) post-install-local-hook prefix=$(prefix)

uninstall-local: $(ASSEMBLY) $(ASSEMBLY_MDB)
	$(MAKE) pre-uninstall-local-hook prefix=$(prefix)
	$(MAKE) uninstall-satellite-assemblies prefix=$(prefix)
	$(call rm,$(ASSEMBLY),$(DESTDIR)$(libdir)/$(PACKAGE))
	$(call rm,$(ASSEMBLY_MDB),$(DESTDIR)$(libdir)/$(PACKAGE))
	$(call rm,$(LOGREADER),$(DESTDIR)$(bindir))
	$(MAKE) post-uninstall-local-hook prefix=$(prefix)
