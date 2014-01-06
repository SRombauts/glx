SDKDIR=$HOME/CompilCroisee/ucineo-generic-V2.04
SYSROOT=$SDKDIR/i586-ucineo-linux-gnu/sysroot
CROSSTOOLS=$SDKDIR/bin/i586-ucineo-linux-gnu-

# Variables de compilation et de linkage
AR=$CROSSTOOLS'ar'
RANLIB=$CROSSTOOLS'ranlib'
CC=$CROSSTOOLS'gcc'
CXX=$CROSSTOOLS'g++'
CPPFLAGS='--sysroot='$SYSROOT
LDFLAGS='--sysroot='$SYSROOT

$CXX glx.cpp -lGL -lX11 -o glx -DAR=$AR -DRANLIB=$RANLIB -DCPPFLAGS=$CPPFLAGS -DLDFLAGS=$LDFLAGS

