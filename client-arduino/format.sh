#!/usr/bin/env bash
clang-format -i sketch/*
cpplint \
    --quiet \
    --filter=-legal/copyright \
    --linelength=100 \
    --extensions=h,ino \
    sketch/*
