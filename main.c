#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

typedef struct {
    int employee_id;
    int is_available;
    int current_session_id;
    int duration;  // Duration of the session in minutes
} Employee;

#define NUM_EMPLOYEES 10

// Shared memory would typically be setup here
Employee employees[NUM_EMPLOYEES];

void* employee_accompaniment(void* arg) {
    Employee* emp = (Employee*)arg;
    printf("Employee %d is starting accompaniment for session %d\n", emp->employee_id, emp->current_session_id);
    sleep(emp->duration);  // Simulate the time taken for the session
    printf("Employee %d has finished accompaniment for session %d\n", emp->employee_id, emp->current_session_id);
    emp->is_available = 1;  // Mark as available after session
    return NULL;
}

void load_settings(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening settings file");
        exit(1);
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Here you can expand to read more settings as needed
        // Example: sscanf(line, "num_of_employees=%d", &num_of_employees);
    }
    fclose(file);
}

int main() {
    load_settings("arguments.txt");  // Load simulation settings

    // Initialize employees
    for (int i = 0; i < NUM_EMPLOYEES; i++) {
        employees[i].employee_id = i;
        employees[i].is_available = 1;
        employees[i].current_session_id = i + 100;  // Sample session ID
        employees[i].duration = 2;  // Sample duration in minutes
    }

    pid_t pid;
    int status;

    for (int i = 0; i < NUM_EMPLOYEES; i++) {
        if (employees[i].is_available) {
            pid = fork();
            if (pid == -1) {
                perror("Failed to fork process");
                continue;
            }
            if (pid == 0) {  // Child process
                employees[i].is_available = 0;  // Mark employee as busy
                pthread_t thread;
                if (pthread_create(&thread, NULL, employee_accompaniment, (void*)&employees[i]) != 0) {
                    perror("Failed to create thread");
                    exit(1);
                }
                pthread_join(thread, NULL);
                exit(0);  // End child process
            }
        }
    }

    // Wait for all child processes to finish
    while (wait(&status) > 0);

    return 0;
}
