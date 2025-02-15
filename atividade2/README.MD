# Shell para Sistema de Arquivos FAT32

Este projeto implementa um shell interativo para navegar e manipular um sistema de arquivos FAT32.

## Funcionalidades

O shell suporta os seguintes comandos:

- `ls` - Lista arquivos e diretórios
- `cd` - Navega entre diretórios
- `pwd` - Mostra o diretório atual
- `mkdir` - Cria um novo diretório
- `rmdir` - Remove um diretório vazio
- `touch` - Cria um arquivo vazio
- `rm` - Remove um arquivo
- `cp` - Copia arquivos
- `mv` - Move/renomeia arquivos
- `rename` - Renomeia arquivos
- `attr` - Mostra atributos de um arquivo
- `info` - Mostra informações do sistema de arquivos
- `cluster` - Mostra conteúdo de um cluster específico
- `exit` - Sai do shell

## Compilação

Para compilar o projeto:

```bash
make c
```

## Execução

Para executar:

```bash
make r
```

Para limpar os arquivos gerados:

```bash
make cl
```

## Estrutura

O projeto está organizado em módulos:

- `core/modules/DirEntry` - Estruturas de entrada de diretório
- `core/modules/FAT` - Estruturas e funções do sistema FAT32
- `core/modules/common` - Funções comuns utilizadas por vários módulos
- `core/modules/commands` - Implementação dos comandos do shell

## Implementação

- Usa alocação dinâmica de memória para manipular clusters e entradas de diretório
- Suporta navegação por toda a hierarquia de diretórios
- Permite operações entre sistema host e imagem FAT32
- Mantém a estrutura do sistema de arquivos consistente

## Autor

- Matheus Santos Andrade
- Joao Vitor Girotto

## Data

13/12/2023
