#!/bin/bash

# Script will clone, compile and test yapgen parser generator.

# Usage example:
#   bash try_yapgen.sh

# PREREQUISITES: bash, git, python2.7 ...

ROOT_DIR=$(pwd)

# - clone repositories -
if [ ! -d cont ]
then
  git clone https://github.com/izuzanak/cont.git
fi

if [ ! -d yapgen ]
then
  git clone https://github.com/izuzanak/yapgen.git
fi

# - compile container generator -
cd cont/cont_build
sh build.sh

CONT_DIR=$(pwd)
if [[ ":$PATH:" != *":$CONT_DIR:"* ]]
then
    export PATH="${PATH:+"$PATH:"}$CONT_DIR"
fi

cd $ROOT_DIR

# - compile parser generator -
cd yapgen/yapgen_build
sh build.sh

cd $ROOT_DIR

# - run demo parsers -
YG_BUILD_DIR=yapgen/yapgen_build

while true
do
  case $1 in
    1)               RES_NAME=demo_parse/1 ;;
    2)               RES_NAME=demo_parse/2 ;;
    3)               RES_NAME=demo_parse/3 ;;
    demo_exp)        RES_NAME=demo_exp ;;
    jit_parser)      RES_NAME=jit_parser ;;
    jit_types)       RES_NAME=jit_types ;;
    json)            RES_NAME=json ;;
    pack_code)       RES_NAME=pack_code ;;
    packet_parser)   RES_NAME=packet_parser ;;
    regexp)          RES_NAME=regexp ;;
    string_format)   RES_NAME=string_format ;;
    uclang_parser)   RES_NAME=uclang_parser ;;
    xml)             RES_NAME=xml ;;
    xml_declaration) RES_NAME=xml_declaration ;;
    xml_reference)   RES_NAME=xml_reference ;;

    *) exit ;;
  esac

  $YG_BUILD_DIR/yapgen \
    --parser_descr $YG_BUILD_DIR/../build/rules/$RES_NAME.rules \
    --source $YG_BUILD_DIR/../build/rules/$RES_NAME.src

  shift
done

