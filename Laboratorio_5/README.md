# ex5.c

---

## Descrição

Este programa realiza a soma de dois vetores de inteiros de mesmo tamanho, utilizando processos filhos para dividir o trabalho. O número de elementos do vetor e o número de processos filhos são passados pelo usuário.

### Bibliotecas usadas

- stdio.h
- stdlib.h
- sys/mman.h
- sys/types.h
- sys/wait.h
- unistd.h

### Compilação

Para compilar o programa, utilize o comando:

```bash
make
```

### Uso

Para executar o programa, utilize o comando:

```bash
make run
```

O programa solicitará ao usuário o número de elementos do vetor e o número de processos filhos a serem utilizados.

#### Exemplo

```bash
make run
Digite o número de elementos do vetor: 10
Digite o número de processos filhos: 2
```

---

## Funcionamento

1. O programa solicita ao usuário o número de elementos do vetor e o número de processos filhos.
2. Aloca memória compartilhada para os vetores V1, V2 e V3, e para o vetor de sinalização.
3. Inicializa os vetores V1 e V2 com valores aleatórios.
4. Cria pipes e processos filhos.
5. O pai envia intervalos de índices para os filhos processarem.
6. Cada filho soma os elementos correspondentes de V1 e V2, armazenando o resultado em V3.
7. O pai aguarda todos os filhos terminarem.
8. O programa imprime o vetor resultante V3.
9. Realiza a limpeza da memória compartilhada e espera o término dos processos filhos.

---

## Autor

Matheus Santos de Andrade

## Data

29/11/2024
