#!/bin/zsh
make \
&& make evil.so \
&& adb push injector /data/local/tmp \
&& adb push evil.so /data/local/tmp
