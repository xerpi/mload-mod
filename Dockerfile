FROM debian:buster
MAINTAINER Leseratte

# This Dockerfile uses the precompiled devkitARM r32 to compile the d2xl-cIOS
# Update and install a bunch of crap: 
RUN apt-get update -y --force-yes && \
apt-get install -y --force-yes \
make gawk wget bzip2 unzip dos2unix libmpc-dev libelf-dev gcc g++ git libfl2 libfl-dev

# Download the pre-built devkitARM r32 
# see https://github.com/Leseratte10/compile-devkitARM-r32 for sources
# or https://wii.leseratte10.de/devkitPro/devkitARM/r32%20(2010)/ for binaries.

RUN wget https://wii.leseratte10.de/devkitPro/file.php/devkitARM-r32-linux_debian-buster.tar.gz --no-check-certificate

RUN tar -C /opt -xvf devkitARM-r32-linux_debian-buster.tar.gz 

# At this point, we extracted the working devkitARM setup

COPY dist.sh /dist.sh

ENTRYPOINT ["/dist.sh"]


