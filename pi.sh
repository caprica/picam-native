# Basic build script for building the shared library on the Pi
JNI_INCLUDE=/usr/lib/jvm/java-9-openjdk-armhf/include
JNI_LIB=/usr/lib/jvm/java-9-openjdk-armhf/lib
PI_INCLUDE=/opt/vc/include
PI_LIB=/opt/vc/lib
LDFLAGS="-lc -lmmal -lmmal_core -lmmal_util"
SRC="uk_co_caprica_picam_Camera.c Camera.c Configuration.c Defaults.c Encoder.c Port.c"
gcc -I"$JNI_INCLUDE" -I"$JNI_INCLUDE/linux" -I"$PI_INCLUDE" -L"$PI_LIB" -o picam.so -shared -Wl,-soname,picam.so $SRC $LDFLAGS
