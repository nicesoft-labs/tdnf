Install first: build-essential, cmake, libcurl4-openssl-dev, gettext,
libsolv-dev, libsolv-tools, libgpgme-dev

Download and install https://ftp.osuosl.org/pub/rpm/releases/rpm-4.19.x/rpm-4.19.1.1.tar.bz2
than build against rpm-4.19.1.1

mkdir build && cd build
cmake ..
make
