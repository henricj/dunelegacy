SUBDIRS = src tests

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = include \
             IDE/codeblocks/dunelegacy.cbp \
             IDE/xCode \
             debian/changelog \
             debian/compat \
             debian/control \
             debian/copyright \
             debian/dirs \
             debian/docs \
             debian/rules \
             data/LEGACY.PAK \
             data/OPENSD2.PAK \
             data/GFXHD.PAK \
             data/Dune2-Versions.txt \
             data/maps \
             data/locale \
             Doxyfile \
             ToDo.txt \
             SoftwareDesign.txt \
             buildclangdebug.sh \
             buildlocal.sh \
             builddebug.sh \
             buildcrosswin32.sh \
             runUnitTests.sh \
             dunelegacy.png \
             dunelegacy.svg \
             dunelegacy-128x128.png \
             dunelegacy.ico \
             dunelegacy.icns \
             dunelegacy.desktop \
             MacOSX \
             dunelegacy.spec \
             dunelegacy-0.96.4.ebuild \
             dunelegacy.nsi \
             nsis-background1.bmp \
             nsis-background2.bmp \
             clean.sh \
             scanbuild.sh \
             updatelocale.sh \
             resource.rc

datafiles_DATA = data/LEGACY.PAK \
				 data/OPENSD2.PAK \
				 data/GFXHD.PAK \
                 data/Dune2-Versions.txt

datafilesdir = $(dunelegacydatadir)

install-data-local:
	@$(NORMAL_INSTALL)
	$(MKDIR_P) $(DESTDIR)$(dunelegacydatadir)/locale
	$(INSTALL_DATA) data/locale/*.pot $(DESTDIR)$(dunelegacydatadir)/locale/
	$(INSTALL_DATA) data/locale/*.po $(DESTDIR)$(dunelegacydatadir)/locale/
	$(MKDIR_P) $(DESTDIR)$(dunelegacydatadir)/maps/singleplayer
	$(INSTALL_DATA) data/maps/singleplayer/*.ini $(DESTDIR)$(dunelegacydatadir)/maps/singleplayer/
	$(MKDIR_P) $(DESTDIR)$(dunelegacydatadir)/maps/multiplayer
	$(INSTALL_DATA) data/maps/multiplayer/*.ini $(DESTDIR)$(dunelegacydatadir)/maps/multiplayer/


uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -f $(DESTDIR)$(dunelegacydatadir)/locale
	rm -f $(DESTDIR)$(dunelegacydatadir)/maps

dist-hook:
	rm -rf `find $(distdir) -name .svn`
	rm -rf $(distdir)/IDE/xCode/Dune\ Legacy.xcodeproj/.svn
