# Laboratório 3

---

## Array

Informe o tamanho de um vetor e a quantidade de threads, o programa irá preencher o vetor com valores aleatórios e perguntar o valor que deve ser achado neste vetor.

### Bibliotecas usadas

- stdio.h
- stdlib.h
- pthread.h

### Compilação

```bash
make array
```

### Uso

```bash
./array <tamanho do vetor> <quantidade_threads>
```

#### Exemplo

```bash
./array 100 4
```

---

## Matriz

Informe a quantidade de threads e a matriz de entrada, o programa irá dividir a matriz para os número de threads calcular a média aritmética de cada linha e a média geométrica de cada coluna.

### Bibliotecas usadas

- math.h
- pthread.h
- stdio.h
- stdlib.h

#### Dependências locais

- matriz/matriz.h

### Compilação

```bash
make matriz
```

### Uso

Primeiro gere a matriz de entrada:

```bash
./createMatrix <nr linhas> <nr colunas>
```

Depois execute o programa com a matriz gerada:

```bash
./matriz <nr threads> matrix.in
```

Para um uso direto:

```bash
  ./createMatrix <nr linhas> <nr colunas> && ./matriz <nr threads> matrix.in
```

#### Exemplo

```bash
./createMatrix 3 3
./matriz 3 matrix.in
```

Exemplo de uso direto:

```bash
  ./createMatrix 3 3 && ./matriz 3 matrix.in

```
