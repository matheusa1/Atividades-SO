#include <stdint.h>

typedef struct {
    char     DIR_Name[11];        // Nome curto (8.3)
    uint8_t  DIR_Attr;            // Atributos do arquivo
    uint8_t  DIR_NTRes;           // Reservado para NT (geralmente 0)
    uint8_t  DIR_CrtTimeTenth;    // Décimos de segundo da hora de criação
    uint16_t DIR_CrtTime;         // Hora de criação
    uint16_t DIR_CrtDate;         // Data de criação
    uint16_t DIR_LstAccDate;      // Data do último acesso
    uint16_t DIR_FstClusHI;       // Parte alta do número do primeiro cluster
    uint16_t DIR_WrtTime;         // Hora da última modificação
    uint16_t DIR_WrtDate;         // Data da última modificação
    uint16_t DIR_FstClusLO;       // Parte baixa do número do primeiro cluster
    uint32_t DIR_FileSize;        // Tamanho do arquivo em bytes
} FAT32_DirEntry;