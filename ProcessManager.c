// Process Manager GUI with Reliable Signal Feedback
// Compile with: gcc ProcessManager.c -o ProcessManager `pkg-config --cflags --libs gtk4`

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>

GtkWidget *output_view;

void append_text(const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
}

void clear_output() {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    gtk_text_buffer_set_text(buffer, "", -1);
}

void list_all_processes(GtkWidget *widget, gpointer data) {
    clear_output();
    DIR *dir = opendir("/proc");
    if (!dir) {
        append_text("Error: Could not open /proc directory\n");
        return;
    }

    struct dirent *entry;
    char buffer[512];
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(*entry->d_name)) {
            char path[256], cmd[256];
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
            FILE *fp = fopen(path, "r");
            if (fp) {
                if (fgets(cmd, sizeof(cmd), fp)) {
                    cmd[strcspn(cmd, "\n")] = 0;
                    snprintf(buffer, sizeof(buffer), "PID: %-10s CMD: %s\n", entry->d_name, cmd);
                    append_text(buffer);
                }
                fclose(fp);
            }
        }
    }
    closedir(dir);
}

void list_processes_by_user(GtkWidget *widget, gpointer data) {
    clear_output();
    DIR *dir = opendir("/proc");
    if (!dir) {
        append_text("Error: Could not open /proc directory\n");
        return;
    }

    struct dirent *entry;
    char buffer[512];
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(*entry->d_name)) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            FILE *fp = fopen(path, "r");
            if (!fp) continue;

            char line[256];
            uid_t uid = -1;
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    sscanf(line, "Uid:\t%u", &uid);
                    break;
                }
            }
            fclose(fp);

            if (uid != -1) {
                struct passwd *pw = getpwuid(uid);
                snprintf(buffer, sizeof(buffer), "User: %s, PID: %s\n",
                         pw ? pw->pw_name : "Unknown", entry->d_name);
                append_text(buffer);
            }
        }
    }
    closedir(dir);
}

void list_pids_only(GtkWidget *widget, gpointer data) {
    clear_output();
    DIR *dir = opendir("/proc");
    if (!dir) {
        append_text("Error: Could not open /proc directory\n");
        return;
    }

    struct dirent *entry;
    char buffer[64];
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(*entry->d_name)) {
            snprintf(buffer, sizeof(buffer), "%s\n", entry->d_name);
            append_text(buffer);
        }
    }
    closedir(dir);
}

static void dialog_response(GtkDialog *dialog, int response, gpointer user_data) {
    if (response == GTK_RESPONSE_OK) {
        GtkEntry *entry = GTK_ENTRY(user_data);
        const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
        pid_t pid;
        int sig;
        char feedback[256];

        // Validate input format
        if (sscanf(text, "%d %d", &pid, &sig) != 2) {
            snprintf(feedback, sizeof(feedback), "✗ Error: Use 'PID SIGNAL' (e.g., 1234 9)\n");
            append_text(feedback);
            gtk_window_destroy(GTK_WINDOW(dialog));
            return;
        }

        // Check if process exists
        if (kill(pid, 0) == -1) {
            switch (errno) {
                case ESRCH:
                    snprintf(feedback, sizeof(feedback), "✗ Error: Process %d does not exist\n", pid);
                    break;
                case EPERM:
                    snprintf(feedback, sizeof(feedback), "✗ Error: No permission to signal process %d\n", pid);
                    break;
                default:
                    snprintf(feedback, sizeof(feedback), "✗ Error: Cannot signal process %d (errno=%d)\n", pid, errno);
            }
            append_text(feedback);
            gtk_window_destroy(GTK_WINDOW(dialog));
            return;
        }

        // Send the actual signal
        if (kill(pid, sig) == 0) {
            snprintf(feedback, sizeof(feedback), "✓ Sent signal %d to PID %d\n", sig, pid);
        } else {
            snprintf(feedback, sizeof(feedback), "✗ Failed to send signal %d to PID %d (errno=%d)\n", sig, pid, errno);
        }
        append_text(feedback);
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

void send_signal(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Send Signal",
                                                    NULL,
                                                    0,
                                                    "_Send",
                                                    GTK_RESPONSE_OK,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "PID SIGNAL (e.g., 1234 9)");
    gtk_box_append(GTK_BOX(content_area), entry);

    g_signal_connect(dialog, "response", G_CALLBACK(dialog_response), entry);
    gtk_widget_show(dialog);
}

void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Process Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Create buttons
    GtkWidget *btn1 = gtk_button_new_with_label("List All Processes");
    GtkWidget *btn2 = gtk_button_new_with_label("List by User");
    GtkWidget *btn3 = gtk_button_new_with_label("List PIDs Only");
    GtkWidget *btn4 = gtk_button_new_with_label("Send Signal");

    // Configure output view
    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(output_view), TRUE);

    // Layout
    gtk_grid_attach(GTK_GRID(grid), btn1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn4, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), output_view, 0, 1, 4, 1);

    // Connect signals
    g_signal_connect(btn1, "clicked", G_CALLBACK(list_all_processes), NULL);
    g_signal_connect(btn2, "clicked", G_CALLBACK(list_processes_by_user), NULL);
    g_signal_connect(btn3, "clicked", G_CALLBACK(list_pids_only), NULL);
    g_signal_connect(btn4, "clicked", G_CALLBACK(send_signal), NULL);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.processmanager", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
