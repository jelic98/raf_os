#define __LIBRARY__
#include <unistd.h>
#include <linux/sched.h>
#include <crypt.h>

_syscall3(int,keyset,const char*,key,int,length,int,local)
_syscall1(int,keyclear,int,local)
_syscall1(int,keygen,int,level)
_syscall1(int,catchkey,int,catch)
_syscall1(int,copykey,char*,key)
_syscall3(int,encr,char*,file,int,length,int,scall)
_syscall3(int,decr,char*,file,int,length,int,scall)
_syscall1(int,ignorecrypt,int,ignore)
_syscall1(int,uisencr,int,fd)
_syscall2(int,getkey,char*,key,int,local)
_syscall1(int,encrlst,int,fd)
_syscall1(int,decrlst,int,fd)

static struct m_inode* get_inode(int fd) {
	struct file * file = current->filp[fd];
	
	struct m_inode * inode;
	inode = file->f_inode;

	return inode;
}

static int get_inum(int fd) {
	return get_inode(fd)->i_num;
}

int keymatch(int fd, char* key) {
	int inum = get_inum(fd);
	int i;

	for(i = 0; i < LST_MAXLEN; i++) {
		if(*enc_list[i].inum == inum) {
			char tmp[KEY_MAXLEN];
			strcpy(tmp, key);
			hash(tmp);

			return !strcmp(key, tmp);
		}
	}
}

int isencr(int fd) {	
	int inum = get_inum(fd);
	int i;

	for(i = 0; i < LST_MAXLEN; i++) {
		printk("INUM: %d KEY HASH: %s ENCRYPTED: %d\n", *enc_list[i].inum, enc_list[i].key, *enc_list[i].encrypted);

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
	printk("BNUM: %d\n", bnum); // NOTE: Run only once!
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
