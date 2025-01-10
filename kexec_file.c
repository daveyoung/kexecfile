/* simple kexec_file_load utility */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <linux/kexec.h>

int debug = 0;

#define dbgprintf(...) \
do { \
    if (debug) \
        fprintf(stderr, __VA_ARGS__); \
} while(0)

#define MIN(a, b) ((a)) > ((b)) ? (b) : (a)

const char *kernel_name;
int kernel_fd;

const char *initrd_name;
int initrd_fd;

unsigned long cmdlen, flags;

#define MAX_CMDLINE_LEN 2048
char cmdline_buf[MAX_CMDLINE_LEN];

#define LINUX_REBOOT_KEXEC	0x45584543

static void perror_exit(const char *msg)
{
    if (errno) 
        perror(msg);
    else
        printf("Error: %s!\n", msg);

    exit(-1);
}


/* Initialize cmdline_buf[] */
static int get_current_cmdline(void)
{
	FILE *file;
	const char *proc_name = "/proc/cmdline";

	file = fopen(proc_name, "r");
	if (file == NULL)
		return -1;

	fgets(cmdline_buf, sizeof(cmdline_buf), file);
	fclose(file);

	if (strlen(cmdline_buf) > sizeof(cmdline_buf) - 10)
		return -1;

	cmdlen = strlen(cmdline_buf) + 1;

	return 0;
}

int load_file(const char *filename, int *fd)
{
	int ifd, ret;

    ifd = open(filename, O_RDONLY);
    if (ifd < 0)
		return -1;

	*fd = ifd;
	return 0;
}

void free_resources(void)
{
	close(kernel_fd);
	close(initrd_fd);
}

static inline long kexecf_load(int kfd, int ifd, unsigned long cmdlen, const char *cmdline, unsigned long flags)
{
	dbgprintf("Begin to call kexec_file_load syscall...\n");

	return (long) syscall(SYS_kexec_file_load, kfd, ifd, cmdlen, cmdline, flags);
}

static void usage(char *cmd)
{
	fprintf(stderr, "Usage: %s -l|-p <kernel> [-i <initrd>] [-c <cmdline>] [-d]\n", cmd);
	fprintf(stderr, "Usage: %s -u [-d]\n", cmd);
	fprintf(stderr, "Usage: %s -e [-d]\n", cmd);
}

int main(int argc, char *argv[])
{
	int has_cmdline = 0, reboot_exec = 0;
	int opt;

	while ((opt = getopt(argc, argv, "l:p:i:c:ude")) != -1) {
		switch (opt) {
		case 'l':
			kernel_name = optarg;
			break;
		case 'u':
			flags |= KEXEC_FILE_UNLOAD;
			goto l;
		case 'p':
			kernel_name = optarg;
			flags |= KEXEC_FILE_ON_CRASH;
			break;
		case 'i':
			initrd_name = optarg;
			break;
		case 'c':
			strncpy(cmdline_buf, optarg, MIN(sizeof(cmdline_buf), strlen(optarg)));
			has_cmdline = 1;
			break;
		case 'd':
			debug = 1;
			flags |= KEXEC_FILE_DEBUG;
			break;
		case 'e':
			reboot_exec = 1;
			goto r;
		default: /* '?' */
			usage(argv[0]);
			exit(-1);
		}
	}

r:
	if (reboot_exec) {
		reboot(LINUX_REBOOT_KEXEC);
		printf("kexec -e failed! Please load something beforehand.\n");
		return -1;
	}

	if (kernel_name == NULL) {
		usage(argv[0]);
		exit(-1);
	}

	/* Get /proc/cmdline as the default command line */
	if (!has_cmdline && get_current_cmdline() < 0)
		perror_exit("get_current_cmdline failed");

	printf("kernel: %s\ninitrd: %s\ncmdline: %s\n", kernel_name, initrd_name, cmdline_buf);


	if (load_file(kernel_name, &kernel_fd) < 0)
		perror_exit("load kernel image failed");

	if (initrd_name) {
		if (load_file(initrd_name, &initrd_fd) < 0)
			perror_exit("load initrd failed");
	} else
		flags |= KEXEC_FILE_NO_INITRAMFS;

l:
	/* Make kexec syscall */
	kexecf_load(kernel_fd, initrd_fd, cmdlen, cmdline_buf, flags);

	free_resources();

	return 0;
}
