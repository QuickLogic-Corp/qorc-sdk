#!/usr/bin/env bash

case "$1" in
   build)
   
   cd $2/GCC_Project 
   make -j
   ;;
   clean)
   
   cd $2/GCC_Project 
   make clean
   ;;
  *)
    echo -e "## Commands"
    echo -e '\n    build <path-to-app>: build the application'
    echo -e '\n    clean <path-to-app>: clean the application'    
    echo -e "\n\n## Running docker container"
    echo -e "\nThe qorc-sdk needs to be mounted to the volume qorc-sdk in the docker container "
    echo -e '\n    docker run -v "<absolute-path-to-qorc-sdk-on-host>:/qorc-sdk>" build <path-to-app>'
    echo -e '\n    docker run -v "<absolute-path-to-qorc-sdk-on-host>:/qorc-sdk>" clean <path-to-app>'
    echo -e "\nNote: <path-to-app> should start with /qorc-sdk/"
    echo -e "\n## Example build"
    echo -e '\n   docker run -v "<absolute-path-to-qorc-sdk-on-host>:/qorc-sdk>" build /qorc-sdk/qf_apps/qf_ssi_ai_app\n\n'
    exec "$@"
    ;;
esac