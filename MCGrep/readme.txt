#Parallel Version of Grep utilizing threads

The program has following functions:

void parse(char* filename, int numThread);
Takes input arguments as input filename and number of threads. Opens the input file from the filename. Memcopies the entire file.
Creates "numThread" number of output files. Breaks the input files into "numthread" number of chunks. Writes each chunk to an output file.
After writing, unmaps the input and output files form memory. Finally, closes all the streams

void* grep(int input);
If -r option is not raisedm this function searches through the specified text file and find all instances
of a single character and print each line that contains the specified character. If replace flag is raised,
the replacement character replaces any instance of the search character
that is found.  

void wrapToFile(char* filename, int numThread);
Takes number of threads. Opens the output file stream. Memcopies the chunk sized tmp .out files.
 Combines the tmp files into stdout.After writing, unmaps the input and output streams form memory. Finally, closes all the streams

void wrapToStdout(int numThread);
Same as above. Only prints out to stdout
