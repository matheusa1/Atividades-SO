# Leitores/Escritores com Semáforos

---

## Descrição

Este programa implementa a solução para o problema clássico de leitores/escritores utilizando semáforos para sincronização. O problema de leitores/escritores é um problema de controle de concorrência que envolve múltiplos processos leitores e escritores que acessam um recurso compartilhado. O objetivo é garantir que:

1. Múltiplos leitores possam ler o recurso compartilhado simultaneamente.
2. Apenas um escritor possa modificar o recurso compartilhado de cada vez.
3. Quando um escritor está escrevendo, nenhum leitor pode ler o recurso.

### Bibliotecas usadas

- pthread.h
- semaphore.h
- stdio.h
- stdlib.h
- unistd.h

### Compilação

Para compilar o programa, utilize o comando:

```bash
make compile
```

### Uso

Para executar o programa, utilize o comando:

```bash
make run
```

O programa cria 5 threads leitoras e 2 threads escritoras que acessam o recurso compartilhado de forma concorrente.

---

## Funcionamento

1. O programa inicializa os semáforos `mutex` e `writer`.
2. Cria 5 threads leitoras e 2 threads escritoras.
3. Cada thread leitora:
   - Incrementa o contador de leitores.
   - Se for o primeiro leitor, bloqueia os escritores.
   - Lê o recurso compartilhado.
   - Decrementa o contador de leitores.
   - Se for o último leitor, libera os escritores.
4. Cada thread escritora:
   - Bloqueia o acesso ao recurso compartilhado.
   - Modifica o recurso compartilhado.
   - Libera o acesso ao recurso compartilhado.
5. As threads leitoras e escritoras repetem suas operações 5 vezes.
6. O programa espera todas as threads terminarem.
7. Destrói os semáforos.

---

## Autor

Matheus Santos de Andrade

## Data

13/12/2024
