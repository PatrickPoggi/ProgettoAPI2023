#!/bin/bash

# Compile the C program
# gcc your_program.c -o your_program

# Loop through test cases from 0 to 100
for number in {1..111}; do
    input_file="Tests/archivio_test_aperti/open_${number}.txt"
    output_file="Output_Miei/open_${number}_output_mio.txt"

    # Redirect input from the input file and output to the output file
    ./cmake-build-debug/untitled < "$input_file" > "$output_file"

    echo "Test ${number} completed."
done
