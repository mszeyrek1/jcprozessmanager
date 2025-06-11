#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PROCESSES 2048

typedef struct {
    char pid[16];
    char name[64];
    char state[32];
    char vmsize[32];
    char ppid[16];
    char cputime[64];
    char rss[32];
    double cpu_percent;
} ProcessInfo;

int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

int cmp_pid(const void *a, const void *b) {
    return atoi(((ProcessInfo *)a)->pid) - atoi(((ProcessInfo *)b)->pid);
}
int cmp_name(const void *a, const void *b) {
    return strcmp(((ProcessInfo *)a)->name, ((ProcessInfo *)b)->name);
}
int cmp_state(const void *a, const void *b) {
    return strcmp(((ProcessInfo *)a)->state, ((ProcessInfo *)b)->state);
}
int cmp_vmsize(const void *a, const void *b) {
    return atoi(((ProcessInfo *)a)->vmsize) - atoi(((ProcessInfo *)b)->vmsize);
}
int cmp_ppid(const void *a, const void *b) {
    return atoi(((ProcessInfo *)a)->ppid) - atoi(((ProcessInfo *)b)->ppid);
}
int cmp_cpu(const void *a, const void *b) {
    double diff = ((ProcessInfo *)b)->cpu_percent - ((ProcessInfo *)a)->cpu_percent;
    return (diff > 0) - (diff < 0);
}
int cmp_rss(const void *a, const void *b) {
    return atoi(((ProcessInfo *)b)->rss) - atoi(((ProcessInfo *)a)->rss);
}

int main() {
    ProcessInfo processes[MAX_PROCESSES];
    int count = 0;
    char pathStatus[512], pathStat[512], line[1024];
    FILE *fileStatus, *fileStat, *uptimeFile;
    double uptime = 0;
    uptimeFile = fopen("/proc/uptime", "r");
    if (uptimeFile) {
        fscanf(uptimeFile, "%lf", &uptime);
        fclose(uptimeFile);
    }
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Fehler beim Öffnen von /proc");
        return 1;
    }
    int max_pid = 3, max_name = 4, max_state = 5, max_vmsize = 6, max_ppid = 4, max_cputime = 8;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && count < MAX_PROCESSES) {
        if (!is_numeric(entry->d_name)) continue;
        ProcessInfo *p = &processes[count];
        strcpy(p->pid, entry->d_name);
        snprintf(pathStatus, sizeof(pathStatus), "/proc/%s/status", entry->d_name);
        snprintf(pathStat, sizeof(pathStat), "/proc/%s/stat", entry->d_name);
        fileStatus = fopen(pathStatus, "r");
        fileStat = fopen(pathStat, "r");
        if (!fileStatus || !fileStat) {
            if (fileStatus) fclose(fileStatus);
            if (fileStat) fclose(fileStat);
            continue;
        }
        p->name[0] = p->state[0] = p->vmsize[0] = p->ppid[0] = p->cputime[0] = p->rss[0] = '\0';
        p->cpu_percent = 0.0;
        while (fgets(line, sizeof(line), fileStatus)) {
            if (strncmp(line, "Name:", 5) == 0)
                sscanf(line, "Name:\t%63[^\n]", p->name);
            else if (strncmp(line, "State:", 6) == 0)
                sscanf(line, "State:\t%31[^\n]", p->state);
            else if (strncmp(line, "VmSize:", 7) == 0)
                sscanf(line, "VmSize:\t%31[^\n]", p->vmsize);
            else if (strncmp(line, "PPid:", 5) == 0)
                sscanf(line, "PPid:\t%15[^\n]", p->ppid);
        }
        if (fgets(line, sizeof(line), fileStat)) {
            char *token = strtok(line, " ");
            unsigned long utime = 0, stime = 0, starttime = 0, rss_pages = 0;
            for (int field = 1; token && field <= 25; field++) {
                if (field == 14) utime = strtoul(token, NULL, 10);
                else if (field == 15) stime = strtoul(token, NULL, 10);
                else if (field == 22) starttime = strtoul(token, NULL, 10);
                else if (field == 25) rss_pages = strtoul(token, NULL, 10);
                token = strtok(NULL, " ");
            }
            long clk_tck = sysconf(_SC_CLK_TCK);
            long page_size = getpagesize();
            double total_time = (double)(utime + stime) / clk_tck;
            double seconds = uptime - ((double)starttime / clk_tck);
            double cpu = (seconds > 0) ? (total_time / seconds) * 100.0 : 0.0;
            long rss_kb = (rss_pages * page_size) / 1024;

            snprintf(p->cputime, sizeof(p->cputime), "%.2fs (%.1f%%)", total_time, cpu);
            snprintf(p->rss, sizeof(p->rss), "%ld", rss_kb);
            p->cpu_percent = cpu;
        }
        fclose(fileStatus);
        fclose(fileStat);
        if ((int)strlen(p->pid) > max_pid) max_pid = strlen(p->pid);
        if ((int)strlen(p->name) > max_name) max_name = strlen(p->name);
        if ((int)strlen(p->state) > max_state) max_state = strlen(p->state);
        if ((int)strlen(p->vmsize) > max_vmsize) max_vmsize = strlen(p->vmsize);
        if ((int)strlen(p->ppid) > max_ppid) max_ppid = strlen(p->ppid);
        if ((int)strlen(p->cputime) > max_cputime) max_cputime = strlen(p->cputime);
        count++;
    }
    closedir(dir);
    char input[32];
    printf("\nSortieren nach [pid/name/state/vmsize/ppid/cputime/rss]: ");
    if (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;  // \n entfernen
        if (strcmp(input, "pid") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_pid);
        else if (strcmp(input, "name") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_name);
        else if (strcmp(input, "state") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_state);
        else if (strcmp(input, "vmsize") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_vmsize);
        else if (strcmp(input, "ppid") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_ppid);
        else if (strcmp(input, "cputime") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_cpu);
        else if (strcmp(input, "rss") == 0) qsort(processes, count, sizeof(ProcessInfo), cmp_rss);
        else printf("Ungültige Eingabe. Keine Sortierung durchgeführt.\n");
    }
    printf("[\n");
    for (int i = 0; i < count; i++) {
    printf("  {\n");
    printf("    \"pid\": \"%s\",\n", processes[i].pid);
    printf("    \"name\": \"%s\",\n", processes[i].name);
    printf("    \"state\": \"%s\",\n", processes[i].state);
    printf("    \"vmsize\": \"%s\",\n", processes[i].vmsize);
    printf("    \"ppid\": \"%s\",\n", processes[i].ppid);
    printf("    \"cputime\": \"%s\",\n", processes[i].cputime);
    printf("    \"rss\": \"%s\"\n", processes[i].rss);
    printf("  }%s\n", (i < count - 1) ? "," : "");
}
    printf("]\n");
    printf("\nAnzahl Prozesse: %d\n", count);
    return 0;
}
