#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<error.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<scsi/scsi_ioctl.h>
#include<scsi/sg.h>

/* writes the LBA entered from the terminal */
/* compile: gcc file.c*/
/* execute: sudo ./a.out /dev/sg<> */

#define LBA_SIZE 512
#define CMD_LEN 16
#define LBA_MAX (1<<30)

int main(int argc, char* argv[]){
  int fd,i;
  int lba;
  int no_of_blocks = 2;
  printf("Enter the LBA you want to write to: ");
  scanf("%d",&lba);

  if(lba> LBA_MAX){
	printf("LBA greater than (1<<30) is not allowed with this version: ");
	return 1;
  }
  unsigned char cmd_blk[CMD_LEN] =
   {0x85,0x0D,0x26,0,0,(no_of_blocks >> 8),no_of_blocks,(lba >> 8),lba,(lba>>24),(lba>>16),0,0,0x40,0x35,0};

  sg_io_hdr_t io_hdr;
  char* file_name = 0;
  unsigned char buffer[LBA_SIZE * no_of_blocks];
  unsigned char sense_buffer[32];

  if(argc < 2){
	printf("please enter a device file\n");
	return 1;
  }else{
	file_name = argv[1];

  }

  /////////opening the device file/////////////

  if((fd = open(file_name,O_RDWR))<0){
	printf("device file opening failed\n");
	return 1;
  }

  /////////// data buffer ///////////

  printf("********data buffer initializing***********\n");
  for(i=0;i<512*no_of_blocks;i++){
	buffer[i] = 0;
  }/* code will write my name */
  buffer[2] = 'A';
  buffer[3] = 'M';
  buffer[4] = 'A';
  buffer[5] = 'R';

  printf("\n");

  printf("********data buffer after initialling***********\n");
  for(i=0;i<512*no_of_blocks;i++){
	printf("%hx ",buffer[i]);
  }
  printf("\n");
  ////////////////prepare sg header for write//////
  memset(&io_hdr,0,sizeof(sg_io_hdr_t));
  io_hdr.interface_id = 'S';
  io_hdr.cmd_len = sizeof(cmd_blk);
  io_hdr.mx_sb_len = sizeof(sense_buffer);
  io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
  io_hdr.dxfer_len = LBA_SIZE*no_of_blocks;
  io_hdr.dxferp = buffer;
  io_hdr.cmdp = cmd_blk;
  io_hdr.sbp = sense_buffer;
  io_hdr.timeout = 20000;


  if(ioctl(fd,SG_IO,&io_hdr)<0){
	printf("ioctl failed\n");
	for(i = 0;i<32;i++){
		printf("%hx ",sense_buffer[i]);
  	}
  	printf("\n");
 	return 1;
  }else printf("write successfull\n");

 printf("duration: %d\n",io_hdr.duration);

 return 1;
}
