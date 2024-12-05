#ifndef JOB_H
#define JOB_H

void execute_bg(int job_id);
void execute_fg(int job_id);
void launch_job(Command *cmds,bool background);
void handle_terminal_signals(void);
void handle_sigint(int _);
void handle_sigstop(int _);
void handle_sigchld(int _);
void free_all_commands(Command *cmds);
void free_all_jobs(void);
void kill_job_by_status(int status);
void kill_job(int job_id,int signal);
void remove_job(pid_t pgid);
void update_job_status(pid_t pgid,int status);
Job *add_job(pid_t pgid,const char *command,int status);
int get_job_id(pid_t pgid);
Job *find_job_by_id(int job_id);
char *gc_strndup(const char *str,size_t length);
void add_argv(Command *cmd,const char *arg);
Command *add_command(Command *head,Command *new_cmd);
Command *new_command(void);
void disable_raw_mode(void);
void enable_raw_mode(void);

#endif
