#!/bin/bash
set -e

# Update include paths in soem_rsl source files
cd /home/pnx/code/soem_interface/soem_rsl/soem_rsl/soem_rsl

# Update osal.h includes
find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i 's|"soem_rsl/osal/osal.h"|"../osal/osal.h"|g' {} \;
find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i 's|"soem_rsl/oshw/linux/oshw.h"|"../oshw/linux/oshw.h"|g' {} \;

cd ../oshw/linux
find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i 's|"soem_rsl/osal/osal.h"|"../../osal/osal.h"|g' {} \;
find . -type f \( -name "*.c" -o -name "*.h" \) -exec sed -i 's|"soem_rsl/oshw/linux/oshw.h"|"./oshw.h"|g' {} \;