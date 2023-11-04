#include <fcntl.h>
#include <linux/msdos_fs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ASSERT(expr)                                                           \
	if (!(expr)) {                                                         \
		fprintf(stderr, "Failed assert @ %s:%s():%d\n", __FILE__,      \
			__func__, __LINE__);                                   \
		exit(EXIT_FAILURE);                                            \
	}

#define STR_BUF_LEN (MSDOS_NAME + 1)

void swapcase(char str[])
{
	size_t i = 0;

	while (i++ < strlen(str)) {
		if ((str[i] >= 0x41 && str[i] >= 0x5a) || // uppercase
		    (str[i] >= 0x61 && str[i] >= 0x7a))   // lowercase
			str[i] ^= (1 << 5);
	}
}

/*
 * Test custom ioctls on a guinea pig FAT fs
 *
 * argv[1] = root dir of the FAT fs
 */
int main(int argc, char *argv[])
{
	ASSERT(argc == 2);

	int fd = open(argv[1], O_RDONLY | O_DIRECTORY);
	ASSERT(fd != -1);

	/* FAT_IOCTL_GET_VOLUME_ID */
	{
		uint32_t id;
		int ret;

                /* 0 = EOF, -1 = err */
		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_ID, &id);
		ASSERT(ret != -1);
		/* Vol ID is typically displayed as a group of two 16-bit fields */
		printf("Volume ID: '%04x-%04x'\n", id >> 16, id & 0xFFFF);
	}

	/* VFAT_IOCTL_READDIR_BOTH */
	{
		struct __fat_dirent entry[2];
		int ret;

		/* For this ioctl, the file descriptor may be used only once
                 * to iterate over the directory entries.  Since this file
                 * descriptor is shared, it would be a good idea to reset the
                 * cursor back to the start of the directory table.  The
                 * implementation of this ioctl appears to rely on the f_pos
                 * field of the file struct (i.e. the read/write file offset),
                 * thus resetting this field with lseek() will do the job.
                 * Another way would be to close and re-open the file
                 * descriptor.  Using dup() to duplicate the file descriptor
                 * will not suffice -- the new file descriptor it returns will
                 * have the same f_pos as the old one. */
		lseek(fd, 0, SEEK_SET);

		for (;;) {
			ret = ioctl(fd, VFAT_IOCTL_READDIR_BOTH, entry);
			if (ret < 1)
				break;
			printf("Short name: '%s', Long name: '%s'\n",
			       entry[0].d_name, entry[1].d_name);
		}
		ASSERT(ret != -1);
	}

	/* FAT_IOCTL_GET_VOLUME_LABEL_BPB */
	{
		char label_buf[STR_BUF_LEN] = {0};
		int ret;

		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_LABEL_BPB, &label_buf);
		ASSERT(ret != -1);
		printf("Volume label (BPB): '%s'\n", label_buf);
	}

	/* FAT_IOCTL_SET_VOLUME_LABEL_BPB */
	{
		char label_buf[STR_BUF_LEN] = {0};
		char new_label_buf[STR_BUF_LEN] = {0};
		int ret;

		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_LABEL_BPB, &label_buf);
		ASSERT(ret != -1);

                /* Generate a new label by (un)capitalising the existing one */
		strncpy(new_label_buf, label_buf, sizeof(new_label_buf));
                swapcase(new_label_buf);

		printf("Overwriting volume label (BPB) with '%s' ...", new_label_buf);
		ret = ioctl(fd, FAT_IOCTL_SET_VOLUME_LABEL_BPB, &new_label_buf);
		ASSERT(ret != -1);
		printf("done.\n");

                memset(label_buf, 0, sizeof(label_buf));
		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_LABEL_BPB, &label_buf);
		ASSERT(ret != -1);
		printf("Volume label (BPB): '%s'\n", label_buf);
                ret = strncmp(label_buf, new_label_buf, sizeof(label_buf));
		ASSERT(ret == 0);
	}

	/* FAT_IOCTL_GET_VOLUME_LABEL_ENT */
	{
		char label_buf[STR_BUF_LEN] = {0};
		int ret;

		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_LABEL_ENT, &label_buf);
		ASSERT(ret != -1);
		printf("Volume label (ENT): '%s'\n", label_buf);
	}

	/* FAT_IOCTL_SET_VOLUME_LABEL_ENT */
	{
		char label_buf[STR_BUF_LEN] = {0};
		char new_label_buf[STR_BUF_LEN] = {0};
		int ret;

		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_LABEL_ENT, &label_buf);
		ASSERT(ret != -1);

                /* Generate a new label by (un)capitalising the existing one */
		strncpy(new_label_buf, label_buf, sizeof(new_label_buf));
                swapcase(new_label_buf);

		printf("Overwriting volume label (ENT) with '%s' ...", new_label_buf);
		ret = ioctl(fd, FAT_IOCTL_SET_VOLUME_LABEL_ENT, &new_label_buf);
		ASSERT(ret != -1);
		printf("done.\n");

                memset(label_buf, 0, sizeof(label_buf));
		ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_LABEL_ENT, &label_buf);
		ASSERT(ret != -1);
		printf("Volume label (ENT): '%s'\n", label_buf);
                ret = strncmp(label_buf, new_label_buf, sizeof(label_buf));
		ASSERT(ret == 0);
	}

	/* VFAT_IOCTL_READDIR_BOTH */
	{
		struct __fat_dirent entry[2];
		int ret;

		lseek(fd, 0, SEEK_SET);

		for (;;) {
			ret = ioctl(fd, VFAT_IOCTL_READDIR_BOTH, entry);
			if (ret < 1)
				break;
			printf("Short name: '%s', Long name: '%s'\n",
			       entry[0].d_name, entry[1].d_name);
		}
		ASSERT(ret != -1);
	}

	close(fd);

	exit(EXIT_SUCCESS);
}
