AM_MAKEINFOFLAGS = --no-split

info_TEXINFOS += %D%/porting.texi

html-local: %D%/porting/index.html
%D%/porting/index.html: %D%/porting.texi %D%/$(am__dirstamp)
	$(AM_V_GEN)$(MAKEINFOHTML) $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) \
		--split=node $< -o $(@D)
