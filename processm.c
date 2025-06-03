#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

void list_all_processes() {
    system("ps aux");
}

void list_by_user() {
    system("ps -eo user | sort -u | grep -v USER | xargs -I {} sh -c 'echo \"\nUser: {}\"; ps -u {} -o pid,stat,%cpu,%mem,start,command'");
}

void display_pids() {
    system("ps -eo pid");
}

void run_process() {
    char cmd[256];
    printf("Enter command to run: ");
    getchar();
    fgets(cmd, sizeof(cmd), stdin);


    cmd[strcspn(cmd, "\n")] = '\0';

    printf("[DEBUG] Command: '%s'\n", cmd);
    strcat(cmd, " &");
    system(cmd);
    printf("Process started in background\n");
}

void stop_process() {
    int pid;
    printf("Enter PID to stop: ");
    scanf("%d", &pid);


    if (kill(pid, 0) == -1) {
        perror("Error: Process does not exist or access denied");
    } else {
        if (kill(pid, SIGTERM) == 0) {
            printf("Process %d stopped\n", pid);
        } else {
            perror("Error stopping process");
        }
    }
}
void send_signal() {
    int pid;
    int choice;
    int sig;

    printf("Enter PID: ");
    scanf("%d", &pid);

    printf("\nChoose signal to send:\n");
    printf("1. SIGTERM (Terminate)\n");
    printf("2. SIGKILL (Kill immediately)\n");
    printf("3. SIGSTOP (Stop execution)\n");
    printf("4. SIGCONT (Continue execution)\n");
    printf("5. SIGINT  (Interrupt)\n");
    printf("6. Custom signal number\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            sig = SIGTERM;
            break;
        case 2:
            sig = SIGKILL;
            break;
        case 3:
            sig = SIGSTOP;
            break;
        case 4:
            sig = SIGCONT;
            break;
        case 5:
            sig = SIGINT;
            break;
        case 6:
            printf("Enter signal number: ");
            scanf("%d", &sig);
            break;
        default:
            printf("Invalid choice.\n");
            return;
    }

    if (kill(pid, sig) == 0) {
        printf("Signal %d sent to process %d\n", sig, pid);
    } else {
        perror("Error sending signal");
    }
}


int main() {
    char choice;

    do {
        system("clear");
        printf("=====================\n");
        printf("   Process Manager   \n");
        printf("=====================\n");
        printf("A. List all processes\n");
        printf("B. List by user\n");
        printf("C. Display PIDs\n");
        printf("D. Run/Stop process\n");
        printf("E. Send signal\n");
        printf("F. Exit\n");
        printf("=====================\n");
        printf("Enter choice: ");
        scanf(" %c", &choice);

        switch(toupper(choice)) {
            case 'A':
                list_all_processes();
                break;
            case 'B':
                list_by_user();
                break;
            case 'C':
                display_pids();
                break;
            case 'D':
                printf("1. Run process\n2. Stop process\n");
                int sub_choice;
                scanf("%d", &sub_choice);
                if (sub_choice == 1) run_process();
                else if (sub_choice == 2) stop_process();
                break;
            case 'E':
                send_signal();
                break;
            case 'F':
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice!\n");
        }
        printf("\nPress Enter to continue...");
        getchar();
        while (getchar() != '\n');
    } while (1);

        return 0;
}
