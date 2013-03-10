/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>
 
 This file is part of Open Orbit.
 
 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <pthread.h>
#include <histedit.h>
#include <stdlib.h>
#include <string.h>

#include <openorbit/log.h>
#include <gencds/avl-tree.h>

#include "terminal.h"
#include "res-manager.h"
#include "palloc.h"


// Length of terminal buffer
#define TERM_BUFF_LEN 1000

static pthread_t terminal_thread;
static avl_tree_t *commands = NULL;

typedef struct {
  const char *name;
  int (*cmd)(int argc, const char* argv[]);
  const char *synopsis;
  const char *help;
} terminal_command_t;

const char* terminal_prompt(EditLine *e) {
  return "openorbit> ";
}

int
terminal_help_cmd(int argc, const char *argv[argc])
{
  if (argc != 2) {
    terminal_command_t *cmd = avl_find(commands, "help");

    fprintf(stdout, "%s\n", cmd->help);
    return 1;
  }

  terminal_command_t *cmd = avl_find(commands, argv[1]);
  if (!cmd) {
    fprintf(stdout, "help: unknown command '%s'\n", argv[1]);
    return 1;
  }

  fprintf(stdout, "%s\n", cmd->help);
  return 0;
}

int
terminal_quit_cmd(int argc, const char *argv[argc])
{
  if (argc != 1) {
    return 1;
  }

  // TODO: Nicer exit
  exit(0);
  return 0;
}

void
list_node(avl_node_t *node)
{
  if (node == NULL) return;
  list_node(node->left);
  avl_str_node_t *sn = (avl_str_node_t*)node;
  terminal_command_t *cmd = node->data;
  
  fprintf(stdout, "\t%s: %s\n", sn->key, cmd->synopsis);
  list_node(node->right);
}

int
terminal_list_cmd(int argc, const char *argv[argc])
{
  if (argc != 1) {
    return 1;
  }
  

  avl_node_t *root = commands->root;
  list_node(root);
  
  return 0;
}



void
terminal_init_builtins(void)
{
  // Initialize terminal code and add help function
  commands = avl_str_new();
  static terminal_command_t hc;
  hc.name = "help";
  hc.synopsis = "show help for command";
  hc.help = "help [command-name]\n"
  "Shows help for the given command";
  hc.cmd = terminal_help_cmd;
  avl_insert(commands, "help", &hc);
  
  static terminal_command_t qc;
  qc.name = "quit";
  qc.synopsis = "quit open orbit";
  qc.help = "quit|exit\n"
            "exits openorbit";
  qc.cmd = terminal_quit_cmd;
  avl_insert(commands, "quit", &qc);
  avl_insert(commands, "exit", &qc);
  
  static terminal_command_t lc;
  lc.name = "list";
  lc.synopsis = "list all commands";
  lc.help = "list|commands\n"
            "Shows a list of all available commands";
  lc.cmd = terminal_list_cmd;
  avl_insert(commands, "list", &lc);
  avl_insert(commands, "commands", &lc);
}

void
terminal_add_command(const char *cmd,
                     int (*cmd_func)(int argc, const char* argv[]),
                     const char *short_help,
                     const char *help)
{
  if (!commands) {
    terminal_init_builtins();
  }

  if (avl_find(commands, cmd)) {
    log_error("terminal command '%s' already added", cmd);
    return;
  }

  terminal_command_t *c = smalloc(sizeof(terminal_command_t));
  c->name = strdup(cmd);
  c->synopsis = strdup(short_help);
  c->help = strdup(help);
  c->cmd = cmd_func;

  avl_insert(commands, cmd, c);
}

int
terminal_dispatch(int argc, const char *argv[argc])
{
  if (argc > 0) {
    terminal_command_t *cmd = avl_find(commands, argv[0]);
    if (!cmd) {
      fprintf(stdout, "cannot find '%s'\n", argv[0]);
      return -1;
    }
    return cmd->cmd(argc, argv);
  }
  return -1;
}

void*
terminal_main(void *arg)
{
  EditLine *el = NULL;
  History *hist = NULL;
  HistEvent ev;
  const char *line = NULL;
  int line_len = 0;

  el = el_init("openorbit", stdin, stdout, stderr);
  el_set(el, EL_PROMPT, terminal_prompt);
  el_set(el, EL_EDITOR, "emacs");
  el_set(el, EL_SIGNAL, 1); // Ensure resize is called

  hist = history_init();
  history(hist, &ev, H_SETSIZE, 800);
  el_set(el, EL_HIST, history, hist);

  el_source(el, NULL);

  // Create tokenizer with default splitting
  Tokenizer *tok = tok_init(NULL);
  while (1) {
    line = el_gets(el, &line_len);
  
    if (line_len > 0) {
      tok_reset(tok);
      int argc;
      const char **argv;
      int tok_res = tok_str(tok, line, &argc, &argv);
      if (tok_res == 0) {
        // Line valid, add to history and execute commands
        history(hist, &ev, H_ENTER, line);
        terminal_dispatch(argc, argv);
      }
    }
  }
  log_info("EOF on console, terminating\n");
  history_end(hist);
  el_end(el);
  tok_end(tok);
  return NULL;
}

void
terminal_start(void)
{
  if (!commands) {
    terminal_init_builtins();
  }

  pthread_create(&terminal_thread, NULL, terminal_main, NULL);
}

void
terminal_stop(void)
{
  pthread_cancel(terminal_thread);
}
