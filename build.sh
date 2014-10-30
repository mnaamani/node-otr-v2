#!/bin/bash
export otr_version=`./check-otr-version.sh`

if [ "${otr_version}" == '3' ]
then
    echo "Found local version of libotr: ${otr_version}"
    make module
    exit
fi

if [ "${otr_version}" == '4' ]
then
    echo "Found local version of libotr: ${otr_version}"
    echo "module 'otr3' requires native libotr 3.2.1 on the system."
    exit
fi

echo "No libotr found on the system."
echo "On Debian/Ubuntu you can install it with the following command:"
echo "sudo apt-get install libotr2 libotr2-dev"
echo ""
echo "On MacOS 10.x you can install it using brew:"
echo "  brew install libotr"

