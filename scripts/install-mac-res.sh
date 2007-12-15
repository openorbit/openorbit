#! /bin/sh

# install-mac-res.sh
# openorbit
#
# Created by Mattias Holm on 2007-09-23.
# Copyright 2007 Mattias Holm. All rights reserved.

# Installs a named resource file in a MacOS X bundle.
# syntax: install-mac-res.sh {file} {appname} [resource-dir]
# where file is the path of the resource file, appname the application name
# and resource-dir is an optional resource sub directory.

# Thus install-mac-res.sh ../res/foo.tga "Open Orbit" textures
# would install the file foo.tga in OpenOrbit.app/Contents/Resources/textures/

# This file is intended as support for the CMake build system.

#cp -f "$1" "${2}.app/Contents/Resources/${3}"
#else
#    cp -f "$1" "${2}.app/Contents/Resources/"
#fi


while [ $# != 0 ]
do
    case $1 in
    "-f")
        if [ -f $2 ]; then
            FILENAME=$2
            shift 2
        else
            echo $2: invalid file >&2
            exit 1
        fi
        ;;
    "-a")
        if [ -d "${2}.app" ]; then
            APPNAME="${2}.app"
            shift 2
        else
            echo $2.app: Unknown app bundle >&2
        fi
        ;;
    "-r")
        RESSUBDIR=${2:-}
        shift 2
        ;;
    *)
        echo $1: unknown option >&2
        exit 1
        ;;
    esac
done

RESDIR="${APPNAME}.app/Contents/Resources"
if [ ! -d ${RESDIR} ]; then
    if ! mkdir ${RESDIR}; then
        echo ${RESDIR}: could not create dir >&2
        exit 1
    fi
fi

cp -f "${FILENAME}" "${RESDIR}/${RESSUBDIR}"
