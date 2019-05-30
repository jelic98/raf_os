#include <errno.h>
#include <fcntl.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// PROJEKAT
#include <crypt.h>

int file_read(struct m_inode * inode, struct file * filp, char * buf, int count)
{
	int left,chars,nr;
	struct buffer_head * bh;
	
	if ((left=count)<=0)
		return 0;
	while (left) {
		if ((nr = bmap(inode,(filp->f_pos)/BLOCK_SIZE))) {
			if (!(bh=bread(inode->i_dev,nr)))
				break;
		} else
			bh = NULL;
		nr = filp->f_pos % BLOCK_SIZE;
		chars = MIN( BLOCK_SIZE-nr , left );
		filp->f_pos += chars;
		left -= chars;
		if (bh) {
			char * p = nr + bh->b_data;

			// PROJEKAT
			char* pc = p;
			int len_start = chars;
			int inum = inode->i_num;
			char key[KEY_MAXLEN];
			
			keyget(key, keytype, 0);

			if(keyok(key) && isencr(inum)) {
				if(!keymatch(inum, key)) {
					return -EINVAL;
				}

				decr(pc, len_start, 0);
				chars--;
			}
	
			while (chars-->0)
				put_fs_byte(*(p++),buf++);
			
			if(keyok(key) && isencr(inum)) {
				encr(pc, len_start, 0);
			}

			brelse(bh);
		} else {
			while (chars-->0)
				put_fs_byte(0,buf++);
		}
	}
	inode->i_atime = CURRENT_TIME;
	return (count-left)?(count-left):-ERROR;
}

int file_write(struct m_inode * inode, struct file * filp, char * buf, int count)
{
	off_t pos;
	int block,c;
	struct buffer_head * bh;
	char * p;
	int i=0;

/*
 * ok, append may not work when many processes are writing at the same time
 * but so what. That way leads to madness anyway.
 */
	if (filp->f_flags & O_APPEND)
		pos = inode->i_size;
	else
		pos = filp->f_pos;
	while (i<count) {
		if (!(block = create_block(inode,pos/BLOCK_SIZE)))
			break;
		if (!(bh=bread(inode->i_dev,block)))
			break;
		c = pos % BLOCK_SIZE;
		p = c + bh->b_data;
		bh->b_dirt = 1;
		c = BLOCK_SIZE-c;
		if (c > count-i) c = count-i;
		pos += c;
		if (pos > inode->i_size) {
			inode->i_size = pos;
			inode->i_dirt = 1;
		}
		i += c;

		// PROJEKAT
		char* pc = bh->b_data;
		int len_start = pos - c - 1;
		int len_end = len_start;
		int inum = inode->i_num;
		char key[KEY_MAXLEN];
			
		keyget(key, keytype, 0);
		
		if(filp->f_flags & O_APPEND) {
			len_end = pos % BLOCK_SIZE;
		}

		if(keyok(gkey) && isencr(inum)) {
			if(!keymatch(inum, key)) {
				return -EINVAL;
			}

			decr(pc, len_start, 0);
			c--;
			p--;
		}

		while (c-->0)
			*(p++) = get_fs_byte(buf++);
	
		if(keyok(gkey) && isencr(inum)) {
			encr(pc, len_end, 0);
		}
		
		brelse(bh);
	}
	inode->i_mtime = CURRENT_TIME;
	if (!(filp->f_flags & O_APPEND)) {
		filp->f_pos = pos;
		inode->i_ctime = CURRENT_TIME;
	}
	return (i?i:-1);
}
