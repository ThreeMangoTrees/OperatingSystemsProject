#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define BLOCK_SIZE 1024
#define inode_size 64
#define LAST(k,n) ((k) & ((1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))

typedef struct  {
    unsigned short isize;
    unsigned int fsize;
    unsigned int nfree;
    unsigned int free[200];
    unsigned short ninode;
    unsigned short inode[100];
    char flock;
    char ilock;
    char fmode;
    unsigned short time[2];
} super_block_struct;

typedef struct {
    unsigned short flags;
    char nlinks;
    char uid;
    char gid;
    unsigned short size0;
    unsigned short size1;
    unsigned int addr[11];
    unsigned short actime[2];
    unsigned short modtime[2];
} inode_struct;

typedef struct {
    unsigned short inode_offset;
    char file_name[14];
} file_struct;

typedef struct  {
    unsigned int nfree;
    unsigned int free[200];
} free_blocks_struct;

super_block_struct read_super_block(FILE* file_system);
int add_file_to_dir(const char* to_filename,FILE* file_system);
int get_inode_by_file_name(const char* to_filename,FILE* file_system);
inode_struct read_inode(int to_file_inode_num, FILE* file_system);
inode_struct init_file_inode(int to_file_inode_num, unsigned int file_size, FILE* file_system);
int write_inode(int inode_num, inode_struct inode, FILE* file_system);
int write_super_block(super_block_struct block1,FILE* file_system);
void add_free_block(int block_num, FILE* file_system);
int get_free_block(FILE* file_system);
void copy_data_blocks(unsigned int *from_array, unsigned int *to_array, int buf_len);
void add_block_to_inode(int block_order_num, int block_num, int to_file_inode_num, FILE* file_system);
unsigned int get_block_for_big_file(int file_node_num,int block_number_order,FILE* file_system);
unsigned int get_inode_file_size(int to_file_inode_num, FILE* file_system);
void add_block_to_free_list(int next_block_number, FILE* file_system);
void remove_file_from_directory(int file_node_num, FILE*  file_system);
void add_block_to_inode_small_file(int block_order_num, int block_num, int to_file_inode_num, FILE* file_system);
unsigned int get_block_for_small_file(int file_node_num,int block_number_order, FILE* file_system);
int initialize_filesystem(int num_blocks, int num_inodes, FILE* file_system);
int cpin(const char* from_filename, const char* to_filename,FILE* file_system);
int cpout(const char* from_filename, const char* to_filename, FILE* file_system);
int Rm(const char* filename,FILE* file_system);
int make_directory(const char* filename,FILE* file_system);

inode_struct init_file_inode(int to_file_inode_num, unsigned int file_size, FILE* file_system){
    inode_struct file_inode;
    unsigned int bit0_15;
    unsigned int bit16_31;
    unsigned int bit32;

    bit0_15 = LAST(file_size,16);
    bit16_31 = MID(file_size,16,32);
    bit32 = MID(file_size,32,33);

    file_inode.flags=0;       
    file_inode.flags |=1 <<15; 
    file_inode.flags |=0 <<14; 
    file_inode.flags |=0 <<13;

    if (bit32 == 1){                  
        file_inode.flags |=1 <<0;
    } else{
        file_inode.flags |=0 <<0;
    }

    if (file_size<=7*BLOCK_SIZE){
        file_inode.flags |=0 <<12; 
    } else {
        file_inode.flags |=1 <<12; 
    }

    file_inode.nlinks=0;
    file_inode.uid=0;
    file_inode.gid=0;
    file_inode.size0=bit16_31; 
    file_inode.size1=bit0_15; 
    file_inode.addr[0]=0;
    file_inode.addr[1]=0;
    file_inode.addr[2]=0;
    file_inode.addr[3]=0;
    file_inode.addr[4]=0;
    file_inode.addr[5]=0;
    file_inode.addr[6]=0;
    file_inode.addr[7]=0;
    file_inode.addr[8]=0;
    file_inode.addr[9]=0;
    file_inode.addr[10]=0;
    file_inode.actime[0]=0;
    file_inode.actime[1]=0;
    file_inode.modtime[0]=0;	
    file_inode.modtime[1]=0;
    return file_inode;
}

inode_struct read_inode(int to_file_inode_num, FILE* file_system){
    inode_struct file_inode;
    fseek(file_system, (BLOCK_SIZE*2+inode_size*(to_file_inode_num-1)), SEEK_SET); 
    fread(&file_inode,inode_size,1,file_system);
    return file_inode;
}

int write_inode(int inode_num, inode_struct inode, FILE* file_system){
    fseek(file_system, (BLOCK_SIZE*2+inode_size*(inode_num-1)), SEEK_SET); 
    fwrite(&inode, inode_size, 1, file_system);
    return 0;
}

super_block_struct read_super_block(FILE* file_system){
    super_block_struct block1;
    fseek(file_system,BLOCK_SIZE,SEEK_SET); 
    fread(&block1,sizeof(block1),1,file_system);
    return block1;
}

int write_super_block(super_block_struct block1,FILE* file_system){
    fseek(file_system,BLOCK_SIZE,SEEK_SET); 
    fwrite(&block1,sizeof(block1),1,file_system);
    return 0;
}

void copy_data_blocks(unsigned int *from_array, unsigned int *to_array, int buf_len){
    int i;
    for (i = 0;i < buf_len; i++){
        to_array[i] = from_array[i];
    }
}

void add_free_block(int block_num, FILE* file_system){
    super_block_struct block1;
    free_blocks_struct copy_to_block;
    fseek(file_system, BLOCK_SIZE, SEEK_SET);
    fread(&block1,sizeof(block1),1,file_system);
    if (block1.nfree == 200){ 
        copy_to_block.nfree = 200;
        copy_data_blocks(block1.free, copy_to_block.free, 200);
        fseek(file_system,block_num*BLOCK_SIZE,SEEK_SET);
        fwrite(&copy_to_block,sizeof(copy_to_block),1,file_system);
        block1.free[0] = block_num;
        block1.nfree = 1;
    } else {
        block1.free[block1.nfree] = block_num;
        block1.nfree++;	
    }
    fseek(file_system, BLOCK_SIZE, SEEK_SET);
    fwrite(&block1,sizeof(block1),1,file_system);
}

unsigned int get_block_for_big_file(int file_node_num,int block_number_order, FILE* file_system){
    inode_struct file_inode;
    unsigned int block_num_to;
    unsigned int sec_ind_block;

    file_inode = read_inode(file_node_num,file_system);
    unsigned int logical_block = block_number_order/256;
    unsigned int word_in_block = block_number_order % 256;
    if (logical_block < 10){
        fseek(file_system, file_inode.addr[logical_block]*BLOCK_SIZE+word_in_block*4, SEEK_SET);
        fread(&block_num_to,sizeof(block_num_to),1,file_system);
    } else {
        fseek(file_system, file_inode.addr[10]*BLOCK_SIZE+(logical_block-10)*4, SEEK_SET);
        fread(&sec_ind_block,sizeof(sec_ind_block),1,file_system);
        fseek(file_system, sec_ind_block*BLOCK_SIZE+word_in_block*4, SEEK_SET);
        fread(&block_num_to,sizeof(block_num_to),1,file_system);
    }
    return block_num_to;
}

unsigned int get_block_for_small_file(int file_node_num,int block_number_order, FILE* file_system){
    inode_struct file_inode;
    file_inode = read_inode(file_node_num,file_system);
    return (file_inode.addr[block_number_order]);
}

unsigned int get_inode_file_size(int to_file_inode_num, FILE* file_system){
    inode_struct to_file_inode;
    unsigned int file_size;
    unsigned int bit0_15;
    unsigned int bit16_31;
    unsigned int bit32;

    to_file_inode = read_inode(to_file_inode_num, file_system);

    bit32 = LAST(to_file_inode.flags, 1);
    bit16_31 = to_file_inode.size0;
    bit0_15 = to_file_inode.size1;

    file_size = (bit32<<32) | ( bit16_31 << 16) | bit0_15;
    return file_size;
}

int initialize_filesystem(int num_blocks, int num_inodes, FILE* file_system) {	
    inode_struct i_node;
    char buff[BLOCK_SIZE];
    super_block_struct block1;
    file_struct directory_entry;
    memset(buff,0,BLOCK_SIZE);
    printf("\nInitialize file_system with %i number of blocks and %i number of i-nodes,size of i-node %i Superblock %i\n",num_blocks,num_inodes,sizeof(i_node), sizeof(block1));
    int written=0;
    int i;
    rewind(file_system);
    for (i=0;i<num_blocks;i++){
        written += fwrite(buff,1,BLOCK_SIZE,file_system);
    }
    printf("\nBytes (%i) written, block size %i, buffer:%s", written,BLOCK_SIZE,buff);

    block1.isize=0;
    block1.fsize=0;
    block1.nfree=1;
    block1.free[0] = 0;
    fseek(file_system,BLOCK_SIZE,SEEK_SET);
    fwrite(&block1,BLOCK_SIZE,1,file_system);
    int number_inode_blocks = ceil(num_inodes/64); 
    int start_free_block=2+number_inode_blocks+1; 
    int index;

    for (index=start_free_block;index<num_blocks; index++ ){
        add_free_block(index, file_system);
    }

    fseek(file_system,BLOCK_SIZE,SEEK_SET);
    fread(&block1,sizeof(block1),1,file_system);
    block1.ninode = 99;
    int next_free_inode=1; 
    for (i = 0; i <= 99; i++){
        block1.inode[i]=next_free_inode;
        next_free_inode++;
    }
    fseek(file_system,BLOCK_SIZE,SEEK_SET);
    fwrite(&block1,BLOCK_SIZE,1,file_system);

    inode_struct first_inode;
    first_inode.flags=0;       
    first_inode.flags |=1 <<15; 
    first_inode.flags |=1 <<14; 
    first_inode.flags |=0 <<13;
    first_inode.nlinks=0;
    first_inode.uid=0;
    first_inode.gid=0;
    first_inode.size0=0;
    first_inode.size1=16*2;
    first_inode.addr[0]=0;
    first_inode.addr[1]=0;
    first_inode.addr[2]=0;
    first_inode.addr[3]=0;
    first_inode.addr[4]=0;
    first_inode.addr[5]=0;
    first_inode.addr[6]=0;
    first_inode.addr[7]=0;
    first_inode.addr[8]=0;
    first_inode.addr[9]=0;
    first_inode.addr[10]=0;
    first_inode.actime[0]=0;
    first_inode.actime[1]=0;
    first_inode.modtime[0]=0;	
    first_inode.modtime[1]=0;	

    directory_entry.inode_offset = 1; 
    strcpy(directory_entry.file_name, ".");
    int dir_file_block = start_free_block-1;      
    fseek(file_system,dir_file_block*BLOCK_SIZE,SEEK_SET); 
    fwrite(&directory_entry,16,1,file_system);
    strcpy(directory_entry.file_name, "..");
    fwrite(&directory_entry,16,1,file_system);
    printf("\nDirectory in the block %i",dir_file_block);
    first_inode.addr[0]=dir_file_block;
    write_inode(1,first_inode,file_system);

    return 0;
}

int cpin(const char* from_filename,const char* to_filename,FILE* file_system){
    printf("\nInside cpin, copy from %s to %s \n",from_filename, to_filename);
    inode_struct to_file_inode;
    int to_file_inode_num, status_ind, num_bytes_read, free_block_num, file_node_num;
    unsigned long file_size;
    FILE* from_file_fd;
    unsigned char read_buffer[BLOCK_SIZE];

    file_node_num = get_inode_by_file_name(to_filename, file_system);
    if (file_node_num != -1){
        printf("\nFile %s already exists. Choose different name",to_filename);
        return -1;
    }

    if( access(from_filename, F_OK) != -1 ) {
        printf("\nCopy From File %s exists. Trying to open...\n",from_filename);
        from_file_fd = fopen(from_filename, "rb");
        fseek(from_file_fd, 0L, SEEK_END); 
        file_size = ftell(from_file_fd);
       
        if (file_size == 0){
            printf("\nCopy from File %s doesn't exists. Type correct file name\n",from_filename);
            return -1;
        } else {
            printf("\nCopy from File size is %i\n",file_size);
            rewind(file_system);
        }
    } else {
        printf("\nCopy from File %s doesn't exists. Type correct file name\n",from_filename);
        return -1;
    }
    to_file_inode_num = add_file_to_dir(to_filename, file_system);
    to_file_inode = init_file_inode(to_file_inode_num, file_size, file_system);
    status_ind = write_inode(to_file_inode_num, to_file_inode, file_system);

    int num_blocks_read = 1;
    int total_num_blocks = 0;
    fseek(from_file_fd, 0L, SEEK_SET);	
    int block_order = 0;
    while(num_blocks_read == 1){
        
        num_blocks_read = fread(&read_buffer, BLOCK_SIZE, 1, from_file_fd);
        total_num_blocks += num_blocks_read;
        free_block_num = get_free_block(file_system);
        
        if (free_block_num == -1){
            printf("\nNo free blocks left. Total blocks read so far:%i",total_num_blocks);
            return -1;
        }
        if (file_size > BLOCK_SIZE*11) {      
            add_block_to_inode(block_order, free_block_num, to_file_inode_num, file_system);	
        } // *** Vinit :: if greater means large file with 9 single indirect, 1 double indirect, 1 triple indirect.
        else {
            add_block_to_inode_small_file(block_order,free_block_num, to_file_inode_num, file_system);
        } // *** Vinit :: if less means small file with direct block addressing. 
        
        fseek(file_system, free_block_num*BLOCK_SIZE, SEEK_SET);
        fwrite(&read_buffer, sizeof(read_buffer), 1, file_system);
        block_order++;
    }
    printf("cpin completed\n");
    return 0;
}

int Rm(const char* filename,FILE* file_system){
    int file_node_num, file_size, block_number_order, next_block_number; 
    inode_struct file_inode, tmp_node;
    unsigned char bit_14; 
    file_node_num = get_inode_by_file_name(filename,file_system);
    if (file_node_num ==-1){
        printf("\nFile %s not found",filename);
        return -1;
    }
    file_inode = read_inode(file_node_num, file_system);
    bit_14 = MID(file_inode.flags,14,15);
    if (bit_14 == 0){ 
        file_size = get_inode_file_size(file_node_num, file_system);
        block_number_order = file_size/BLOCK_SIZE;
        if (file_size%BLOCK_SIZE !=0)
            block_number_order++;  

        block_number_order--; 
        while(block_number_order>0){
            if (file_size > BLOCK_SIZE*8) 
                next_block_number = get_block_for_big_file(file_node_num,block_number_order,file_system);
            else
                next_block_number = file_inode.addr[block_number_order];

            add_block_to_free_list(next_block_number, file_system);
            block_number_order--;
        }
    }
    file_inode.flags=0; 
    write_inode(file_node_num, file_inode, file_system);
    remove_file_from_directory(file_node_num, file_system);
    fflush(file_system);
    return 0;
}

void add_block_to_inode(int block_order_num, int block_num, int to_file_inode_num, FILE* file_system) {  
    inode_struct file_inode, tmp_node;
    
    int logical_block, prev_logical_block;
    int logical_block_t, prev_logical_block_t;
    int word_in_block, free_block_num;
    int word_in_sec, second_block_num, word_in_third;
    
    unsigned int block_num_to = block_num;
    unsigned int sec_ind_block, triple_ind_block = 0;
    
    file_inode = read_inode(to_file_inode_num, file_system);
    
    logical_block = block_order_num/256;
    prev_logical_block = (block_order_num - 1)/256;

    //logical_block_t = logical_block/256;
    //prev_logical_block_t = (logical_block-1)/256;
    
    logical_block_t = (block_order_num)/(256*256);
    prev_logical_block_t = (block_order_num - 1)/(256*256);

    printf("Initial logical_block = %d, block_order_num = %d \n", logical_block, block_order_num);

    word_in_block = block_order_num % 256;
    
    if (word_in_block == 0 && logical_block <= 9){ 
        free_block_num = get_free_block(file_system);
        file_inode.addr[logical_block] = free_block_num;
    }

    if (logical_block < 9){ 
        fseek(file_system, file_inode.addr[logical_block]*BLOCK_SIZE+word_in_block*4, SEEK_SET);
        fwrite(&block_num_to,sizeof(block_num_to),1,file_system);
    } 

    if (logical_block >= 9 && logical_block < 256) {
        printf("Hi\n");
        if (logical_block > prev_logical_block){
            sec_ind_block = get_free_block(file_system);
            //printf(" -- logical_block = %d, file inode addr =  %d \n",logical_block,file_inode.addr[9]);
            fseek(file_system, file_inode.addr[9]*BLOCK_SIZE+(logical_block-9)*4, SEEK_SET);
            fwrite(&sec_ind_block,sizeof(sec_ind_block),1,file_system);
            fflush(file_system);
        }
        fseek(file_system, file_inode.addr[9]*BLOCK_SIZE+(logical_block-9)*4, SEEK_SET);
        fread(&sec_ind_block,sizeof(sec_ind_block),1,file_system);

        fseek(file_system, sec_ind_block*BLOCK_SIZE+word_in_block*4, SEEK_SET);
        fwrite(&block_num_to,sizeof(block_num_to),1,file_system);
    } 
    
    if(logical_block >= 256 && logical_block_t >= 0 && logical_block_t < 256)  {
        
        if (logical_block > prev_logical_block){ 
            if (logical_block_t > prev_logical_block_t) { 
                triple_ind_block = get_free_block(file_system);
                fseek(file_system, file_inode.addr[10]*BLOCK_SIZE+(logical_block_t)*4, SEEK_SET);
                fwrite(&triple_ind_block,sizeof(triple_ind_block),1,file_system);
            }

            fseek(file_system, file_inode.addr[10]*BLOCK_SIZE+(logical_block_t)*4, SEEK_SET);
            fread(&triple_ind_block,sizeof(triple_ind_block),1,file_system);
            fseek(file_system, triple_ind_block*BLOCK_SIZE+(logical_block-10)*4, SEEK_SET);
            sec_ind_block = get_free_block(file_system);
            fwrite(&sec_ind_block,sizeof(sec_ind_block),1,file_system);
            fflush(file_system);

        }

        fseek(file_system, file_inode.addr[10]*BLOCK_SIZE+(logical_block_t)*4, SEEK_SET);
        fread(&triple_ind_block,sizeof(triple_ind_block),1,file_system);

        fseek(file_system, triple_ind_block*BLOCK_SIZE+(logical_block-10)*4, SEEK_SET);
        fread(&sec_ind_block,sizeof(sec_ind_block),1,file_system);

        fseek(file_system, sec_ind_block*BLOCK_SIZE+word_in_block*4, SEEK_SET);
        fwrite(&block_num_to,sizeof(block_num_to),1,file_system);
    }
    
    write_inode(to_file_inode_num, file_inode, file_system);
    tmp_node = read_inode(to_file_inode_num, file_system);
}

void add_block_to_inode_small_file(int block_order_num, int block_num, int to_file_inode_num, FILE* file_system){  
    inode_struct file_inode, tmp_node;
    unsigned int block_num_to = block_num;
    file_inode = read_inode(to_file_inode_num, file_system);
    file_inode.addr[block_order_num] = block_num_to;			
    write_inode(to_file_inode_num, file_inode, file_system);
    tmp_node = read_inode(to_file_inode_num, file_system);
    return;
}

int get_free_block(FILE* file_system){
    super_block_struct block1;
    free_blocks_struct copy_from_block;
    unsigned int free_block;

    fseek(file_system, BLOCK_SIZE, SEEK_SET);
    fread(&block1,sizeof(block1),1,file_system);
    block1.nfree--;
    free_block = block1.free[block1.nfree];
    if (free_block ==0){ 											
        printf("(\nNo free blocks left");
        fseek(file_system, BLOCK_SIZE, SEEK_SET);
        fwrite(&block1,sizeof(block1),1,file_system);
        fflush(file_system);
        return -1;
    }

    if (block1.nfree == 0) {
        fseek(file_system, BLOCK_SIZE*block1.free[block1.nfree], SEEK_SET);
        fread(&copy_from_block,sizeof(copy_from_block),1,file_system);
        block1.nfree = copy_from_block.nfree;
        copy_data_blocks(copy_from_block.free, block1.free, 200);
        block1.nfree--;
        free_block = block1.free[block1.nfree];
    }

    fseek(file_system, BLOCK_SIZE, SEEK_SET);
    fwrite(&block1,sizeof(block1),1,file_system);
    fflush(file_system);
    return free_block;
}

void add_block_to_free_list(int freed_block_number, FILE* file_system){
    super_block_struct block1;
    free_blocks_struct copy_to_block;

    fseek(file_system, BLOCK_SIZE, SEEK_SET);
    fread(&block1,sizeof(block1),1,file_system);
    if (block1.nfree < 200){
        block1.free[block1.nfree] = freed_block_number;
        block1.nfree++;
    } else{	
        copy_data_blocks(block1.free, copy_to_block.free, 200);
        copy_to_block.nfree = 200;
        fseek(file_system, BLOCK_SIZE*freed_block_number, SEEK_SET);
        fwrite(&copy_to_block,sizeof(copy_to_block),1,file_system);
        block1.nfree =1;
        block1.free[0] = freed_block_number;
    }
    fseek(file_system, BLOCK_SIZE, SEEK_SET);
    fwrite(&block1,sizeof(block1),1,file_system);
    fflush(file_system);

    return;
}

int make_directory(const char* filename,FILE* file_system){
    inode_struct directory_inode, free_node;
    file_struct directory_entry;
    int to_file_inode_num, to_file_first_block, flag, status_ind, file_node_num;

    file_node_num = get_inode_by_file_name(filename,file_system);
    if (file_node_num !=-1) {
        printf("\nDirectory %s already exists. Choose different name",filename);
        return -1;
    }

    int found = 0;
    to_file_inode_num=1;
    while(found == 0) {
        to_file_inode_num++;
        free_node = read_inode(to_file_inode_num,file_system);
        flag = MID(free_node.flags,15,16);  
        if (flag == 0){
            found = 1;
        }
    }

    directory_inode = read_inode(1,file_system);

    printf("\nDirectory node block number is %i",directory_inode.addr[0]);
    fseek(file_system,(BLOCK_SIZE*directory_inode.addr[0]+directory_inode.size1),SEEK_SET);
    directory_entry.inode_offset = to_file_inode_num; 
    strcpy(directory_entry.file_name, filename);
    fwrite(&directory_entry,16,1,file_system);
    directory_inode.size1+=16;
    write_inode(1,directory_inode,file_system);

    free_node.flags=0;       
    free_node.flags |=1 <<15; 
    free_node.flags |=1 <<14; 
    free_node.flags |=0 <<13;

    status_ind = write_inode(to_file_inode_num, free_node, file_system);

    return 0;
}

int add_file_to_dir(const char* to_filename,FILE* file_system){
    inode_struct directory_inode, free_node;
    file_struct directory_entry;
    int to_file_inode_num, to_file_first_block, flag;

    int found = 0;
    to_file_inode_num=1;
    while(found==0){
        to_file_inode_num++;
        free_node = read_inode(to_file_inode_num,file_system);
        flag = MID(free_node.flags,15,16);  
        if (flag == 0){
            found = 1;
        }
    }

    directory_inode = read_inode(1,file_system);

    fseek(file_system,(BLOCK_SIZE*directory_inode.addr[0]+directory_inode.size1),SEEK_SET);
    directory_entry.inode_offset = to_file_inode_num; 
    strcpy(directory_entry.file_name, to_filename);
    fwrite(&directory_entry,16,1,file_system);

    directory_inode.size1+=16;
    write_inode(1,directory_inode,file_system);

    return to_file_inode_num;

}

int cpout(const char* from_filename,const char* to_filename,FILE* file_system){
    int file_node_num;

    file_node_num = get_inode_by_file_name(from_filename,file_system);
    if (file_node_num ==-1){
        printf("\nFile %s not found",from_filename);
        return -1;
    }
    
    FILE* write_to_file;
    unsigned char buffer[BLOCK_SIZE];
    unsigned int next_block_number, block_number_order, number_of_blocks, file_size;
    write_to_file = fopen(to_filename,"w");
    block_number_order=0;
    
    file_size = get_inode_file_size(file_node_num, file_system);
    int number_of_bytes_last_block = file_size%BLOCK_SIZE;
    unsigned char last_buffer[number_of_bytes_last_block];

    if (number_of_bytes_last_block == 0) {
        number_of_blocks = file_size/BLOCK_SIZE;
    } else {
        number_of_blocks = file_size/BLOCK_SIZE +1;
    }
    
    while(block_number_order < number_of_blocks){
        if (file_size > BLOCK_SIZE*10)
            next_block_number = get_block_for_big_file(file_node_num,block_number_order,file_system);
        else
            next_block_number = get_block_for_small_file(file_node_num,block_number_order,file_system);

        fseek(file_system, next_block_number*BLOCK_SIZE, SEEK_SET);
        if ((block_number_order <(number_of_blocks-1)) || (number_of_bytes_last_block == 0)){
            fread(buffer,sizeof(buffer),1,file_system);
            fwrite(buffer,sizeof(buffer),1,write_to_file);
        } else {
            fread(last_buffer,sizeof(last_buffer),1,file_system);
            fwrite(last_buffer,sizeof(last_buffer),1,write_to_file);
        }

        block_number_order++;

    }
    fclose(write_to_file);
    return 0;
}

int get_inode_by_file_name(const char* filename,FILE* file_system){
    int inode_number;
    inode_struct directory_inode;
    file_struct directory_entry;
    directory_inode = read_inode(1,file_system);

    fseek(file_system,(BLOCK_SIZE*directory_inode.addr[0]),SEEK_SET);
    int records=(BLOCK_SIZE-2)/sizeof(directory_entry);
    int i;
    for(i=0;i<records; i++){
        fread(&directory_entry,sizeof(directory_entry),1,file_system);
        if (strcmp(filename,directory_entry.file_name) == 0) {
            return directory_entry.inode_offset;
        }
    }
    printf("\nFile %s not found", filename);
    return -1;
}

void remove_file_from_directory(int file_node_num, FILE* file_system) {
    inode_struct directory_inode;
    file_struct directory_entry;
    directory_inode = read_inode(1, file_system);
    int i;

    fseek(file_system,(BLOCK_SIZE * directory_inode.addr[0] + sizeof(directory_entry) * 2), SEEK_SET);
    int records = (BLOCK_SIZE-2) / sizeof(directory_entry);
    
    for(i = 0; i < records; i++) {
        fread(&directory_entry,sizeof(directory_entry), 1, file_system);
        if (directory_entry.inode_offset == file_node_num){
            fseek(file_system, (-1)*sizeof(directory_entry), SEEK_CUR);
            directory_entry.inode_offset = 0;
            memset(directory_entry.file_name, 0, sizeof(directory_entry.file_name));
            fwrite(&directory_entry, sizeof(directory_entry), 1, file_system);      
            return;
        }
    }
    return;
}

int main(int argc, char *argv[]) {
    int i, j, status, file_size;
    int num_blocks;
    int num_inodes;
    int num_data_blocks;
    int num_inode_blocks;
    int max = 400;
    int cmd_counter = 0;

    FILE* file_system = NULL;
    
    char c;
    char *filename;
    char *command = (char*)malloc(max);
    char *cmd;
    char *arg;

    while(1) {
        printf("v6filesystem $ ");
        cmd_counter++;
        scanf(" %[^\n]s", command);
        
        if (strcmp(command, "q") == 0){
            printf("Number of executed commands is.. %i\n", cmd_counter);
            printf("Exiting the filesystem. Saving all changes.\n");
            if (file_system != NULL) {
                fclose(file_system);
            }
            exit(0);
        }

        cmd = strtok(command, " \n");
        printf("cmd %s\n", cmd);

        if (strcmp(cmd, "initfs") == 0) {
            filename = strtok(NULL, " ");
            printf("%s\n", filename);
            printf("Init file_system was requested: %s\n",filename);
            arg = strtok (NULL, " ");
            num_blocks = atoi(arg);
            arg = strtok (NULL, " ");
            num_inodes = atoi(arg);
            num_inode_blocks = ceil(num_inodes / 64); 
            num_data_blocks = num_blocks - num_inode_blocks - 2 - 1;

            if (num_data_blocks < 0) {
                printf("Error in data\n");
            } else {
                printf("Total blocks:%d, Total inodes:%d\n", num_blocks, num_inodes);
                printf("Blocks for data:%d, Blocks for inodes:%d\n", num_data_blocks, num_inode_blocks);
                file_system = fopen(filename, "w+");

                if (file_system == NULL) {
                    printf("Error in opening filesystem. Retry...\n");
                } else {
                    status = initialize_filesystem(num_blocks, num_inodes, file_system);
                    if (status == 0) {
                        printf("\nFile system successfully initialized\n");
                    } else {
                        printf("\nFile system initialization failed\n");
                    }
                }
            }
        } else if (strcmp(command,"cpin")==0){
            printf("cpin was requested\n");
            char *p    = strtok (NULL, " ");
            const char *from_filename = p;
            p = strtok (NULL, " ");
            const char *to_filename = p;
            if (strlen(to_filename) > 14) {
                printf("Target file name  %s is too long:%i, maximum length is 14",to_filename,strlen(to_filename));
                status=1;
            } else
                status = cpin(from_filename,to_filename,file_system);
            
            if (status == 0)
                printf("\nFile  successfully copied\n");
            else
                printf("\nFile copy failed\n");

        } else if (strcmp(command,"cpout")==0){
            printf("cpout was requested\n");
            char *  p    = strtok (NULL, " ");
            const char *from_filename = p;
            p = strtok (NULL, " ");
            const char *to_filename = p;
            status = cpout(from_filename,to_filename,file_system);
            if (status ==0)
                printf("\nFile %s successfully copied\n",from_filename);
            else
                printf("\nFile copy failed\n");		
        } else if (strcmp(command,"rm")==0){
            printf("rm was requested\n");
            char *  p    = strtok (NULL, " ");
            const char *filename = p;
            status = Rm(filename,file_system);
            if (status ==0)
                printf("\nFile  successfully removed\n");
            else
                printf("\nFile removal failed\n");		
        } else if (strcmp(command,"mkdir") == 0) {
            printf("mkdir was requested\n");
            char *  p    = strtok (NULL, " ");
            const char *filename = p;
            status = make_directory(filename,file_system);
            if (status ==0)
                printf("\nDirectory %s successfully created\n",filename);
            else
                printf("\nDirectory creation failed\n");		
        } else {
            printf("Not valid command. Available commands: initfs, cpin, cpout, rm, q, mkdir. Please try again\n");
        }
    }
}
