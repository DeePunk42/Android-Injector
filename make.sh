#!/bin/zsh
$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang \
	$1.c -o $1 \
	-m32 \
&& adb push $1 /data/local/tmp
