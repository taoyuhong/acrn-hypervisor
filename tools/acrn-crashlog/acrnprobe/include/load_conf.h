/*
 * Copyright (C) 2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LOAD_CONF_H__
#define __LOAD_CONF_H__

#include <stdio.h>
#include <sys/queue.h>
#include <openssl/sha.h>
#include <ext2fs/ext2fs.h>
#include "event_queue.h"
#include "probeutils.h"

#define CONTENT_MAX 10
#define EXPRESSION_MAX 5
#define LOG_MAX 20
#define TRIGGER_MAX 20
#define SENDER_MAX 3
#define DATA_MAX 3
#define CRASH_MAX 20
#define INFO_MAX 20
#define VM_MAX 4
#define VM_EVENT_TYPE_MAX 20

struct trigger_t {
	char *name;
	char *type;
	char *path;
};

struct vm_t {
	char *name;
	char *channel;
	char *interval;
	char *syncevent[VM_EVENT_TYPE_MAX];

	ext2_filsys datafs;
	unsigned long history_size[SENDER_MAX];
	char *history_data;
	char last_synced_line_key[SENDER_MAX][SHORT_KEY_LENGTH + 1];
};

struct log_t {
	char *name;
	char *type;
	char *path;
	char *lines;

	void (*get)(struct log_t *, void *);
};

struct crash_t {
	char *name;
	char *channel;
	char *interval;
	struct trigger_t *trigger;
	char *content[CONTENT_MAX];
	char *mightcontent[EXPRESSION_MAX][CONTENT_MAX];
	struct log_t *log[LOG_MAX];
	char *data[DATA_MAX];

	struct crash_t *parents;

	TAILQ_ENTRY(crash_t) entries;
	TAILQ_HEAD(, crash_t) children;

	int wd;
	int level;
	struct crash_t *(*reclassify)(const struct crash_t *, const char*,
					char**, char**, char**);
};

struct info_t {
	char *name;
	char *channel;
	char *interval;
	struct trigger_t *trigger;
	struct log_t *log[LOG_MAX];
};

struct uptime_t {
	char *name;
	char *frequency;
	char *eventhours;

	int wd;
	char *path;
};

struct sender_t {
	char *name;
	char *outdir;
	char *maxcrashdirs;
	char *maxlines;
	char *spacequota;
	struct uptime_t *uptime;

	void (*send)(struct event_t *);
	char *log_vmrecordid;
	int sw_updated; /* each sender has their own record */
};

struct conf_t {
	struct sender_t *sender[SENDER_MAX];
	struct vm_t *vm[VM_MAX];
	struct trigger_t *trigger[TRIGGER_MAX];
	struct log_t *log[LOG_MAX];
	struct crash_t *crash[CRASH_MAX];
	struct info_t *info[INFO_MAX];
};

struct conf_t conf;

#define for_each_sender(id, sender, conf) \
		for (id = 0; \
		     id < SENDER_MAX && (sender = conf.sender[id]); \
		     id++)

#define for_each_trigger(id, trigger, conf) \
		for (id = 0; \
		     id < TRIGGER_MAX && (trigger = conf.trigger[id]); \
		     id++)

#define for_each_vm(id, vm, conf) \
		for (id = 0; \
		     id < VM_MAX && (vm = conf.vm[id]); \
		     id++)

#define for_each_syncevent_vm(id, event, vm) \
		for (id = 0; \
		     id < VM_EVENT_TYPE_MAX && (event = vm->syncevent[id]); \
		     id++)

#define for_each_info(id, info, conf) \
		for (id = 0; \
		     id < INFO_MAX && (info = conf.info[id]); \
		     id++)

#define for_each_log(id, log, conf) \
		for (id = 0; \
		     id < LOG_MAX && (log = conf.log[id]); \
		     id++)

#define for_each_crash(id, crash, conf) \
		for (id = 0; \
		     id < CRASH_MAX && (crash = conf.crash[id]); \
		     id++)

#define for_each_log_collect(id, log, type) \
		for (id = 0; \
		     id < LOG_MAX && (log = type->log[id]); \
		     id++)

#define for_each_content_crash(id, content, crash) \
		for (id = 0; \
		     id < CONTENT_MAX && (content = crash->content[id]); \
		     id++)

#define for_each_content_expression(id, content, exp) \
		for (id = 0; \
		     id < CONTENT_MAX && (content = exp[id]); \
		     id++)

#define exp_valid(exp) \
(__extension__ \
({ \
		int _ret = 0; \
		int _id; \
		char *content; \
		for_each_content_expression(_id, content, exp) { \
			if (content) \
				_ret = 1; \
		} \
		_ret; \
}) \
)

#define for_each_expression_crash(id, exp, crash) \
		for (id = 0; \
		     id < EXPRESSION_MAX && (exp = crash->mightcontent[id]); \
		     id++)

#define for_crash_children(crash, tcrash) \
			TAILQ_FOREACH(crash, &tcrash->children, entries)

#define is_leaf_crash(crash) \
			(crash && TAILQ_EMPTY(&crash->children))

#define is_root_crash(crash) \
			(crash && crash->parents == NULL)

#define to_collect_logs(type) \
(__extension__ \
({ \
		int _id; \
		int _ret = 0; \
		for (_id = 0; _id < LOG_MAX; _id++) \
			if (type->log[_id]) \
				_ret = 1; \
		_ret; \
}) \
)

int load_conf(const char *path);
struct trigger_t *get_trigger_by_name(char *name);
struct log_t *get_log_by_name(char *name);
int sender_id(const struct sender_t *sender);
struct sender_t *get_sender_by_name(char *name);
enum event_type_t get_conf_by_wd(int wd, void **private);
struct crash_t *get_crash_by_wd(int wd);
int crash_depth(struct crash_t *tcrash);

#endif
