## Commands"

- build <path-to-app>: build the application
- clean <path-to-app>: clean the application

## Running docker container"

The qorc-sdk needs to be mounted to the volume qorc-sdk in the docker container

    docker run -v "<absolute-path-to-qorc-sdk-on-host>:/qorc-sdk>" build <path-to-app>

    docker run -v "<absolute-path-to-qorc-sdk-on-host>:/qorc-sdk>" clean <path-to-app>

    *Note: <path-to-app> should start with /qorc-sdk/*

## Example build"

docker run -v "<absolute-path-to-qorc-sdk-on-host>:/qorc-sdk>" build /qorc-sdk/qf_apps/qf_ssi_ai_app
