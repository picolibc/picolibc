# -*- Makefile -*- for winsup/doc
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

man1_MANS =
man3_MANS =
man5_MANS =

doc_DATA = \
	cygwin-ug-net/cygwin-ug-net.pdf \
	cygwin-api/cygwin-api.pdf

htmldir = $(datarootdir)/doc

XMLTO=xmlto --skip-validation --with-dblatex
DOCBOOK2XTEXI=@DOCBOOK2XTEXI@ --xinclude --info --utf8trans-map=charmap

-include Makefile.dep

.PHONY: install-extra-man install-etc

all-local: Makefile.dep \
	cygwin-api/cygwin-api.html \
	cygwin-ug-net/cygwin-ug-net.html \
	faq/faq.html faq/faq.body \
	cygwin-ug-net/cygwin-ug-net-nochunks.html.gz \
	api2man.stamp intro2man.stamp utils2man.stamp \
	cygwin-api.info cygwin-ug-net.info

clean-local:
	rm -f Makefile.dep
	rm -f *.html *.html.gz
	rm -Rf cygwin-api cygwin-ug-net faq
	rm -f api2man.stamp intro2man.stamp utils2man.stamp
	rm -f *.1
	rm -f *.3
	rm -f *.5
	rm -f *.info* charmap

install-html-local: cygwin-ug-net/cygwin-ug-net.html cygwin-api/cygwin-api.html
	@$(MKDIR_P) $(DESTDIR)$(htmldir)/cygwin-ug-net
	$(INSTALL_DATA) cygwin-ug-net/*.html $(DESTDIR)$(htmldir)/cygwin-ug-net
	(cd $(DESTDIR)$(htmldir)/cygwin-ug-net && ln -f cygwin-ug-net.html index.html)
	$(INSTALL_DATA) $(srcdir)/docbook.css $(DESTDIR)$(htmldir)/cygwin-ug-net
	@$(MKDIR_P) $(DESTDIR)$(htmldir)/cygwin-api
	$(INSTALL_DATA) cygwin-api/*.html $(DESTDIR)$(htmldir)/cygwin-api
	(cd $(DESTDIR)$(htmldir)/cygwin-api && ln -f cygwin-api.html index.html)
	$(INSTALL_DATA) $(srcdir)/docbook.css $(DESTDIR)$(htmldir)/cygwin-api

install-extra-man: api2man.stamp intro2man.stamp utils2man.stamp
	@$(MKDIR_P) $(DESTDIR)$(man1dir)
	$(INSTALL_DATA) *.1 $(DESTDIR)$(man1dir)
	@$(MKDIR_P) $(DESTDIR)$(man3dir)
	$(INSTALL_DATA) *.3 $(DESTDIR)$(man3dir)
	@$(MKDIR_P) $(DESTDIR)$(man5dir)
	$(INSTALL_DATA) *.5 $(DESTDIR)$(man5dir)

install-info-local: cygwin-ug-net.info cygwin-api.info
	@$(MKDIR_P) $(DESTDIR)$(infodir)
	$(INSTALL_DATA) *.info* $(DESTDIR)$(infodir)

install-etc:
	@$(MKDIR_P) $(DESTDIR)$(sysconfdir)/postinstall
	$(INSTALL_SCRIPT) $(srcdir)/etc.postinstall.cygwin-doc.sh $(DESTDIR)$(sysconfdir)/postinstall/cygwin-doc.sh
	@$(MKDIR_P) $(DESTDIR)$(sysconfdir)/preremove
	$(INSTALL_SCRIPT) $(srcdir)/etc.preremove.cygwin-doc.sh $(DESTDIR)$(sysconfdir)/preremove/cygwin-doc.sh

install-data-hook: install-extra-man install-html-local install-info-local install-etc

uninstall-extra-man:
	for i in *.1 ; do \
	    rm -f $(DESTDIR)$(man1dir)/$$i ; \
	done
	for i in *.3 ; do \
	    rm -f $(DESTDIR)$(man3dir)/$$i ; \
	done
	for i in *.5 ; do \
	    rm -f $(DESTDIR)$(man5dir)/$$i ; \
	done

uninstall-html:
	for i in cygwin-ug-net/*.html ; do \
	    rm -f $(DESTDIR)$(htmldir)/$$i ; \
	done ;
	rm -f $(DESTDIR)$(htmldir)/cygwin-ug-net/index.html
	rm -f $(DESTDIR)$(htmldir)/cygwin-ug-net/docbook.css
	for i in cygwin-api/*.html ; do \
	    rm -f $(DESTDIR)$(htmldir)/$$i ; \
	done ;
	rm -f $(DESTDIR)$(htmldir)/cygwin-api/index.html
	rm -f $(DESTDIR)$(htmldir)/cygwin-api/docbook.css

uninstall-info:
	for i in *.info* ; do \
	    rm -f $(DESTDIR)$(infodir)/$$i ; \
	done ;

uninstall-etc:
	rm -f $(DESTDIR)$(sysconfdir)/postinstall/cygwin-doc.sh
	rm -f $(DESTDIR)$(sysconfdir)/preremove/cygwin-doc.sh

uninstall-hook: uninstall-extra-man uninstall-html uninstall-info uninstall-etc

# nochunks ug html is not installed, but will be deployed to website
cygwin-ug-net/cygwin-ug-net-nochunks.html.gz: $(cygwin-ug-net_SOURCES) html.xsl
	$(XMLTO) html-nochunks -m $(srcdir)/html.xsl $<
	@$(MKDIR_P) cygwin-ug-net
	cp cygwin-ug-net.html cygwin-ug-net/cygwin-ug-net-nochunks.html
	rm -f cygwin-ug-net/cygwin-ug-net-nochunks.html.gz
	gzip cygwin-ug-net/cygwin-ug-net-nochunks.html

cygwin-ug-net/cygwin-ug-net.html: $(cygwin-ug-net_SOURCES) html.xsl
	$(XMLTO) html -o cygwin-ug-net/ -m $(srcdir)/html.xsl $<

cygwin-ug-net/cygwin-ug-net.pdf: $(cygwin-ug-net_SOURCES) fo.xsl
	$(XMLTO) pdf -o cygwin-ug-net/ -m $(srcdir)/fo.xsl $<

utils2man.stamp: $(cygwin-ug-net_SOURCES) man.xsl
	$(XMLTO) man -m $(srcdir)/man.xsl $<
	@touch $@

cygwin-ug-net.info: $(cygwin-ug-net_SOURCES) charmap
	$(DOCBOOK2XTEXI) $(srcdir)/cygwin-ug-net.xml --string-param output-file=cygwin-ug-net

cygwin-api/cygwin-api.html: $(cygwin-api_SOURCES) html.xsl
	$(XMLTO) html -o cygwin-api/ -m $(srcdir)/html.xsl $<

cygwin-api/cygwin-api.pdf: $(cygwin-api_SOURCES) fo.xsl
	$(XMLTO) pdf -o cygwin-api/ -m $(srcdir)/fo.xsl $<

api2man.stamp: $(cygwin-api_SOURCES) man.xsl
	$(XMLTO) man -m $(srcdir)/man.xsl $<
	@touch $@

cygwin-api.info: $(cygwin-api_SOURCES) charmap
	$(DOCBOOK2XTEXI) $(srcdir)/cygwin-api.xml --string-param output-file=cygwin-api

# this generates a custom charmap for docbook2x-texi which has a mapping for &reg;
charmap:
	cp /usr/share/docbook2X/charmaps/texi.charmap charmap
	echo "ae (R)" >>charmap

intro2man.stamp: intro.xml man.xsl
	$(XMLTO) man -m $(srcdir)/man.xsl $<
	@echo ".so intro.1" >cygwin.1
	@touch $@

faq/faq.html: $(faq_SOURCES) html.xsl
	$(XMLTO) html -o faq -m $(srcdir)/html.xsl $(srcdir)/faq.xml
	sed -i 's;<a name="id[mp][0-9]*"></a>;;g' faq/faq.html

# faq body is not installed, but is intended to be deployed to website, where it
# can be SSI included in a framing page
faq/faq.body: faq/faq.html
	$(srcdir)/bodysnatcher.pl $<

Makefile.dep: cygwin-ug-net.xml cygwin-api.xml faq.xml intro.xml
	cd $(srcdir) && ./xidepend $^ > "$(CURDIR)/$@"
