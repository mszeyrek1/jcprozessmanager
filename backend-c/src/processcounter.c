#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#define MAX_PROCESSES 2048

typedef struct {
    char pid[16];
    char name[64];
    char state[32];
    char vmsize[32];
    char ppid[16];
} ProcessInfo;
int is_numeric(const char *str) {
    int i = 0;
    while(str[i] != '\0') {
        if (!isdigit(str[i]))
            return 0;
        i++;
    }
    return 1;
}
int main() {
    ProcessInfo processes[MAX_PROCESSES];
    int count = 0;
    char path[512];
    char line[256];
    FILE *file;
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("Fehler beim Öffnen von /proc");
        return 1;
    }
    int max_pid = 3;    
    int max_name = 4;  
    int max_state = 5; 
    int max_vmsize = 6; 
    int max_ppid = 4;  

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (count >= MAX_PROCESSES)
            break;

        if (is_numeric(entry->d_name)) {
            ProcessInfo *p = &processes[count];
            strcpy(p->pid, entry->d_name);

            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            file = fopen(path, "r");
            if (file == NULL) {
                continue;
            }
            p->name[0] = '\0';
            p->state[0] = '\0';
            p->vmsize[0] = '\0';
            p->ppid[0] = '\0';

            while(fgets(line, sizeof(line), file) != NULL) {
                if (strncmp(line, "Name:", 5) == 0) {
                    sscanf(line, "Name:\t%63[^\n]", p->name);
                } else if (strncmp(line, "State:", 6) == 0) {
                    sscanf(line, "State:\t%31[^\n]", p->state);
                } else if (strncmp(line, "VmSize:", 7) == 0) {
                    sscanf(line, "VmSize:\t%31[^\n]", p->vmsize);
                } else if (strncmp(line, "PPid:", 5) == 0) {
                    sscanf(line, "PPid:\t%15[^\n]", p->ppid);
                }
            }
            fclose(file);
            int len = strlen(p->pid);
            if (len > max_pid) max_pid = len;

            len = strlen(p->name);
            if (len > max_name) max_name = len;

            len = strlen(p->state);
            if (len > max_state) max_state = len;

            len = strlen(p->vmsize);
            if (len > max_vmsize) max_vmsize = len;

            len = strlen(p->ppid);
            if (len > max_ppid) max_ppid = len;

            count++;
        }
    }
    closedir(dir);
    printf("%-*s  %-*s  %-*s  %-*s  %-*s\n",
           max_pid, "PID",
           max_name, "Name",
           max_state, "State",
           max_vmsize, "VmSize",
           max_ppid, "PPid");
    for (int i = 0; i < count; i++) {
        printf("%-*s  %-*s  %-*s  %-*s  %-*s\n",
               max_pid, processes[i].pid,
               max_name, processes[i].name,
               max_state, processes[i].state,
               max_vmsize, processes[i].vmsize,
               max_ppid, processes[i].ppid);
    }
    printf("Anzahl Prozesse: %d\n", count);

    return 0;
}
