## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

gtk_flatterdocdir = ${prefix}/doc/gtk_flatter
gtk_flatterdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


EXTRA_DIST = $(gtk_flatterdoc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(gtk_flatterdocdir)
