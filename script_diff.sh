#!/bin/bash

# Compile the C program
# gcc your_program.c -o your_program

# Loop through test cases from 0 to 100
for number in {1..111}; do
    originale_file="Tests/archivio_test_aperti/open_${number}.output.txt"
    mio_file="Output_Miei/open_${number}_output_mio.txt"

    # Redirect input from the input file and output to the output file
    echo "diff #${number}"
    diff "$originale_file" "$mio_file"
done