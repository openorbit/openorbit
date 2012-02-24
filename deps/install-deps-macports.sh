#! /bin/sh

which -s port

if [ ! $? ] ; then
    echo "Cannot find Mac Ports"
    echo "Please visit http://www.macports.org"
    exit
fi

# Greadilly try to install necessary libraries. If a library is already
# installed the port command will simply ignore it.

# libjansson, a json parser needed to parse configuration files
port install jansson

# swig needed to build scripting support
port install swig
port install swig-python
