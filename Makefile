ICON_PATH = .github/assets/fmw_32.png

icon: $(ICON_PATH)
	magick $(ICON_PATH) -depth 8 RGBA:icon.rgba
	xxd -i icon.rgba > icon.h
	coreutils rm -f icon.rgba
