clean-local:
	$(MAKE) pre-clean-local-hook
	$(MAKE) $(CONFIG)_BeforeClean
	-rm -f $(call quote_each,$(CLEANFILES))
	$(MAKE) $(CONFIG)_AfterClean
	$(MAKE) post-clean-local-hook

install-local:
uninstall-local:

q2quote = '$(subst ?, ,$1)'
quote_each = $(foreach f,$(call s2q,$1),$(call q2quote,$f))

dist-local:
	$(MAKE) pre-dist-local-hook "distdir=$$distdir"
	for f in Makefile $(call quote_each,$(EXTRA_DIST)); do \
		d=`dirname "$$f"`; \
		test -d "$(distdir)/$$d" || \
			mkdir -p "$(distdir)/$$d"; \
		cp -p "$$f" "$(distdir)/$$d" || exit 1; \
	done
	$(MAKE) post-dist-local-hook "distdir=$$distdir"

dist-local-recursive:
	for dir in $(call quote_each,$(SUBDIRS)); do \
		mkdir -p "$(distdir)/$$dir" || true; \
		case "$$dir" in \
		.) $(MAKE) dist-local "distdir=$(distdir)" || exit 1;; \
		*) (cd "$$dir"; $(MAKE) dist-local "distdir=$(distdir)/$$dir") || exit 1; \
		esac \
	done

#hooks: Available hooks - all, clean, install, uninstall and dist
#	and their *-local variants
pre-%-hook: ; @:
post-%-hook: ; @:

#targets for custom commands
%_BeforeBuild: ; @:
%_AfterBuild: ; @:
%_BeforeClean: ; @:
%_AfterClean: ; @:
