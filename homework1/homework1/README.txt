Using the following command on os1 to compile my code:
gcc --std=gnu99 -o movies main.c

Note: if you want my code to read your test csv file, after compiling my code, add your file name as an argument of the executable file. For example, if your test csv file is named "test.csv", then you need to use:
./movies test.csv
to run my code with your file. If you don't do that, by code will read the sample file by default. 
