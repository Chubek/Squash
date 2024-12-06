#ifndef TYPES_H
#define TYPES_H

#define LINE_SIZE 4096
#define ARGV_MAX 256

#define JSTAT_Running 1
#define JSTAT_Stopped 2
#define JSTAT_Done 3

typedef struct Arena Arena;

typedef struct Job {
  int job_id;
  pid_t pgid;
  char *command;
  int status;
  struct Job *next;
} Job;

typedef struct Command {
  int argc;
  const char *argv[ARGV_MAX];
  struct Command *next;
} Command;


#endif
