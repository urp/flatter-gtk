## Process this file with automake to produce Makefile.in

## Created by Anjuta

uidir = $(datadir)/gtk_flatter/ui
ui_DATA = \
	gl-view.ui\
	mmp-propagation-observer.ui \
	surface-import-dialog.ui \
	spring-solver.ui \
	gtk_flatdoc.ui \
	surface-drawable.ui \
	surface-model.ui

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(GTK_FLATTER_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = gtk_flatter \
	cli_flatter \
	cli_measure

cli_flatter_SOURCES = \
	common.cpp \
	common.h \
	he-mesh.cpp \
	he-mesh.h \
	mds-solver.cpp \
	mds-solver.h \
	mmp-common.h \
	mmp-eventpoint.cpp \
	mmp-eventpoint.h \
	mmp-geodesics.cpp \
	mmp-geodesics.h \
	mmp-queue.cpp \
	mmp-queue.h \
	mmp-trim_ac.cpp \
	mmp-trim_ac.h \
	mmp-utilities.cpp \
	mmp-utilities.h \
	mmp-window.cpp \
	mmp-window.h \
	quad-surface.cpp \
	quad-surface.h \
	solver.cpp \
	solver.h \
	spring-force.cpp \
	spring-force.h \
	spring-solver.cpp \
	spring-solver.h \
	surface.cpp \
	surface.h \
	surface-generators.cpp \
	surface-generators.h \
	gl-canvas.cpp \
	gl-canvas.h \
	gl-view.cpp \
	gl-view.h \
	surface-drawable.cpp \
	surface-drawable.h \
	view.cpp \
	view.h \
	mmp-visualizer.cpp \
	mmp-visualizer.h \
	flatter-cli-main.cpp \
	gl-tools.h \
	gl-tools.cpp

cli_flatter_CXXFLAGS = \
	-std=c++0x \
	-I$(top_srcdir)/../libutk/include

cli_flatter_LDADD = \
	-lboost_program_options \
	-lboost_signals \
	-lboost_graph \
	$(GTK_FLATTER_LIBS)

cli_measure_SOURCES =  \
	common.cpp \
	common.h \
	drawable.cpp \
	drawable.h \
	gl-canvas.cpp \
	gl-canvas.h \
	gl-view.cpp \
	gl-view.h \
	gl-view.ui \
	he-mesh.cpp \
	he-mesh.h \
	mmp-common.h \
	mmp-eventpoint.cpp \
	mmp-eventpoint.h \
	mmp-geodesics.cpp \
	mmp-geodesics.h \
	mmp-propagation-observer.ui \
	mmp-queue.cpp \
	mmp-queue.h \
	mmp-trim_ac.cpp \
	mmp-trim_ac.h \
	mmp-utilities.cpp \
	mmp-utilities.h \
	mmp-visualizer.cpp \
	mmp-visualizer.h \
	mmp-visualizer-cairo.cpp \
	mmp-visualizer-cairo.h \
	mmp-window.cpp \
	mmp-window.h \
	quad-surface.cpp \
	quad-surface.h \
	surface.cpp \
	surface.h \
	surface-drawable.cpp \
	surface-drawable.h \
	surface-generators.cpp \
	surface-generators.h \
	view.cpp \
	view.h \
	mmp-measure-cli-main.cpp \
	gl-tools.cpp \
	gl-tools.h

cli_measure_CXXFLAGS = -std=c++0x \
	-I$(top_srcdir)/../libutk/include \
	-DUSE_FLAT_MMP_VISUALIZE_GTK_OBSERVER

cli_measure_LDADD = -lboost_program_options \
	-lboost_signals \
	-lboost_graph \
	$(GTK_FLATTER_LIBS)

gtk_flatter_SOURCES = \
	surface-generators.h\
	surface-import-dialog.cpp \
	he-mesh.h \
	surface-import-dialog.h \
	interface.h \
	surface-drawable.cpp \
	controller.cpp \
	controller.h \
	spring-solver-interface.cpp \
	spring-solver-interface.h \
	mds-solver-interface.h \
	interface.cpp \
	surface-interface.cpp \
	surface-interface.h \
	quad-surface-interface.cpp \
	surface-drawable.h \
	surface.h \
	spring-solver.cpp \
	spring-solver.h \
	mds-solver.cpp \
	mds-solver.h \
	common.h \
	surface.cpp \
	mmp-visualizer.cpp \
	mmp-geodesics.h \
	mmp-geodesics.cpp \
	mmp-window.h \
	quad-surface.h \
	surface-generators.cpp \
	model.h \
	quad-surface-interface.h \
	mds-solver-interface.cpp \
	mmp-common.h \
	quad-surface.cpp \
	mmp-visualizer.h \
	image-export-dialog.cpp \
	mmp-visualizer-cairo.h \
	mmp-eventpoint.h \
	common.cpp \
	image-export-dialog.h \
	gl-view.cpp \
	drawable.cpp \
	drawable.h \
	view.h \
	gl-view.h \
	view.cpp \
	mmp-trim_ac.h \
	mmp-trim_ac.cpp \
	mmp-window.cpp \
	mmp-eventpoint.cpp \
	mmp-utilities.cpp \
	mmp-visualizer-cairo.cpp \
	gl-canvas.h \
	gl-canvas.cpp \
	mmp-utilities.h \
	mmp-queue.h \
	solver.h \
	mmp-queue.cpp \
	he-mesh.cpp \
	model.cpp \
	solver.cpp \
	spring-integrator.cpp \
	spring-integrator.h \
	spring-force.cpp \
	spring-force.h \
	flatter-gtk-main.cpp \
	gl-tools.cpp \
	gl-tools.h

gtk_flatter_CXXFLAGS = \
	-std=c++0x \
	-I$(top_srcdir)/../libutk/include

gtk_flatter_LDFLAGS = 

gtk_flatter_LDADD = -lboost_signals \
	-lboost_graph \
	$(GTK_FLATTER_LIBS)

EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/gtk_flatter
