/*
 *      Tool to convert ELF image to be the AP downloadable binary
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: cvimg.c,v 1.4 2009/06/12 07:10:44 michael Exp $
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
//#undef HOME_GATEWAY
//#define HOME_GATEWAY

#define _LITTLE_ENDIAN_

#if defined(_LITTLE_ENDIAN_)
#define DWORD_SWAP(v)                       \
  ((((v & 0xff) << 24) & 0xff000000) |      \
   ((((v >> 8) & 0xff) << 16) & 0xff0000) | \
   ((((v >> 16) & 0xff) << 8) & 0xff00) | (((v >> 24) & 0xff) & 0xff))
#define WORD_SWAP(v) ((unsigned short)(((v >> 8) & 0xff) | ((v << 8) & 0xff00)))
#elif defined(_BIG_ENDIAN_)
#define DWORD_SWAP(v) (v)
#define WORD_SWAP(v) (v)
#else
#error "Endian is not defined"
#endif

#ifdef HEADER_LEN_INT
#define HEADER_SWAP(v) DWORD_SWAP(v)
#else
#define HEADER_SWAP(v) WORD_SWAP(v)
#endif

#define SIGNATURE_LEN 4

/* Firmware image file header */
typedef struct img_header {
  unsigned char signature[SIGNATURE_LEN];
  unsigned int startAddr;
  unsigned int burnAddr;
  unsigned int len;
} IMG_HEADER_T, *IMG_HEADER_Tp;

/* 32-bit ELF base types. */
typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef unsigned int Elf32_Word;

#define EI_NIDENT 16

typedef struct elf32_hdr {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry; /* Entry point */
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;

static unsigned short calculateChecksum(char *buf, int len);

/////////////////////////////////////////////////////////
static unsigned int calculate_long_checksum(unsigned int *buf, int len) {
  int i, j;
  unsigned int sum = 0, tmp;

  j = len / 4;

  for (i = 0; i < j; buf++, i++) {
    tmp = *buf;
    sum += DWORD_SWAP(tmp);
  }
  return ~sum + 1;
}

unsigned int extractStartAddr(char *filename) {
  int fh;
  Elf32_Ehdr hdr;
  char *buf;

  buf = (char *)&hdr;
  fh = open(filename, O_RDONLY);
  if (fh == -1) {
    printf("Open input file error2!\n");
    exit(1);
  }
  lseek(fh, 0L, SEEK_SET);
  if (read(fh, buf, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
    printf("Read file error2!\n");
    close(fh);
    exit(1);
  }
  close(fh);

  return (hdr.e_entry);
}

void printf_usage(void) {
  printf("Version: 1.1\n");
  printf(
      "Usage: cvimg <option> input-filename output-filename start-addr "
      "burn-addr [signature]\n");
  printf("<option>: root|linux-ro|boot|linux|signature\n");
  printf("[signature]: user-specified signature (4 characters)\n");
}

int main(int argc, char **argv) {
  const char *inFile = NULL, *outFile = NULL;
  int fh, size;
  struct stat status;
  char *buf;
  IMG_HEADER_Tp pHeader;
  unsigned int startAddr;
  unsigned int burnAddr;
  unsigned short checksum = 0;
  unsigned int padding_len = 0;
  unsigned int start_addr = 0;

  if (argc < 6) {
    printf_usage();
	exit(1);
  }

  inFile = argv[2];
  outFile = argv[3];

  sscanf(argv[4], "%x", &startAddr);
  sscanf(argv[5], "%x", &burnAddr);

  // check input file and allocate buffer
  if (stat(inFile, &status) < 0) {
    printf("Can't stat file! [%s]\n", inFile);
    exit(1);
  }

  size = status.st_size + sizeof(IMG_HEADER_T);
  if (size % 2) size++;  // pad

  buf = malloc(size);
  if (buf == NULL) {
    printf("Malloc buffer failed!\n");
    exit(1);
  }
  memset(buf, '\0', size);
  pHeader = (IMG_HEADER_Tp)buf;
  buf += sizeof(IMG_HEADER_T);

  // Read data and generate header
  fh = open(inFile, O_RDONLY);
  if (fh == -1) {
    printf("Open input file error!\n");
    free(pHeader);
    exit(1);
  }
  lseek(fh, 0L, SEEK_SET);
  if (read(fh, buf, status.st_size) != status.st_size) {
    printf("Read file error!\n");
    close(fh);
    free(pHeader);
    exit(1);
  }
  close(fh);

  {
    if (!strcmp("root", argv[1]))
      memcpy(pHeader->signature, "syfs", SIGNATURE_LEN);
    else if (!strcmp("boot", argv[1]))
      memcpy(pHeader->signature, "boot", SIGNATURE_LEN);
    else if (!strcmp("linux", argv[1]))
      memcpy(pHeader->signature, "kern", SIGNATURE_LEN);
    else if (!strcmp("linux-ro", argv[1]))
      memcpy(pHeader->signature, "cr6c", SIGNATURE_LEN);
    else if (!strcmp("signature", argv[1]))
      memcpy(pHeader->signature, argv[6], SIGNATURE_LEN);
    else {
      printf("not supported signature\n");
      exit(1);
    }
    pHeader->len = DWORD_SWAP((size - sizeof(IMG_HEADER_T)));
    pHeader->startAddr = DWORD_SWAP(startAddr);
    pHeader->burnAddr = DWORD_SWAP(burnAddr);
    checksum = WORD_SWAP(calculateChecksum(buf, status.st_size));
    *((unsigned short *)&buf[size - sizeof(IMG_HEADER_T) - sizeof(checksum)]) =
        checksum;
  }

  // Write image to output file
  fh = open(outFile, O_RDWR | O_CREAT | O_TRUNC, 0600);
  if (fh == -1) {
    printf("Create output file error! [%s]\n", outFile);
    free(pHeader);
    exit(1);
  }
  write(fh, pHeader, size);
  close(fh);
  chmod(outFile, DEFFILEMODE);

  printf(
      "Generate image successfully"
      "\n\t start: %x, burn: %x, length=%d, checksum=0x%x\n",
      pHeader->startAddr, pHeader->burnAddr, (int)DWORD_SWAP(pHeader->len),
      checksum);

  free(pHeader);
  return 0;
}

static unsigned short calculateChecksum(char *buf, int len) {
  int i, j;
  unsigned short sum = 0, tmp;

  j = (len / 2) * 2;

  for (i = 0; i < j; i += 2) {
    tmp = *((unsigned short *)(buf + i));
    sum += WORD_SWAP(tmp);
  }

  if (len % 2) {
    tmp = buf[len - 1];
    sum += WORD_SWAP(tmp);
  }
  return ~sum + 1;
}
