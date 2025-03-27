#!/bin/bash

# Compiler
CXX=g++

# Source files
SRC="TLS-server-lib/tls-alex-server.cpp TLS-server-lib/tls_server_lib.cpp TLS-client-lib/tls_pthread.cpp TLS-server-lib/make_tls_server.cpp TLS-client-lib/tls_common_lib.cpp TLS-server-lib/serial.cpp TLS-server-lib/serialize.cpp"

# Output binary
OUT="tls-alex-server"

# Compiler flags
CXXFLAGS="-pthread -lssl -lcrypto"

# Compile
echo "Compiling $OUT..."
$CXX $SRC $CXXFLAGS -o $OUT

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Run ./$OUT"
else
    echo "Compilation failed!"
fi

