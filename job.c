#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

#include "types.h"
#include "job.h"
#include "lexer.h"
#include "memory.h"
#include "parser.tab.h"

bool do_exit = false;

static Job *job_list = NULL;
static struct termios original_termios = {0};

void enable_raw_mode(void) {
  static struct termios raw = {0};

  tcgetattr(STDIN_FILENO, &original_termios);

  raw = original_termios;
  raw.c_lflag = ICANON | ECHO | ECHOE;
  raw.c_cc[VMIN] = 2;
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

Command *new_command(void) {
  Command *cmd = allocate_space(sizeof(Command));
  cmd->argc = 0;
  for (size_t i = 0; i < ARGV_MAX; i++)
    cmd->argv[i] = NULL;
  cmd->next = NULL;
  return cmd;
}

Command *add_command(Command *head, Command *new_cmd) {
  Command *tmp = head;
  while (tmp->next)
    tmp = tmp->next;
  tmp->next = new_cmd;
  return tmp->next;
}

void add_argv(Command *cmd, const char *arg) {
  cmd->argv[cmd->argc++] = gc_strndup(arg, strlen(arg));
}

Job *find_job_by_id(int job_id) {
  Job **current = &job_list;
  while (*current) {
    if ((*current)->job_id == job_id)
      return *current;
    current = &(*current)->next;
  }
  return NULL;
}

int get_job_id(pid_t pgid) {
  Job **current = &job_list;
  while (*current) {
    if ((*current)->pgid == pgid)
      return (*current)->job_id;
    current = &(*current)->next;
  }
  return -1;
}

Job *add_job(pid_t pgid, const char *command, int status) {
  Job *job = allocate_space(sizeof(Job));
  job->job_id = rand();
  job->pgid = pgid;
  job->command = gc_strndup(command, strlen(command));
  job->status = status;
  job->next = job_list;
  job_list = job;
  return job;
}

void update_job_status(pid_t pgid, int status) {
  Job **current = &job_list;
  while (*current) {
    if ((*current)->pgid == pgid) {
      (*current)->status = status;
      return;
    }
    current = &(*current)->next;
  }
}

void remove_job(pid_t pgid) {
  Job **current = &job_list;
  while (*current) {
    if ((*current)->pgid == pgid) {
      *current = (*current)->next;
      return;
    }
    current = &(*current)->next;
  }
}

void kill_job(int job_id, int signal) {
  Job *job = find_job_by_id(job_id);
  kill(-job->pgid, signal);
  remove_job(job->pgid);
}

void kill_job_by_status(int status) {
  Job **current = &job_list;
  while (*current) {
    if ((*current)->status == status)
      kill_job((*current)->job_id, SIGINT);
    current = &(*current)->next;
  }
}

void handle_sigchld(int _) {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      update_job_status(pid, JSTAT_Done);
      remove_job(pid);
    } else if (WIFSTOPPED(status)) {
      update_job_status(pid, JSTAT_Stopped);
    } else if (WIFCONTINUED(status)) {
      update_job_status(pid, JSTAT_Running);
    }
  }
}

void handle_sigstop(int _) {
  pid_t fg_pid = tcgetpgrp(STDIN_FILENO);
  if (fg_pid != getpid()) {
    kill(-fg_pid, SIGSTOP);
  }
}

void handle_sigint(int _) { fprintf(stderr, "\nSIGINT thrown\n"); }

void handle_terminal_signals(void) {
  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
  sigaction(SIGTTOU, &sa, NULL);
  sigaction(SIGTTIN, &sa, NULL);
  sigaction(SIGSTOP, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
}

void launch_job(Command *cmds, bool background) {
  int pipe_fds[2];
  int prev_fd = -1;
  pid_t pgid = 0;
  Job *job = NULL;
  Command **current_cmd = &cmds;

  while (*current_cmd) {
    if ((*current_cmd)->next != NULL) {
      if (pipe(pipe_fds) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
      }
    }

    pid_t pid = fork();
    if (pid == 0) {
      setpgid(0, 0);
      if (!background) {
        tcsetpgrp(STDIN_FILENO, getpid());
      }

      if (prev_fd != -1) {
        dup2(prev_fd, STDIN_FILENO);
        close(prev_fd);
      }

      if ((*current_cmd)->next != NULL) {
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);
        close(pipe_fds[0]);
      }

      signal(SIGINT, handle_sigint);
      signal(SIGSTOP, handle_sigstop);
      signal(SIGCHLD, handle_sigchld);

      execvp((*current_cmd)->argv[0], (char *const *)&(*current_cmd)->argv[0]);
      perror("execvp");
      exit(EXIT_FAILURE);
    } else if (pid > 0) {
      if (pgid == 0)
        pgid = pid;
      setpgid(pid, pgid);

      if (!job) {
        job = add_job(pgid, (*current_cmd)->argv[0], background);
      }

      if (prev_fd != -1)
        close(prev_fd);

      if ((*current_cmd)->next != NULL) {
        close(pipe_fds[1]);
        prev_fd = pipe_fds[0];
      }
    } else {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    current_cmd = &(*current_cmd)->next;
  }

  if (!background) {
    tcsetpgrp(STDIN_FILENO, pgid);
    int status;
    waitpid(-pgid, &status, WUNTRACED);

    if (WIFSTOPPED(status)) {
      update_job_status(pgid, JSTAT_Stopped);
    } else {
      remove_job(pgid);
    }

    tcsetpgrp(STDIN_FILENO, getpid());
  } else {
    fprintf(stderr, "[%d] %d\n", job->job_id, pgid);
  }

}

void execute_fg(int job_id) {
  Job *job = find_job_by_id(job_id);
  if (job) {
    tcsetpgrp(STDIN_FILENO, job->pgid);
    kill(-job->pgid, SIGCONT);
    waitpid(-job->pgid, NULL, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, getpid());
  }
}

void execute_bg(int job_id) {
  Job *job = find_job_by_id(job_id);
  if (job) {
    kill(-job->pgid, SIGCONT);
    job->status = JSTAT_Running;
  }
}

int main(int argc, char **argv) {
  atexit(free_regions);
  handle_terminal_signals();
  enable_raw_mode();

  for (;;) {
    printf("squash> ");
    fflush(stdout);

    int inchr = 0;
    char *prompt = allocate_space(LINE_SIZE);
    size_t cursor = 0;

    while (read(STDIN_FILENO, &inchr, 1)
		&& inchr != '\x04'
		&& inchr != '\x03'
		&& inchr != '\x1a'
		&& inchr != '\x00'
		&& inchr != '\r' 
		&& inchr != '\n') {
      prompt[cursor++] = inchr;
      if (cursor >= LINE_SIZE)
	break;
    }

    printf("\n");

    prompt[cursor] = ';';
    YY_BUFFER_STATE buffer = yy_scan_string(prompt);

    while (yyparse())
      ;

    yy_delete_buffer(buffer);

    if (inchr == '\x04' 
	|| inchr == '\x03' 
	|| inchr == '\x1a'
	|| inchr == '\x00'
	|| do_exit == true)
	break;
  }

  disable_raw_mode();
}
