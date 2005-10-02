TARGET=mingw32
PREFIX=/opt/cross-tools
CC=$PREFIX/bin/mingw32-gcc
#
CFLAGS="-mms-bitfields -DXTHREADS -D_REENTRANT -DXUSE_MTSAFE_API -I$PREFIX/include/gtk-2.0 -I$PREFIX/lib/gtk-2.0/include -I$PREFIX/include/atk-1.0 -I$PREFIX/include/pango-1.0 -I$PREFIX/include/glib-2.0 -I$PREFIX/lib/glib-2.0/include -I$PREFIX/include -Wall"
LDFLAGS="-L$PREFIX/lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoft2-1.0 -lpangowin32-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -liconv -lintl -lgthread-2.0"
PKG_LDFLAGS=" -lusb -Wall "
PKG_CFLAGS=" "
PKG_CPPFLAGS=" "
make "LDFLAGS=$LDFLAGS" "CFLAGS=$CFLAGS" "CC=$CC" "PREFIX=$PREFIX" "TARGET=$TARGET" "PKG_LDFLAGS=$PKG_LDFLAGS" "PKG_CFLAGS=$PKG_CFLAGS" "PKG_CPPFLAGS=$PKG_CPPFLAGS"

