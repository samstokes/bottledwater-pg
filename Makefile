.PHONY: all install clean

DEBIAN_BRANCH = debian

# For git-buildpackage's mysterious purposes we need to tell it the revision
# of the upstream branch we are building from.  (We can't just give it the
# branch name because Travis' git clone doesn't include branch refs.)
# XXX update this if you merge in new changes from upstream!
DEBIAN_UPSTREAM_TAG = 7b864b2

all:
	$(MAKE) -C ext all
	$(MAKE) -C client all
	$(MAKE) -C kafka all

install:
	$(MAKE) -C ext install

clean:
	$(MAKE) -C ext clean
	$(MAKE) -C client clean
	$(MAKE) -C kafka clean

deb-snapshot:
	gbp dch --debian-branch=$(DEBIAN_BRANCH) --snapshot

deb-release:
	gbp dch --debian-branch=$(DEBIAN_BRANCH) --release --distribution=trusty --commit

deb-build:
	gbp buildpackage -us -uc -b --git-ignore-branch --git-upstream-tag=$(DEBIAN_UPSTREAM_TAG) --git-verbose --git-tag --git-ignore-new
