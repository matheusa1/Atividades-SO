# Atividades de SO

Repositório contendo os códigos das atividades 2 de Sistemas Operacionais.

## Tree

Informe a profundidade da árvore a ser criada.

### Bibliotecas utilizadas:

- stdio.h
- sys/types.h
- unistd.h
- sys/wait.h

### Compilar o código:

```bash
  make tree
```

### Como executar:

```bash
  ./main
```

Informe o número de galhos requeridos que será criado uma sequencia de fork() com o número de galhos desejados.

## CommandChild

Informe um comando, e o processo filho executará.

### Bibliotecas utilizadas:

- stdio.h
- unistd.h
- sys/types.h
- string.h
- sys/wait.h

### Compilar o código:

```bash
  make command
```

### Como executar:

```bash
  ./main <comando> [argumentos...]
```

#### Exemplo

```bash
  ./main ls -lha
```

```bash
  ./main touch teste.txt
```

## ArrayDivision

Informe a quantidade de filhos, o tamanho do vetor e o valor a ser buscado.
O programa te retornará a posição do array onde o valor desejado estiver, caso exista o valor no array.

### Libs utilizadas:

- stdio.h
- unistd.h
- sys/types.h
- stdlib.h

### Compilar o código:

```bash
  make array
```

### Como executar

```bash
  ./main
```

## Simple Shell

Uma simples interface de shell, feita para rodar o comando desejado.
Após executar o programa, pode ser utilizado como uma shell normal, podendo ser executado comandos em segundo plano e com o operador |

### Bibliotecas utilizadas:

- stdio.h
- unistd.h
- string.h
- sys/wait.h

### Compilar o código:

```bash
  make shell
```

### Como executar:

```bash
  ./main
```
