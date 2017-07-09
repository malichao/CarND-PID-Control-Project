FROM ubuntu:14.04

FROM gcc:5.4

MAINTAINER Malcolm Ma <malichaooo@gmail.com>

# Arguments
ARG user
ARG uid
ARG gid
ARG home
ARG shell

# Basic Utilities
RUN apt-get -y update && apt-get install -y sudo ssh build-essential git

# Install cmake 3.9
RUN wget https://cmake.org/files/v3.9/cmake-3.9.0-rc5.tar.gz && \
	tar xf cmake-3.9.0-rc5.tar.gz && \
	cd cmake-3.9.0-rc5 && ./configure && make -j && make install && \
	cd .. && rm -rf cmake*

# Install dependencies for uWebSockets
RUN apt-get install -y apt-utils libssl-dev autotools-dev

# Build libuv1 from source as it's not available on ubuntu 14
RUN wget https://launchpad.net/ubuntu/+archive/primary/+files/libuv1_1.9.1.orig.tar.gz && \
	tar xf libuv1_1.9.1.orig.tar.gz && \
	cd libuv-v1.9.1 && ./autogen.sh && ./configure && make -j && \
	make check && make install && \
	cd .. && rm -rf libuv* && ldconfig

# Install uWebSockets
RUN git clone https://github.com/uWebSockets/uWebSockets && cd uWebSockets && \
	git checkout e94b6e1 && mkdir build && cd build && cmake .. && make -j &&\
	make install && cd ../.. && ln -s /usr/lib64/libuWS.so /usr/lib/libuWS.so &&\
	rm -r uWebSockets

# Install IPOPT
ADD install_ipopt.sh .

RUN wget https://www.coin-or.org/download/source/Ipopt/Ipopt-3.12.7.tgz &&\
	 tar xf Ipopt-3.12.7.tgz && rm Ipopt-3.12.7.tgz
RUN bash install_ipopt.sh Ipopt-3.12.7

# Install CppAD
RUN wget http://www.coin-or.org/download/source/CppAD/cppad-20170708.gpl.tgz && \
	tar xf cppad-20170708.gpl.tgz && rm cppad-20170708.gpl.tgz && ldconfig

RUN cd cppad-20170708 && mkdir build && cd build &&\
cmake 										\
    -D CMAKE_VERBOSE_MAKEFILE=YES						\
    -G "Unix Makefiles"							   	\
     \
    -D cppad_prefix=/usr/local							\
    -D cppad_postfix=""								\
     \
    -D cmake_install_includedirs=include					\
    -D cmake_install_libdirs=lib						\
     \
    -D cmake_install_datadir=share						\
    -D cmake_install_docdir=share/doc						\
    \
    -D ipopt_prefix=/usr/local/lib						\
    \
    -D cppad_cxx_flags="-Wall -ansi -pedantic-errors -std=c++11 -Wshadow"	\
    \
    -D cppad_testvector=cppad							\
    -D cppad_max_num_threads=48							\
    -D cppad_tape_id_type=size_t						\
    -D cppad_tape_addr_type=size_t						\
    -D cppad_debug_which=debug_none						\
    -D cppad_deprecated=NO		                                   \
    \
    .. && make -j4 && make -j4 check && make -j4 install

RUN rm -rf /var/lib/apt/lists/*

# Install python development file used by matplot
RUN apt-get update -y && apt-get install -y python2.7-dev python-matplotlib

# Flask Server
EXPOSE 4567

# Latest X11 / mesa GL
RUN apt-get update -y && apt-get install -y x11-apps

# Mount the user's home directory
VOLUME "${home}"

RUN mkdir -p /home/${user} && \
    echo "${user}:x:${uid}:${gid}:${user},,,:/home/${user}:/bin/bash" >> /etc/passwd && \
    echo "${user}:x:${uid}:" >> /etc/group && \
    echo "${user} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/${user} && \
    chmod 0440 /etc/sudoers.d/${user} && \
    chown ${uid}:${gid} -R /home/${user}

USER ${user}
ENV HOME /home/${user}

RUN echo ${shell}
