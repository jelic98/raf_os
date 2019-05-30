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
#include <hash.h>

struct m_inode* get_inode(int fd) {
	struct file * file = current->filp[fd];
	
	struct m_inode * inode;
	inode = file->f_inode;

	return inode;
}

int get_inum(int fd) {
	return get_inode(fd)->i_num;
}

int isencr(int inum) {	
	int i;

	for(i = 0; i < LST_MAXLEN; i++) {
		//printk("SEARCH:\t%d\tCURRENT:\t%d\tKEY:\t%s\tENCRYPT: %d\n", inum, *enc_list[i].inum, enc_list[i].key, *enc_list[i].encrypted);

		if(*enc_list[i].inum == inum) {
			return *enc_list[i].encrypted;
		}
	}

	return 0;
}

void init_enclst() {
	static int inited = 0;

	if(inited++) {
		return;
	}

	struct m_inode* inode = iget(0x301, 1);
	int bnum = new_block(inode->i_dev); // NOTE: Run only once!
	//int bnum = 9810;
	//printk("BNUM: %d\n", bnum); // NOTE: Run only once!
	struct buffer_head* bh = bread(inode->i_dev, bnum);
	memset(bh->b_data, 0, sizeof(bh->b_data)); // NOTE: Run only once!
	
	int i;

	for(i = 0; i < LST_MAXLEN; i++) {
		enc_list[i].inum = bh->b_data + i * ENT_MAXLEN;
		enc_list[i].encrypted = bh->b_data + i * ENT_MAXLEN + 1;
		enc_list[i].key = bh->b_data + i * ENT_MAXLEN + 2;
	}

	bh->b_dirt = 1;
	iput(inode);
}

void edit_enclst(int fd, int encrypt) {
	int inum = get_inum(fd);
	int i;

	for(i = 0; i < LST_MAXLEN; i++) {
		if(!*enc_list[i].inum && encrypt) {
			*enc_list[i].inum = inum;
			*enc_list[i].encrypted = encrypt;
			strcpy(enc_list[i].key, gkey);
			hash(enc_list[i].key);
			break;
		}

		if(*enc_list[i].inum == inum) {
			*enc_list[i].encrypted = encrypt;
			break;
		}
	}
}

int sys_keyset(const char* key, int length, int local) {
	if(!keylenok(length)) {
		return -EKEYLEN;
	}
	
	int i;

	if(local) {
		char lockey[KEY_MAXLEN];

		for(i = 0; i < length; i++) {
			lockey[i] = get_fs_byte(key + i);
		}
		
		strcpy(current->local_key, lockey);

		current->local_timeout = jiffies + LOCAL_TIMEOUT;
	}else {
		for(i = 0; i < length; i++) {
			gkey[i] = get_fs_byte(key + i);
		}
	
		global_timeout = jiffies + GLOBAL_TIMEOUT;
	}

	init_enclst();	

	return 0;
}

int sys_keyclear(int local) {
	if(local) {
		memset(current->local_key, 0, sizeof(current->local_key));
		current->local_timeout = 0;
	}else {
		memset(gkey, 0, sizeof(gkey));
		global_timeout = 0;
	}
	
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

int sys_keycatch(int catch) {
	catchkey = catch;
	return 0;
}

int sys_keycopy(char* key) {
	int i;

	for(i = 0; i < tmpindex; i++) {
		put_fs_byte(tmpkey[i], key + i);
	}

	memset(tmpkey, 0, sizeof(tmpkey));

	tmpindex = 0;

	return 0;
}

int sys_keyget(char* key, int local, int scall) {
	int i;
	int length;

	if(local) {
		length = strlen(current->local_key);

		for(i = 0; i < length; i++) {
			if(scall) {
				put_fs_byte(current->local_key[i], key + i);
			}else {
				key[i] = current->local_key[i];
			}
		}
	}else {
		length = strlen(gkey);
		
		for(i = 0; i < length; i++) {
			if(scall) {
				put_fs_byte(gkey[i], key + i);
			}else {
				key[i] = gkey[i];
			}
		}
	}

	if(scall) {
		put_fs_byte(0, key + i);
	}else {
		key[i] = 0;
	}
	
	return 0;
}

int sys_encr(int fd) {
	if(!keyok(gkey)) {
		return -EKEYNS;
	}

	edit_enclst(fd, 1);
	
	return 0;
}

int sys_decr(int fd) {
	if(!keyok(gkey)) {
		return -EKEYNS;
	}

	edit_enclst(fd, 0);

	return 0;
}
