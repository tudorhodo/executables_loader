/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "exec_parser.h"

static so_exec_t *exec;
static int page_size;
static struct sigaction old_action;
static int fd;

/*
 * Handler for SIGSEGV signal
 * For SIGSEGV signal, checks if memory zone is mapped
 * If no, reserves memory for this segment
 * Otherwise lets default handler do the job
 */
static void segv_handler(int signum, siginfo_t *info, void *context)
{
	/* Checks if signal is SIGSEGV */
	if (signum != SIGSEGV) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	int addr = (int)info->si_addr;
	struct so_seg *segment = NULL;

	/*
	 * Search for addres in executable's memory segments
	 * If found, checks if segment is mapped
	 */
	for (int i = 0; i < exec->segments_no; i++) {
		if (addr >= exec->segments[i].vaddr &&
			addr < exec->segments[i].vaddr + exec->segments[i].mem_size) {
			segment = &exec->segments[i];

			int page_num = (addr - segment->vaddr) / page_size;

			char *arrayed_data = (char *)segment->data;

			if (arrayed_data[page_num] == 1) {
				old_action.sa_sigaction(signum, info, context);
				return;
			}

			/*
			 * If segment isn't reserved, start reserving memory
			 * Map a page at the address which caused the signal
			 */
			char *verificare = mmap((void *)segment->vaddr + page_num * page_size,
				page_size, PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
				-1, 0);

			if (verificare == MAP_FAILED)
				return;

			/*
			 * If error came from somewhere where file's data should
			 * be mapped, copy the content from file
			 */
			if (segment->file_size > page_num * page_size) {
				int size;

				if (segment->file_size > (page_num + 1) * page_size)
					size = (page_num + 1) * page_size;
				else
					size = segment->file_size;

				size -= page_num * page_size;

				int ret = lseek(fd, segment->offset + page_num * page_size, SEEK_SET);

				if (ret < 0)
					return;

				ret = read(fd, verificare, size);
				if (ret < 0)
					return;
			}

			/* Set permissions as defined by segment */
			mprotect(verificare, page_size, segment->perm);

			/* Mark page as mapped */
			arrayed_data[page_num] = 1;
			return;
		}
	}

	/* If zone not found, use default handler */
	old_action.sa_sigaction(signum, info, context);
}


int so_init_loader(void)
{
	/* Get size of page */
	page_size = getpagesize();

	/* Set new handler for SEGSIGV signal */
	struct sigaction action;
	int rc;

	action.sa_sigaction = segv_handler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGSEGV);
	action.sa_flags = SA_SIGINFO;

	rc = sigaction(SIGSEGV, &action, &old_action);
	if (rc == -1)
		return -1;

	return 0;
}


int number_of_pages(so_seg_t seg) {

	int num_page = seg.mem_size / page_size;

	if (seg.mem_size % page_size)
		num_page++;

	return num_page;
}


int so_execute(char *path, char *argv[])
{
	/* Open file descriptr to file for future operations */
	fd = open(path, O_RDONLY, 0644);
	if (fd == -1)
		return -1;

	exec = so_parse_exec(path);

	if (!exec)
		return -1;

	/* Initialize segment */
	for (int i = 0; i < exec->segments_no; i++) {
		/* Set number of pages */
		int page_num = number_of_pages(exec->segments[i]);

		/* Allocate memory for data */
		exec->segments[i].data = malloc(page_num * sizeof(char));

		if (exec->segments[i].data == NULL)
			return -1;

		memset(exec->segments[i].data, 0, page_num);
	}

	so_start_exec(exec, argv);

	return 0;
}
