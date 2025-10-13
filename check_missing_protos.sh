#!/bin/bash
cd cryo-protocol

echo "Checking for missing generated proto files..."
echo "=============================================="

missing_count=0

find proto -name "*.proto" | while read proto_file; do
    # Get relative path and basename
    rel_path="${proto_file#proto/}"
    dir_path=$(dirname "$rel_path")
    base_name=$(basename "$proto_file" .proto)

    # Check if generated file exists
    gen_file="generated/cpp/${dir_path}/${base_name}.pb.h"

    if [ ! -f "$gen_file" ]; then
        echo "MISSING: $proto_file -> $gen_file"
        missing_count=$((missing_count + 1))
    fi
done

echo "=============================================="
echo "Total proto files: $(find proto -name '*.proto' | wc -l)"
echo "Generated files: $(find generated/cpp -name '*.pb.h' -not -name '*.grpc.pb.h' | wc -l)"