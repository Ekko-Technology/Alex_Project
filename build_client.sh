#!/bin/bash

# Navigate to the script's directory (Laptop-Client)
cd "$(dirname "$0")"

# Define source files
SRC_FILES="TLS-client-lib/tls-alex-client.cpp libraries/make_tls_client.cpp libraries/tls_client_lib.cpp libraries/tls_pthread.cpp libraries/tls_common_lib.cpp"

# Define output binary
OUTPUT="tls-alex-client"

# Compilation flags
CXX=g++
FLAGS="-pthread -lssl -lcrypto"

echo "Compiling TLS Client..."
$CXX $SRC_FILES $FLAGS -o $OUTPUT

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable: $OUTPUT"
else
    echo "Compilation failed!"
fi


