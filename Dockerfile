FROM debian:squeeze
MAINTAINER Leseratte

# This Dockerfile uses the precompiled devkitARM r32 to compile the d2xl-cIOS
# Set up package sources: 
RUN echo "deb http://archive.debian.org/debian/ squeeze main non-free contrib" > /etc/apt/sources.list && \
echo "deb http://archive.debian.org/debian-security/ squeeze/updates main non-free contrib" >> /etc/apt/sources.list && \
echo "deb http://archive.debian.org/debian squeeze-lts main" >> /etc/apt/sources.list && \
echo "Acquire::Check-Valid-Until false;" > /etc/apt/apt.conf

# Update and install a bunch of crap: 
RUN apt-get update -y --force-yes && \
apt-get install faketime -y --force-yes && \
faketime "2012-01-01" apt-get update -y --force-yes && \
faketime "2012-01-01" apt-get install -y --force-yes \
make gawk wget bzip2 unzip dos2unix libmpc-dev libelf-dev gcc g++ git

# Download the pre-built devkitARM r32 
# see https://github.com/Leseratte10/compile-devkitARM-r32 for sources
# or https://wii.leseratte10.de/devkitPro/devkitARM/r32%20(2010)/ for binaries.

RUN wget https://wii.leseratte10.de/devkitPro/file.php/devkitARM-r32-linux-custom.tar.gz --no-check-certificate

RUN tar -C /opt -xvf devkitARM-r32-linux-custom.tar.gz 

# At this point, we extracted the working devkitARM setup

COPY dist.sh /dist.sh

ENTRYPOINT ["/dist.sh"]


