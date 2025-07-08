//===============================HOW TO DUP DETEC======================

//have the Data organized in ../data
//make sure the Z1.csv Z2.csv ZY1.csv and ZY2.csv are in the data folder
//the data includes all the .tokenz lists 

run this program:

//run under Ubuntu 24.04.1 LST

//have g++ installed: My Flags (in case something fails)
    COLLECT_GCC=g++
    COLLECT_LTO_WRAPPER=/usr/libexec/gcc/x86_64-linux-gnu/13/lto-wrapper
    OFFLOAD_TARGET_NAMES=nvptx-none:amdgcn-amdhsa
    OFFLOAD_TARGET_DEFAULT=1
    Target: x86_64-linux-gnu
    Configured with: ../src/configure -v --with-pkgversion='Ubuntu 13.3.0-6ubuntu2~24.04' --with-bugurl=file:///usr/share/doc/gcc-13/README.Bugs --enable-languages=c,ada,c++,go,d,fortran,objc,obj-c++,m2 --prefix=/usr --with-gcc-major-version-only --program-suffix=-13 --program-prefix=x86_64-linux-gnu- --enable-shared --enable-linker-build-id --libexecdir=/usr/libexec --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --enable-bootstrap --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-libstdcxx-backtrace --enable-gnu-unique-object --disable-vtable-verify --enable-plugin --enable-default-pie --with-system-zlib --enable-libphobos-checking=release --with-target-system-zlib=auto --enable-objc-gc=auto --enable-multiarch --disable-werror --enable-cet --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-offload-targets=nvptx-none=/build/gcc-13-fG75Ri/gcc-13-13.3.0/debian/tmp-nvptx/usr,amdgcn-amdhsa=/build/gcc-13-fG75Ri/gcc-13-13.3.0/debian/tmp-gcn/usr --enable-offload-defaulted --without-cuda-driver --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --with-build-config=bootstrap-lto-lean --enable-link-serialization=2
    Thread model: posix
    Supported LTO compression algorithms: zlib zstd
    gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)


//have c++20

//then compile using:

g++ -fdiagnostics-color=always -g -std=c++20 -O3 main.cpp Evaluation_mngr.cpp Matching_mngr.cpp Parser_mngr.cpp -o dupDetec.out

//and run using:
./dupDetec.out


