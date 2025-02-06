#pragma once

#include <stdint.h>
#include <stdio.h>

// Estrutura para armazenar informações do setor de inicialização do FAT32
typedef struct __attribute__((packed, aligned(1))) {
    // ----- Parte comum a FAT12/16/32 (BIOS Parameter Block - BPB) -----
    uint8_t     BS_jmpBoot[3];      // Instrução de jump para o código de bootstrap (ex: 0xEB5890)
    uint8_t     BS_OEMName[8];      // Nome do OEM (ex: "MSDOS5.0")
    uint16_t    BPB_BytsPerSec;     // Bytes por setor (ex: 512)
    uint8_t     BPB_SecPerClus;     // Setores por cluster (ex: 1, 2, 4, 8, ..., 128)
    uint16_t    BPB_RsvdSecCnt;     // Número de setores reservados (ex: 32)
    uint8_t     BPB_NumFATs;        // Número de FATs (geralmente 2)
    uint16_t    BPB_RootEntCnt;     // Número de entradas no diretório raiz (0 para FAT32)
    uint16_t    BPB_TotSec16;       // Total de setores (se 0, usar BPB_TotSec32)
    uint8_t     BPB_Media;          // Tipo de mídia (ex: 0xF8 para discos fixos)
    uint16_t    BPB_FATSz16;        // Tamanho de cada FAT em setores (0 para FAT32)
    uint16_t    BPB_SecPerTrk;      // Setores por trilha (geometria do disco)
    uint16_t    BPB_NumHeads;       // Número de cabeças (geometria do disco)
    uint32_t    BPB_HiddSec;        // Setores escondidos antes da partição
    uint32_t    BPB_TotSec32;       // Total de setores (usado se BPB_TotSec16 = 0)

    // ----- Extensão FAT32 (Extended BIOS Parameter Block - EBPB) -----
    uint32_t    BPB_FATSz32;        // Tamanho de cada FAT em setores (FAT32)
    uint16_t    BPB_ExtFlags;       // Flags (ex: espelhamento de FAT)
    uint16_t    BPB_FSVer;          // Versão do sistema de arquivos (ex: 0x0000)
    uint32_t    BPB_RootClus;       // Cluster inicial do diretório raiz (ex: 2)
    uint16_t    BPB_FSInfo;         // Setor da estrutura FSInfo (ex: 1)
    uint16_t    BPB_BkBootSec;      // Cópia de backup do boot sector (ex: 6)
    uint8_t     BPB_Reserved[12];   // Reservado para futuras extensões
    uint8_t     BS_DrvNum;          // Número do drive (BIOS)
    uint8_t     BS_Reserved1;       // Reservado
    uint8_t     BS_BootSig;         // Assinatura de boot (0x29)
    uint32_t    BS_VolID;           // ID do volume (serial number)
    uint8_t     BS_VolLab[11];      // Rótulo do volume (ex: "MYDISK    ")
    uint8_t     BS_FilSysType[8];   // Tipo do sistema de arquivos (ex: "FAT32   ")
} FAT32_BootSector;

typedef struct __attribute__((packed, aligned(1))) {
  uint32_t FSI_LeadSig;
  uint8_t FSI_Reserved1[480];
  uint32_t FSI_StrucSig;
  uint32_t FSI_Free_Count;
  uint32_t FSI_Nxt_Free;
  uint8_t FSI_Reserved2[12];
  uint32_t FSI_TrailSig;
} FSInfoStruct;

// Estrutura para armazenar informações da imagem aberta
typedef struct {
    FILE *file; // Ponteiro para o arquivo da imagem
    FAT32_BootSector boot_sector; // Estrutura com informações do setor de inicialização
    FSInfoStruct fs_info; // Estrutura com informações do setor FSInfo
    uint32_t *fat1; // Ponteiro para a FAT1
} Fat32Image;