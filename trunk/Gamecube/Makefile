ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif
 
export TOPDIR	:=	$(CURDIR)
 
default: release

release: cube wii

cube:
	$(MAKE) -f Makefile.gc BUILD=release

wii:
	$(MAKE) -f Makefile.Wii BUILD=release

debug: cube-debug wii-debug

cube-debug:
	$(MAKE) -f Makefile.gc BUILD=debug

wii-debug:
	$(MAKE) -f Makefile.Wii BUILD=debug

clean:
	$(MAKE) -f Makefile.gc clean
	$(MAKE) -f Makefile.Wii clean

