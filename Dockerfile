FROM centos:6

RUN yum install -y centos-release-scl \
                   epel-release && \
    yum install -y devtoolset-7 \
                   curl
RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.1.0/cmake-3.1.0-Linux-x86_64.tar.gz | tar -xzC /usr/local --strip-components=1

RUN yum install -y m4 autoconf gettext

SHELL ["/usr/bin/scl", "enable", "devtoolset-7"]

ADD . /src
WORKDIR /build
RUN cmake /src && make -j
