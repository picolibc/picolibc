XTENSA_BOARD_ESP=`echo $CC | sed 's/.*-mdynconfig=xtensa_\(.*\)\.so.*/\1/;s/.*-mcpu=\(^ *\).*/\1/;s/.* .*/unknown/'`
AC_SUBST([XTENSA_BOARD_ESP])
AM_CONDITIONAL([HAVE_XTENSA_BOARD_ESP32], [test x$XTENSA_BOARD_ESP = xesp32])
AM_CONDITIONAL([HAVE_XTENSA_BOARD_ESP32S3], [test x$XTENSA_BOARD_ESP = xesp32s3])
AM_CONDITIONAL([HAVE_XTENSA_BOARD_ESP], [echo $XTENSA_BOARD_ESP | grep -w -e esp32 -e esp32s3 >/dev/null 2>&1])
