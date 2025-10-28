#!/bin/bash

# List of headers to fix with their new paths
declare -A header_paths=(
    ["osal.h"]="soem_rsl/osal/osal.h"
    ["osal_defs.h"]="soem_rsl/osal/linux/osal_defs.h"
    ["ethercattype.h"]="soem_rsl/soem_rsl/ethercattype.h"
    ["ethercatmain.h"]="soem_rsl/soem_rsl/ethercatmain.h"
    ["ethercatbase.h"]="soem_rsl/soem_rsl/ethercatbase.h"
    ["ethercatdc.h"]="soem_rsl/soem_rsl/ethercatdc.h"
    ["ethercateoe.h"]="soem_rsl/soem_rsl/ethercateoe.h"
    ["ethercatfoe.h"]="soem_rsl/soem_rsl/ethercatfoe.h"
    ["ethercatcoe.h"]="soem_rsl/soem_rsl/ethercatcoe.h"
    ["ethercatsoe.h"]="soem_rsl/soem_rsl/ethercatsoe.h"
    ["ethercatconfig.h"]="soem_rsl/soem_rsl/ethercatconfig.h"
    ["ethercatprint.h"]="soem_rsl/soem_rsl/ethercatprint.h"
    ["nicdrv.h"]="soem_rsl/oshw/linux/nicdrv.h"
    ["oshw.h"]="soem_rsl/oshw/linux/oshw.h"
)

# Also fix ethercat.h include
declare -A more_headers=(
    ["ethercat.h"]="soem_rsl/soem_rsl/ethercat.h"
)

# Combine both arrays
for key in "${!more_headers[@]}"; do
    header_paths[$key]=${more_headers[$key]}
done

# Process all .c and .h files
for file in $(find . -type f \( -name "*.c" -o -name "*.h" \)); do
    echo "Processing $file..."
    for header in "${!header_paths[@]}"; do
        path="${header_paths[$header]}"
        # Handle both quoted and bracketed includes
        sed -i "s|#include [<\"]${header}[>\"]|#include \"${path}\"|g" "$file"
    done
done