#!/bin/bash

set -e
asciidoc -a data-uri -a icons linux_tests_list.txt
firefox linux_tests_list.html
