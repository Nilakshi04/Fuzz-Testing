#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_MUTATIONS 900   // Number of fuzzing iterations
#define MAX_CHANGES 500    // Maximum number of random mutations per iteration

// Function to randomly mutate a file
void mutate_file(const char *input_file, const char *output_file) {
    FILE *in, *out;
    unsigned char buffer[4096]; // Buffer to read file data
    size_t file_size, i, num_changes;
    
    in = fopen(input_file, "rb");
//if (!in) {
//perror("Error opening input file");
//exit(1);
//  }
    
    out = fopen(output_file, "wb");
//  if (!out) {
//        perror("Error creating output file");
//      fclose(in);
//      exit(1);
//  }
    
    // Read input file into buffer
    fseek(in, 0, SEEK_END);
    file_size = ftell(in);
    rewind(in);
    fread(buffer, 1, file_size, in);
    fclose(in);
    
    // Introduce random mutations
    srand(time(NULL));
    num_changes = rand() % MAX_CHANGES + 1; // At least 1 change
    for (i = 0; i < num_changes; i++) {
        size_t pos = rand() % file_size;
        buffer[pos] = rand() % 300; // Random byte mutation
    }
    
    // Write mutated data to new file
    fwrite(buffer, 1, file_size, out);
    fclose(out);
}

int main() {
    int i, status;
    char mutated_file[] = "mutated.jpg";
    char command[256];
    
    for (i = 0; i < MAX_MUTATIONS; i++) {
//        printf("[*] Running iteration %d...\n", i + 1);
        mutate_file("cross.jpg", mutated_file);
        
        // Construct command to execute jpeg2bmp with mutated input
        snprintf(command, sizeof(command), "./jpeg2bmp %s output.bmp 2> error.log", mutated_file);
        int ret = system(command);
        
        // Check if the process crashed (Segmentation fault, Abort, etc.)
        wait(&status);
        if (WIFSIGNALED(status)) {
            int signal = WTERMSIG(status);
//            printf("[!] Crash detected! Signal: %d (Segmentation Fault or Abort)\n", signal);
            
            // Print the error message from stderr
            FILE *error_log = fopen("error.log", "r");
            if (error_log) {
                char error_msg[512];
                while (fgets(error_msg, sizeof(error_msg), error_log)) {
                    if (strstr(error_msg, "Bug#")) { // Find the bug identifier
                        printf("[!] Found error: %s", error_msg);
                    }
                }
                fclose(error_log);
            }
        }
    }
    
    printf("[*] Fuzzing complete!\n");
    return 0;
}

