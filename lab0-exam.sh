#!/bin/bash
make
touch hello_os
mkdir hello_os_dir
cp os_hello ./hello_os_dir
cp os_hello ./hello_os_dir/hello_os
rm os_hello
grep -r -n os_hello ./ > hello_os.txt
