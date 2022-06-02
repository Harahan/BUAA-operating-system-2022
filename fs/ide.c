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

int get_fault_disks_sum() {
    int i = 1, ans = 0;
    for (;i <= 5; i++) {
        if (raid4_valid(i) == 0) {
            ans++;
        }
    }
    return ans;
}

int raid4_write(u_int blockno, void *src) {
    char check1[BY2PG], check2[BY2PG];
    int i;
    user_bzero(check1, BY2PG);
    user_bzero(check2, BY2PG);

    int secn = 1;
    for (;secn <= 4; secn++) {
        if (raid4_valid(secn) != 0) {
            ide_write(secn, 2*blockno, src + BY2PG * (secn - 1) / 8, 1);
            ide_write(secn, 2*blockno+1, src + BY2PG * (secn + 3) / 8, 1);
        }
    }


    if (raid4_valid(5) != 0) {
        for (i=0; i<BY2PG/8; i++)
            check1[i] = ((char *)src)[i] ^ ((char *)(src + BY2PG/8))[i] ^ ((char *)(src + 2*BY2PG/8))[i] ^ ((char *)(src + 3*BY2PG/8))[i];
        ide_write(5, 2*blockno, check1, 1);
        for (i=0; i<BY2PG/8; i++)
            check2[i] = ((char *)(src + 4*BY2PG/8))[i] ^ ((char *)(src + 5*BY2PG/8))[i] ^ ((char *)(src + 6*BY2PG/8))[i] ^ ((char *)(src + 7*BY2PG/8))[i];
        ide_write(5, 2*blockno+1, check2, 1);
    }
    return get_fault_disks_sum();
}

int raid4_read(u_int blockno, void *dst) {
    char check1[BY2PG], check2[BY2PG];
    char raid_check1[BY2PG], raid_check2[BY2PG];
    int fault_disk = 0;
    int i, j, k;
    user_bzero(check1, BY2PG);
    user_bzero(check2, BY2PG);
    user_bzero(raid_check1, BY2PG);
    user_bzero(raid_check2, BY2PG);

    fault_disk = get_fault_disks_sum();
    if (fault_disk > 1) return fault_disk;

    int secn = 1;
    for (;secn <= 4; secn++) {
        if (raid4_valid(secn) != 0) {
            ide_read(secn, 2*blockno, dst + BY2PG * (secn - 1) / 8, 1);
            ide_read(secn, 2*blockno+1, dst + BY2PG * (secn + 3) / 8, 1);
        }
    }

    if (raid4_valid(5) == 0) return 1;
    if (fault_disk == 0) {
        ide_read(5, 2*blockno, raid_check1, 1);
        ide_read(5, 2*blockno+1, raid_check2, 1);
        int ok = 1;
        for (i=0; i<BY2PG/8; i++) {
            check1[i] = ((char *)dst)[i] ^ ((char *)(dst + BY2PG/8))[i] ^ ((char *)(dst + 2*BY2PG/8))[i] ^ ((char *)(dst + 3*BY2PG/8))[i];
            if (raid_check1[i] != check1[i]) ok = 0;
        }
        for (i=0; i<BY2PG/8; i++) {
            check2[i] = ((char *)(dst + 4*BY2PG/8))[i] ^ ((char *)(dst + 5*BY2PG/8))[i] ^ ((char *)(dst + 6*BY2PG/8))[i] ^ ((char *)(dst + 7*BY2PG/8))[i];
            if (raid_check2[i] != check2[i]) ok = 0;
        }

        if (ok == 1) return 0;
        else return -1;
    }

    for (k = 1; k <= 4; k++) {
        if (raid4_valid(k) == 0) {
            for (i=0; i<BY2PG/8; i++) {
                char restore_data1, restore_data2;
                int j = 1;
                for (j = 1; j <= 4; j++) if (k != j) {
                        restore_data1 ^= ((char *)(dst + (j-1) * BY2PG/8))[i];
                        restore_data2 ^= ((char *)(dst + (j+3) * BY2PG/8))[i];
                    }
                ((char *)(dst + (k-1) * BY2PG/8))[i] = restore_data1;
                ((char *)(dst + (k+3) * BY2PG/8))[i] = restore_data2;
            }
        }
    }
    return 1;
}