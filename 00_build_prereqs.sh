#!/bin/sh
set -e

sudo yum -y install glibc-devel readline-devel ncurses-devel glibc-static readline-static ncurses-static byacc
sudo yum -y group install "Development Tools"
