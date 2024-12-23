# Projeto 1 - Simulação de Avaliação de Padawans Utilizando Semáforos

---

## Descrição

Este programa simula um sistema de avaliação de Padawans que são avaliados pelo Mestre Yoda, com a presença de espectadores assistindo aos testes. O programa utiliza threads e semáforos para gerenciar a sincronização entre os Padawans, Yoda e o público, garantindo que as interações ocorram de forma correta e coordenada.

### Bibliotecas usadas

- `stdio.h`
- `stdlib.h`
- `pthread.h`
- `semaphore.h`
- `unistd.h`

### Compilação

Para compilar o programa, utilize o comando:

```bash
make co
```

### Uso

Para executar o programa, utilize o comando:

```bash
make r
```

---

## Funcionamento

1. **Inicialização**:

   - O programa inicia inicializando os semáforos que serão utilizados para sincronização entre as threads.

2. **Criação das Threads**:

   - Cria-se uma thread para o Mestre Yoda.
   - Cria-se um conjunto de threads para os Padawans, de acordo com a constante `NUM_PADAWAN`.
   - Cria-se um conjunto de threads para o público, de acordo com a constante `NUM_PUBLICO`.

3. **Processo dos Padawans**:

   - Cada Padawan executa a função `padawan()`, que representa seu comportamento:
     - Aguarda a liberação para entrar no salão.
     - Cumprimenta os mestres avaliadores.
     - Aguarda a avaliação.
     - Realiza a avaliação.
     - Aguarda o resultado da avaliação.
     - Se aprovado, aguarda pelo corte de trança.
     - Se reprovado, cumprimenta Yoda.
     - Sai do salão.

4. **Processo do Público**:

   - Cada espectador executa a função `publico()`, que representa seu comportamento:
     - Aguarda a liberação para entrar no salão.
     - Assiste aos testes enquanto há Padawans restantes.
     - Sai do salão.

5. **Processo de Yoda**:

   - A thread de Yoda executa a função `yoda()`, que coordena todo o processo:
     - Libera a entrada dos Padawans e do público, controlando o número permitido de cada grupo.
     - Inicia os testes após um período de espera.
     - Avalia cada Padawan, decidindo aleatoriamente se ele é aprovado ou reprovado.
     - Anuncia o resultado para cada Padawan.
     - Dependendo do resultado, realiza o corte de trança ou cumprimenta o Padawan.
     - Ao final, faz um discurso baseado no número de Padawans aprovados.

6. **Sincronização**:

   - O programa utiliza semáforos para controlar o acesso ao salão, a interação com Yoda, o início dos testes e para garantir que as ações ocorram na ordem correta.

7. **Finalização**:
   - Após Yoda finalizar o dia, o programa encerra todas as threads e destrói os semáforos utilizados.

---

## Constantes

As constantes que podem ser ajustadas no arquivo `globals.h` são:

- `NUM_PADAWAN`: Número total de Padawans que participarão dos testes (padrão: 7).
- `NUM_PUBLICO`: Número total de espectadores (padrão: 10).
- `NR_PADAWAN_PERMITIDO`: Número de Padawans permitidos no salão ao mesmo tempo (padrão: 4).
- `NR_PUBLICO_PERMITIDO`: Número de espectadores permitidos no salão ao mesmo tempo (padrão: 5).

---

## Estrutura de Pastas

- `main.c`: Arquivo principal que inicia o programa.
- `Makefile`: Script para facilitar a compilação, execução e limpeza dos arquivos.

- `modules/`:
  - `common/`:
    - `globals.h`: Declaração de semáforos, variáveis globais e protótipos das funções comuns.
    - `main.c`: Implementação das funções comuns e inicialização/destruição dos semáforos.
  - `padawan/`:
    - `interface.h`: Prototipação da função `padawan()`.
    - `main.c`: Implementação das ações realizadas pelos Padawans.
  - `publico/`:
    - `interface.h`: Prototipação da função `publico()`.
    - `main.c`: Implementação das ações realizadas pelos espectadores.
  - `yoda/`:
    - `interface.h`: Prototipação da função `yoda()`.
    - `main.c`: Implementação das ações realizadas pelo Mestre Yoda.

---

## Observações

- Para alterar o número de Padawans ou espectadores, ajuste as constantes em `globals.h` e recompile o programa.
- O programa não requer entrada do usuário durante a execução.
- As mensagens impressas no console refletem as ações e interações entre as threads, permitindo acompanhar o fluxo da simulação.

---

## Autor

1. Matheus Santos de Andrade
2. João Vitor Girotto

## Data

23/12/2024

---
