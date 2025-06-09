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
    char cputime[32];
    char rss[32];
    double cpu_seconds;
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

int compare_by_cpu(const void *a, const void *b) {
    double cpuA = ((ProcessInfo *)a) ->cpu_seconds;
    double cpuB = ((ProcessInfo *)b) ->cpu_seconds;
    if (cpuA < cpuB) return 1;
    if (cpuA > cpuB) return-1;
    return 0;
}

int main() {
    ProcessInfo processes[MAX_PROCESSES];
    int count = 0;
    char pathStatus[512];
    char pathStat[512];
    char line[1024];
    FILE *fileStatus;
    FILE *fileStat;
    double uptime = 0;
    FILE *fileUptime = fopen("/proc/uptime", "r");
    if (fileUptime) {
        fscanf(fileUptime, "%lf", &uptime);
        fclose(fileUptime);
    }
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
    int max_cputime = 8;
    int max_rss = 3;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (count >= MAX_PROCESSES)
            break;

        if (is_numeric(entry->d_name)) {
            ProcessInfo *p = &processes[count];
            strcpy(p->pid, entry->d_name);

            snprintf(pathStatus, sizeof(pathStatus), "/proc/%s/status", entry->d_name);
            snprintf(pathStat, sizeof(pathStat),"/proc/%s/stat", entry->d_name);
            fileStatus = fopen(pathStatus, "r");
            fileStat = fopen(pathStat, "r");
            if (fileStatus == NULL || fileStat == NULL) {
                if (fileStatus) fclose(fileStatus);
                if (fileStat) fclose(fileStat);
                continue;
            }
            p->name[0] = '\0';
            p->state[0] = '\0';
            p->vmsize[0] = '\0';
            p->ppid[0] = '\0';
            p->cputime[0] = '\0';

            while(fgets(line, sizeof(line), fileStatus) != NULL) {
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
            if(fgets(line, sizeof(line), fileStat) != NULL) {
                char *token; 
                int field = 1;
                unsigned long utime = 0, stime = 0, starttime = 0, rss_pages = 0;
                token = strtok(line, " ");
                while (token != NULL) {
                    if (field == 14) {
                        utime = strtoul(token, NULL, 10);
                     } else if (field == 15) {
                        stime = strtoul(token, NULL, 10);
                     } else if (field == 22) {
                        starttime = strtoul(token, NULL, 10);
                     } else if (field == 24) {
                        rss_pages = strtoul(token, NULL, 10);
                        break;
                     }
                field++;
                token = strtok(NULL, " ");
                }
                long clk_tck = sysconf(_SC_CLK_TCK);
                long page_size = getpagesize();
                long rss_kb = (rss_pages * page_size) / 1024;
                double total_time = (double)(utime + stime) / clk_tck;
                double seconds = uptime - ((double)starttime / clk_tck);
                double cpu_usage = (seconds > 0) ? (total_time / seconds) * 100.00 : 0.0;
                if(clk_tck <= 0) {
                    fprintf(stderr, "Fehler: sysconf returned %ld\n", clk_tck);
                    return 1;
                }
                if (seconds > 0) {
                    cpu_usage = (total_time / seconds) * 100.0;
                }   
                p->cpu_seconds = total_time;
                snprintf(p->rss, sizeof(p->rss), "%ld KB", rss_kb); 
                snprintf(p->cputime, sizeof(p->cputime), "%.2fs (%.1f%%)", total_time, cpu_usage);

            }
              
            fclose(fileStatus);
            fclose(fileStat);
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

            len = strlen(p->cputime);
            if (len > max_cputime) max_cputime = len;

            len = strlen(p->rss);
            if (len > max_rss) max_rss = len;

            count++;
        }
    }
    closedir(dir);
    qsort(processes, count, sizeof(ProcessInfo), compare_by_cpu);
    printf("%-*s  %-*s  %-*s  %-*s  %-*s  %-*s %-*s\n",
           max_pid, "PID",
           max_name, "Name",
           max_state, "State",
           max_vmsize, "VmSize",
           max_ppid, "PPid",
           max_cputime, "CPUTime",
           max_rss, "RSS");
    for (int i = 0; i < count; i++) {
        printf("%-*s  %-*s  %-*s  %-*s  %-*s  %-*s %-*s\n",
               max_pid, processes[i].pid,
               max_name, processes[i].name,
               max_state, processes[i].state,
               max_vmsize, processes[i].vmsize,
               max_ppid, processes[i].ppid,
               max_cputime, processes[i].cputime,
               max_rss, processes[i].rss);
    }
    printf("Anzahl Prozesse: %d\n", count);

    return 0;
}
