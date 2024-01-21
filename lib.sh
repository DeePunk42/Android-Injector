#!/bin/zsh
$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang \
	$1.c -o $1.so \
	-m32 \
  -shared \
  -llog \
&& adb push $1.so /data/app/com.example.crackme1-3yTR1T6Or9SL_kDjCEEfYg==/lib/arm:/data/app/com.example.crackme1-3yTR1T6Or9SL_kDjCEEfYg==
