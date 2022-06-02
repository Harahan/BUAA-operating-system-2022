/*
 * operations on IDE disk.
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>

// Overview:
// 	read data from IDE disk. First issue a read request through
// 	disk register and then copy data from disk buffer
// 	(512 bytes, a sector) to destination array.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurrs during the read of the IDE disk, panic.
//
// Hint: use syscalls to access device registers and buffers
/*** exercise 5.2 ***/
void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
    int offset_now = offset_begin;
    int op_status = 0;
    int read = 0;

	while (offset_begin + offset < offset_end) {
		// Your code here
		// error occurred, then panic.
        offset_now = offset_begin + offset;
        // set id
        if (syscall_write_dev(&diskno, 0x13000010, 4) != 0) user_panic("write_failed!\n");
        // set offset
        if (syscall_write_dev(&offset_now, 0x13000000, 4) != 0) user_panic("write_failed!\n");
        // start read
        if (syscall_write_dev(&read, 0x13000020, 4) != 0) user_panic("write_failed!\n");
        // get status
        if (syscall_read_dev(&op_status, 0x13000030, 4) != 0) user_panic("read_failed!\n");
        if (op_status == 0) user_panic("can't read!\n");
        // get data
        if (syscall_read_dev(dst + offset, 0x13004000, 0x200) != 0) user_panic("read_failed!\n");
        offset += 0x200;

	}
}


// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurrs during the read of the IDE disk, panic.
//
// Hint: use syscalls to access device registers and buffers
/*** exercise 5.2 ***/
void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	// Your code here
	// int offset_begin = ;
	// int offset_end = ;
	// int offset = ;

	// DO NOT DELETE WRITEF !!!
	writef("diskno: %d\n", diskno);

	// while ( < ) {
		// copy data from source array to disk buffer.

		// if error occur, then panic.
	// }
    int offset_begin = secno * 0x200;
    int offset_end = offset_begin + nsecs * 0x200;
    int offset = 0;
    int offset_now = offset_begin;
    int op_status = 0;
    int write = 1;

    while (offset_begin + offset < offset_end) {
        offset_now = offset_begin + offset;
        // set id
        if (syscall_write_dev(&diskno, 0x13000010, 4) != 0) user_panic("write_failed!\n");
        // set offset
        if (syscall_write_dev(&offset_now, 0x13000000, 4) != 0) user_panic("write_failed!\n");
        // set data
        if (syscall_write_dev(src + offset, 0x13004000, 0x200) != 0) user_panic("write_failed!\n");
        // start write
        if (syscall_write_dev(&write, 0x13000020, 4) != 0) user_panic("write_failed!\n");
        // get status
        if (syscall_read_dev(&op_status, 0x13000030, 4) != 0) user_panic("read_failed!\n");
        if (op_status == 0) user_panic("can't write!\n");
        offset += 0x200;

    }
}

int check() {
    int i = 1, ans = 0;
    for (; i <= 5; i++) {
        if (raid4_valid(i) == 0) {
            ans++;
        }
    }
    return ans;
}
int get_error_dis() {
    int i = 1;
    for (; i <= 5; i++) {
        if (!raid4_valid(i)) {
            return i;
        }
    }
    return -1;
}

int arr[128], buf[128];
void get_code(void* dst) {
    int k = 0;
    user_bzero(arr, sizeof(arr));
    for (; k < BY2PG / 2; k += 0x200) {
        int addr = dst + k, i = 0;
        for (; addr < dst + k + 0x200; addr += 4, i++) {
            arr[i] = (*((int*)addr)) ^ arr[i];
        }
    }
}

int check_code(int start_secno) {
    user_bzero(buf, sizeof(buf));
    ide_read(5, start_secno, buf, 1);
    int t = 0;
    for (; t < 128; t++) {
        if (arr[t] != buf[t]) return -1;
    }
    return 0;
}


int raid4_valid(u_int diskno) {
    // 0x200: the size of a sector: 512 bytes.
    int offset = 0;
    int op_status1 = 0;
    int read = 0;

    // set id
    if (syscall_write_dev(&diskno, 0x13000010, 4) != 0) user_panic("write_failed!\n");
    // set offset
    if (syscall_write_dev(&offset, 0x13000000, 4) != 0) user_panic("write_failed!\n");

    // start read
    if (syscall_write_dev(&read, 0x13000020, 4) != 0) user_panic("write_failed!\n");
    // get status
    if (syscall_read_dev(&op_status1, 0x13000030, 4) != 0) user_panic("read_failed!\n");
    if (op_status1 == 0) return 0;
    return 1;

}

int raid4_write(u_int blockno, void *src) {
    int start_secno = blockno * 2, end_secno = blockno * 2 + 1, i = 0;
    while (start_secno + i <= end_secno) {
        int j = 1;
        for (; j <= 4; j++) {
            if (raid4_valid(j)) ide_write(j, start_secno + i, src + i * BY2PG / 2 + 0x200 * (j - 1), 1);
        }
        i++;
    }
    get_code(src);
    if (raid4_valid(5)) ide_write(5, start_secno, arr, 1);
    get_code(src);
    if (raid4_valid(5)) ide_write(5, start_secno + 1, arr, 1);
    return check();
}

int raid4_read(u_int blockno, void *dst) {
    int start_secno = blockno * 2, end_secno = blockno * 2 + 1;
    int ch = check(), error;
    if (ch > 1) return ch;
    if (ch == 1) {

    } else {
        int i = 0;
        while (start_secno + i <= end_secno) {
            int j = 1;
            for (; j <= 4; j++) {
                ide_read(j, start_secno + i, dst + i * BY2PG / 2 + 0x200 * (j - 1), 1);
            }
            i++;
        }
        get_code(dst);
        if (check_code(start_secno) == -1) return -1;
        get_code(dst + BY2PG / 2);
        return check_code(start_secno + 1);
    }
}