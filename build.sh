#!/bin/bash

gcc -o lexiserver ./src/main.c ./src/config.c ./src/server.c ./src/request.c -Wall