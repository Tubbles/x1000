# syntax=docker/dockerfile:1
FROM archlinux:latest AS x1000

RUN pacman -Syyuu --noconfirm --needed
RUN pacman -S --noconfirm --needed base-devel
RUN pacman -S --noconfirm --needed emscripten
RUN pacman -S --noconfirm --needed cmake
RUN pacman -S --noconfirm --needed ninja
RUN pacman -S --noconfirm --needed python
RUN pacman -S --noconfirm --needed python-pip

RUN python3 -m pip install conan

RUN pacman -S --noconfirm --needed libglvnd

RUN pacman -S --noconfirm --needed \
    libfontenc \
    libice \
    libsm \
    libxaw \
    libxcb \
    libxcomposite \
    libxcursor \
    libxdamage \
    libxdmcp \
    libxinerama \
    libxkbfile \
    libxrandr \
    libxres \
    libxss \
    libxtst \
    libxv \
    libxvmc \
    libxxf86vm \
    util-linux-libs \
    xcb-util \
    xcb-util-image \
    xcb-util-keysyms \
    xcb-util-renderutil \
    xcb-util-wm \
    xkeyboard-config \
    xtrans

RUN pacman -S --noconfirm --needed wget

# Install su-exec
RUN set -x ; wget -O "su-exec-0.2.tar.gz" "https://github.com/ncopa/su-exec/archive/refs/tags/v0.2.tar.gz" && \
    echo "ec4acbd8cde6ceeb2be67eda1f46c709758af6db35cacbcde41baac349855e25  su-exec-0.2.tar.gz" | sha256sum -c && \
    tar -xf "su-exec-0.2.tar.gz" && \
    cd "su-exec-0.2" || exit 1 && \
    gcc -O2 -Wall -Werror -o "su-exec" su-exec.c && \
    install -Dm755 "su-exec" "/usr/bin/su-exec"

ENV PS1='\u@\h \w\n\$ '

RUN pacman -S --noconfirm --needed libxft

# TODO : Does not seem to work
RUN passwd -d root

RUN pacman -S --noconfirm --needed vim

RUN sed -i 's,^PS1=.*$,PS1="\\u@\\h \\w\\n\$ ",g' /etc/bash.bashrc

RUN pacman -S --noconfirm --needed sdl2
RUN pacman -S --noconfirm --needed fmt
RUN pacman -S --noconfirm --needed sdl2_ttf

RUN pacman -S --noconfirm --needed catch2
RUN pacman -S --noconfirm --needed spdlog

COPY entrypoint.sh /root
ENTRYPOINT ["./entrypoint.sh"]

# FROM alpine:3.16.0 AS x1000

# RUN apk add build-base
# RUN apk add cmake
# RUN apk add python3
# RUN apk add py3-pip
# RUN apk add ninja

# RUN python3 -m pip install conan

# COPY conan-profile /root/.conan/profiles/default

# RUN apk add emscripten-fastcomp

# # RUN apk add wayland-dev

# # RUN apk add wayland
# # RUN apk add mesa-egl

# docker run --rm -it -v `pwd`:`pwd` -w `pwd`/build x1000
# docker run --rm -it -u $(id -u):$(id -g) -v `pwd`:`pwd` -w `pwd`/build x1000
# rm -fr * .* ; conan install .. --build=missing && cmake .. -G Ninja && cmake --build . && ./bin/hello
