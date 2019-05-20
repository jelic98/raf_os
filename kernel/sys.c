#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>

int sys_ftime()
{
	return -ENOSYS;
}

int sys_mknod()
{
	return -ENOSYS;
}

int sys_break()
{
	return -ENOSYS;
}

int sys_mount()
{
	return -ENOSYS;
}

int sys_umount()
{
	return -ENOSYS;
}

int sys_ustat(int dev,struct ustat * ubuf)
{
	return -1;
}

int sys_ptrace()
{
	return -ENOSYS;
}

int sys_stty()
{
	return -ENOSYS;
}

int sys_gtty()
{
	return -ENOSYS;
}

int sys_rename()
{
	return -ENOSYS;
}

int sys_prof()
{
	return -ENOSYS;
}

int sys_setgid(int gid)
{
	if (current->euid && current->uid)
		if (current->gid==gid || current->sgid==gid)
			current->egid=gid;
		else
			return -EPERM;
	else
		current->gid=current->egid=gid;
	return 0;
}

int sys_acct()
{
	return -ENOSYS;
}

int sys_phys()
{
	return -ENOSYS;
}

int sys_lock()
{
	return -ENOSYS;
}

int sys_mpx()
{
	return -ENOSYS;
}

int sys_ulimit()
{
	return -ENOSYS;
}

int sys_time(long * tloc)
{
	int i;

	i = CURRENT_TIME;
	if (tloc) {
		verify_area(tloc,4);
		put_fs_long(i,(unsigned long *)tloc);
	}
	return i;
}

int sys_setuid(int uid)
{
	if (current->euid && current->uid)
		if (uid==current->uid || current->suid==current->uid)
			current->euid=uid;
		else
			return -EPERM;
	else
		current->euid=current->uid=uid;
	return 0;
}

int sys_stime(long * tptr)
{
	if (current->euid && current->uid)
		return -1;
	startup_time = get_fs_long((unsigned long *)tptr) - jiffies/HZ;
	return 0;
}

int sys_times(struct tms * tbuf)
{
	if (!tbuf)
		return jiffies;
	verify_area(tbuf,sizeof *tbuf);
	put_fs_long(current->utime,(unsigned long *)&tbuf->tms_utime);
	put_fs_long(current->stime,(unsigned long *)&tbuf->tms_stime);
	put_fs_long(current->cutime,(unsigned long *)&tbuf->tms_cutime);
	put_fs_long(current->cstime,(unsigned long *)&tbuf->tms_cstime);
	return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
	if (end_data_seg >= current->end_code &&
	    end_data_seg < current->start_stack - 16384)
		current->brk = end_data_seg;
	return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
	int i;

	if (!pid)
		pid = current->pid;
	if (!pgid)
		pgid = pid;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] && task[i]->pid==pid) {
			if (task[i]->leader)
				return -EPERM;
			if (task[i]->session != current->session)
				return -EPERM;
			task[i]->pgrp = pgid;
			return 0;
		}
	return -ESRCH;
}

int sys_getpgrp(void)
{
	return current->pgrp;
}

int sys_setsid(void)
{
	if (current->uid && current->euid)
		return -EPERM;
	if (current->leader)
		return -EPERM;
	current->leader = 1;
	current->session = current->pgrp = current->pid;
	current->tty = -1;
	return current->pgrp;
}

int sys_oldolduname(void* v)
{
	printk("calling obsolete system call oldolduname\n");
	return -ENOSYS;
//	return (0);
}

int sys_uname(struct utsname * name)
{
	static struct utsname thisname = {
		"linux 0.01-3.x","nodename","release ","3.x","i386"
	};
	int i;

	if (!name) return -1;
	verify_area(name,sizeof *name);
	for(i=0;i<sizeof *name;i++)
		put_fs_byte(((char *) &thisname)[i],i+(char *) name);
	return (0);
}

int sys_umask(int mask)
{
	int old = current->umask;

	current->umask = mask & 0777;
	return (old);
}

int sys_null(int nr)
{
	static int prev_nr=-2;
	if (nr==174 || nr==175) return -ENOSYS;

	if (prev_nr!=nr) 
	{
		prev_nr=nr;
//		printk("system call num %d not available\n",nr);
	}
	return -ENOSYS;
}

// PROJEKAT
#include <crypt.h>
#include <random.h>

struct m_inode* get_inode(int fd) {
	struct file * file = current->filp[fd];
	
	struct m_inode * inode;
	inode = file->f_inode;

	return inode;
}

int get_inum(int fd) {
	return get_inode(fd)->i_num;
}

int sys_keyset(const char* key, int length) {
	if(!keylenok(length)) {
		return -EKEYLEN;
	}

	int i;
	
	for(i = 0; i < length; i++) {
		gkey[i] = get_fs_byte(key + i);
	}

	return 0;
}

int sys_keyclear() {
	memset(gkey, 0, sizeof(gkey));
	
	return 0;
}

int sys_keygen(int level) {	
	int lvls[] = KEY_LVLS;
	int lvlslen = sizeof(lvls) / sizeof(int);
	int lvlok = 0;
	int i;

	for(i = 0; i < lvlslen; i++) {
		if(lvls[i] == level) {
			lvlok = 1;
			break;
		}
	}

	if(!lvlok) {
		return -EKEYLVL;
	}
	
	int len = keylen(level);
	char key[len + 1];
	char used[127] = {0};
	char c;

	for(i = 0; i < len; i++) {
		do {
			c = (rand() % (ASCII_LAST - ASCII_FIRST + 1)) + ASCII_FIRST;
		}while(used[c]);

		key[i] = c;
		used[c] = 1;
	}

	printk("%s\n", key);

	return 0;
}

int sys_encr(char* file, int length, int scall) {
	if(!keyok(gkey)) {
		return -EKEYNS;
	}

	length -= length < BLOCK_SIZE;		

	int i, j, k;

	char txtarr[length];
	char* txt;

	if(scall) {
		txt = txtarr;

		for(i = 0; i < length; i++) {
			txt[i] = get_fs_byte(file + i);
		}
	}else {
		txt = file;
	}

    int m = strlen(gkey);
    int n = length / m + (length % m != 0);

	char hed[m];
	strcpy(hed, gkey);

	char shed[m];
	strcpy(shed, gkey);
	
    char mat[n][m];
    char copmat[n][m];
	
	char tmp;
	
	for(k = 0; k < m - 1; k++) {
		for(j = 0; j < m - k - 1; j++) {
			if(shed[j] > shed[j + 1]) {
				tmp = shed[j];
				shed[j] = shed[j + 1];
				shed[j + 1] = tmp;
			}
		}	
	}
	
	for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            k = i * m + j;

			if(k < length && isascii(txt[k])) {
                mat[i][j] = txt[k];
            }else {
                mat[i][j] = PAD_CHR;
			}

			copmat[i][j] = mat[i][j];
        }
	}

	for(k = 0; k < m; k++) {
		for(j = 0; j < m; j++) {
			if(j != k && hed[k] == shed[j]) {
				for(i = 0; i < n; i++) {
					mat[i][j] = copmat[i][k];
				}
			}
		}
	}

	memset(copmat, 0, sizeof(copmat));

	char* cip = copmat;

	for(k = 0, j = 0; j < m; j++) {
		for(i = 0; i < n; i++) {
			cip[k++] = mat[i][j];
		}
	}
	
	for(i = 0; i < k; i++) {
		if(scall) {
			put_fs_byte(cip[i], file + i);
		}else {
			file[i] = cip[i];
		}
	}

	return 0;
}

int sys_decr(char* file, int length, int scall) {
	if(!keyok(gkey)) {
		return -EKEYNS;
	}

	length -= length < BLOCK_SIZE;		
	
	int i, j, k;
	
	char ciparr[length];
	char* cip;

	if(scall) {
		cip = ciparr;

		for(i = 0; i < length; i++) {
			cip[i] = get_fs_byte(file + i);
		}
	}else {
		cip = file;
	}

    int m = strlen(gkey);
    int n = length / m + (length % m != 0);

	char hed[m];
	strcpy(hed, gkey);

	char shed[m];
	strcpy(shed, gkey);
	
    char mat[n][m];
    char copmat[n][m];
	
	char tmp;

	for(k = 0; k < m - 1; k++) {
		for(j = 0; j < m - k - 1; j++) {
			if(shed[j] > shed[j + 1]) {
				tmp = shed[j];
				shed[j] = shed[j + 1];
				shed[j + 1] = tmp;
			}
		}	
	}
	
	for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            k = j * n + i;

			copmat[i][j] = mat[i][j] = cip[k];
        }
	}

	for(k = 0; k < m; k++) {
		for(j = 0; j < m; j++) {
			if(j != k && hed[k] == shed[j]) {
				for(i = 0; i < n; i++) {
					mat[i][k] = copmat[i][j];
				}
			}
		}
	}

	memset(copmat, 0, sizeof(copmat));

	char* txt = copmat;

	for(k = 0, i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
			if(mat[i][j] != PAD_CHR) {
                txt[k++] = mat[i][j];
			}
        }
	}

	for(i = 0; i < k; i++) {
		if(scall) {
			put_fs_byte(txt[i], file + i);
		}else {
			file[i] = txt[i];
		}
	}

	return 0;
}

int sys_encrlst(int fd, char* path, int length) {
	int i;

	int inum = get_inum(fd);

	for(i = 0; i < length; i++) {
		*(enclst + inum + i) = get_fs_byte(path + i);
	}
}

int sys_decrlst(int fd, char* path, int length) {
	*(enclst + get_inum(fd)) = 0;
}

int isencr(int inum) {
	return *(enclst + inum);
}

int sys_uisencr(int fd) {
	return isencr(get_inum(fd));
}

int sys_initenclst() {
	if(!enclst) {
		struct m_inode* inode = iget(0x301, 1);
		//int bnum = new_block(inode->i_dev);
		int bnum = 9810;
		struct buffer_head* bh = bread(inode->i_dev, bnum);
		enclst = bh->b_data;
		bh->b_dirt = 1;
		iput(inode);
	}
}
