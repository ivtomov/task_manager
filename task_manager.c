#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

#define RESET_COLOR "\x1b[0m"
#define TASK1_COLOR "\x1b[34;47m" 
#define TASK2_COLOR "\x1b[32;43m" 
#define TASK3_COLOR "\x1b[35;46m" 
#define RED_TEXT "\x1b[31m"

void printError(const char* message) {
    printf("%s%s%s", RED_TEXT, message, RESET_COLOR);
}

bool task1Running = false;
bool task2Running = false;
bool task3Running = false;
int task1Parameter;
float task2Parameter;
char task3Parameter[256] = "";
int task1PauseMs = 0;
int task2PauseMs = 0;
int task3PauseMs = 0;

void writeToFile(const char* filename, const char* data) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s\n", data);
    fclose(file);
}

void displayFile(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to read file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, "Task 1")) {
            printf("%s%s%s", TASK1_COLOR, line, RESET_COLOR);
        } else if (strstr(line, "Task 2")) {
            printf("%s%s%s", TASK2_COLOR, line, RESET_COLOR);
        } else if (strstr(line, "Task 3")) {
            printf("%s%s%s", TASK3_COLOR, line, RESET_COLOR);
        } else {
            printf("%s", line);
        }
    }

    fclose(file);
}

void runTask(int taskNumber, bool start, int fd) {
    char data[1024];
    if (start) {
        int pauseMs;
        printf("Enter pause duration in milliseconds for Task %d: ", taskNumber);
        scanf("%d", &pauseMs);
        while (getchar() != '\n'); 
        
        switch (taskNumber) {
            case 1:
                printf("Starting Task 1...\n");
                snprintf(data, sizeof(data), "Task 1 started with int parameter: %d, pause duration: %d ms", task1Parameter, pauseMs);
                task1PauseMs = pauseMs;
                break;
            case 2:
                printf("Starting Task 2...\n");
                snprintf(data, sizeof(data), "Task 2 started with float parameter: %f, pause duration: %d ms", task2Parameter, pauseMs);
                task2PauseMs = pauseMs;
                break;
            case 3:
                printf("Starting Task 3...\n");
                snprintf(data, sizeof(data), "Task 3 started with string parameter: %s, pause duration: %d ms", task3Parameter, pauseMs);
                task3PauseMs = pauseMs;
                break;
            default:
                printf("Unknown task started\n");
                snprintf(data, sizeof(data), "Unknown task started");
                break;
        }
    } else {
        switch (taskNumber) {
            case 1:
                printf("Stopping Task 1...\n");
                snprintf(data, sizeof(data), "Task 1 stopped");
                break;
            case 2:
                printf("Stopping Task 2...\n");
                snprintf(data, sizeof(data), "Task 2 stopped");
                break;
            case 3:
                printf("Stopping Task 3...\n");
                snprintf(data, sizeof(data), "Task 3 stopped");
                break;
            default:
                printf("Unknown task stopped\n");
                snprintf(data, sizeof(data), "Unknown task stopped");
                break;
        }
    }
    
    printf("%s\n", data);
    
    write(fd, data, strlen(data));
    write(fd, "\n", 1); 
}

int main() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        close(pipefd[1]); 

        char buffer[1024];
        ssize_t numRead;
        while ((numRead = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
            buffer[numRead] = '\0';
            writeToFile("output.txt", buffer);
        }

        close(pipefd[0]); 
        exit(EXIT_SUCCESS);
    } else { 
        close(pipefd[0]); 

        int choice;
        do {
            printf("\nMenu:\n");
            printf("1. Start Task 1 (Enter Int Data)\n");
            printf("2. Start Task 2 (Enter Float Data)\n");
            printf("3. Start Task 3 (Enter String Data)\n");
            printf("4. Stop Task 1\n");
            printf("5. Stop Task 2\n");
            printf("6. Stop Task 3\n");
            printf("7. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);

            while (getchar() != '\n'); 

            switch (choice) {
                case 1:
                    printf("Enter an integer value for Task 1: ");
                    if (scanf("%d", &task1Parameter) != 1) {
                        printError("Invalid input. Please enter an integer.\n");
                        while (getchar() != '\n'); 
                        continue;
                    }
                    runTask(1, true, pipefd[1]);
                    break;
                case 2:
                    printf("Enter a float value for Task 2: ");
                    if (scanf("%f", &task2Parameter) != 1) {
                        printError("Invalid input. Please enter a float.\n");
                        while (getchar() != '\n');
                        continue;
                    }
                    runTask(2, true, pipefd[1]);
                    break;
                case 3:
                    printf("Enter a string for Task 3: ");
                    if (scanf(" %[^\n]s", task3Parameter) != 1) {
                        printError("Invalid input. Please enter a string.\n");
                        continue;
                    }
                    runTask(3, true, pipefd[1]);
                    break;
                case 4:
                    runTask(1, false, pipefd[1]);
                    break;
                case 5:
                    runTask(2, false, pipefd[1]);
                    break;
                case 6:
                    runTask(3, false, pipefd[1]);
                    break;
                case 7:
                    printf("Exiting...\n");
                    break;
                default:
                    printf("Invalid choice. Please select a valid option from the menu.\n");
            }
        } while (choice != 7);

        close(pipefd[1]); 

        printf("\nWaiting for child process to finish...\n");
        wait(NULL); 

        printf("\nFile processing completed.\n");
        displayFile("output.txt");
    }

    printf("\nExiting...\n");
    return 0;
}

