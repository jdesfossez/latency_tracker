/*
 * Copyright (C) 2014 Julien Desfossez <jdesfossez@efficios.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; only
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <linux/debugfs.h>
#include <linux/sched.h>

#include "tracker_debugfs.h"
#include "latency_tracker.h"
#include "tracker_private.h"
#include "wrapper/trace-clock.h"

#define DEBUGFSNAME "latency"

static struct dentry *debugfs_root;

int latency_tracker_debugfs_setup(void)
{
	debugfs_root = debugfs_create_dir(DEBUGFSNAME, NULL);
	if (!debugfs_root)
		goto error;

	return 0;

error:
	return -1;
}

void latency_tracker_debugfs_cleanup(void)
{
	debugfs_remove_recursive(debugfs_root);
}

int setup_default_entries(struct latency_tracker *tracker)
{
	struct dentry *dir;

	dir = debugfs_create_u64("threshold", S_IRUSR|S_IWUSR,
			tracker->debugfs_dir, &tracker->threshold);
	if (!dir)
		goto error;
	dir = debugfs_create_u64("timeout", S_IRUSR|S_IWUSR,
			tracker->debugfs_dir, &tracker->timeout);
	if (!dir)
		goto error;

	return 0;
error:
	return -1;
}

static
ssize_t read_wakeup_pipe(struct file *filp, char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct latency_tracker *tracker = filp->private_data;

	wait_event_interruptible(tracker->read_wait,
			tracker->got_alert);
	tracker->got_alert = false;

	return 0;
}

void latency_tracker_debugfs_wakeup_pipe(struct latency_tracker *tracker)
{
	uint64_t ts = trace_clock_read64();

	/* Rate limiter */
	if ((ts - tracker->last_wakeup_ts) < tracker->wakeup_rate_limit_ns)
		return;

	if (atomic_read(&tracker->wakeup_readers))
		irq_work_queue(&tracker->wake_irq);
	tracker->last_wakeup_ts = ts;
}

static
int open_wakeup_pipe(struct inode *inode, struct file *filp)
{
	struct latency_tracker *tracker = inode->i_private;
	filp->private_data = tracker;
	atomic_inc(&tracker->wakeup_readers);

	return 0;
}

static
int release_wakeup_pipe(struct inode *inode, struct file *filp)
{
	struct latency_tracker *tracker = filp->private_data;
	atomic_dec(&tracker->wakeup_readers);

	return 0;
}

static
const struct file_operations wakeup_pipe_fops = {
	.open           = open_wakeup_pipe,
	.release	= release_wakeup_pipe,
	.read           = read_wakeup_pipe,
	.llseek         = default_llseek,
	/* TODO: poll */
};

static
void irq_wake(struct irq_work *entry)
{
	struct latency_tracker *tracker = container_of(entry,
			struct latency_tracker, wake_irq);

	tracker->got_alert = true;
	wake_up_interruptible(&tracker->read_wait);
}

int latency_tracker_debugfs_setup_wakeup_pipe(struct latency_tracker *tracker)
{
	init_irq_work(&tracker->wake_irq, irq_wake);
	init_waitqueue_head(&tracker->read_wait);
	tracker->got_alert = false;
	/* FIXME: param */
	tracker->wakeup_rate_limit_ns = 1000000000;
	tracker->wakeup_pipe = debugfs_create_file("wakeup_pipe", S_IRUSR,
			tracker->debugfs_dir, tracker, &wakeup_pipe_fops);
	if (!tracker->wakeup_pipe)
		return -1;

	return 0;
}

static
void destroy_wakeup_pipe(struct latency_tracker *tracker)
{
	if (!tracker->wakeup_pipe)
		return;

	irq_work_sync(&tracker->wake_irq);
	debugfs_remove(tracker->wakeup_pipe);
	tracker->wakeup_pipe = NULL;
}

int latency_tracker_debugfs_add_tracker(
		struct latency_tracker *tracker)
{
	struct dentry *dir;
	int ret;

	dir = debugfs_create_dir(tracker->tracker_name, debugfs_root);
	if (!dir)
		goto error;
	tracker->debugfs_dir = dir;

	ret = setup_default_entries(tracker);
	if (ret != 0)
		goto error_cleanup;

	return 0;

error_cleanup:
	latency_tracker_debugfs_remove_tracker(tracker);

error:
	return -1;
}

void latency_tracker_debugfs_remove_tracker(struct latency_tracker *tracker)
{
	if (!tracker->debugfs_dir)
		return;
	destroy_wakeup_pipe(tracker);
	debugfs_remove_recursive(tracker->debugfs_dir);
}

struct dentry *latency_tracker_debugfs_add_subfolder(
		struct latency_tracker *tracker, const char *name)
{
	struct dentry *dir;

	if (!tracker->debugfs_dir)
		goto error;

	dir = debugfs_create_dir(name, tracker->debugfs_dir);
	if (!dir)
		goto error;

	return dir;

error:
	return NULL;
}

int latency_open_generic(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}