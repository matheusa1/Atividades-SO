# ex2.c

---

## Descrição

Este programa implementa o problema clássico do Jantar dos Filósofos usando monitores. O programa simula o comportamento de cinco filósofos que alternam entre pensar e comer, garantindo que não ocorram condições de corrida ou deadlocks, através do uso de mutexes e variáveis de condição.

### Bibliotecas usadas

- pthread.h
- stdio.h
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

O programa irá iniciar a simulação, exibindo o estado de cada filósofo enquanto pensam e comem.

#### Exemplo

```bash
make run
Filósofo 0 está PENSANDO.
Filósofo 1 está PENSANDO.
Filósofo 2 está PENSANDO.
Filósofo 3 está PENSANDO.
Filósofo 4 está PENSANDO.
Filósofo 0 está COMENDO.
Filósofo 2 está COMENDO.
Filósofo 0 está PENSANDO.
Filósofo 2 está PENSANDO.
...
```

---

## Funcionamento

1. **Inicialização**:

   - Define-se o número de filósofos (`N = 5`).
   - Declara-se um array `state[N]` para representar o estado de cada filósofo: `PENSANDO`, `FAMINTO` ou `COMENDO`.
   - Inicializa-se um mutex `pthread_mutex_t mutex` para controlar o acesso à região crítica.
   - Inicializa-se um array de variáveis de condição `pthread_cond_t cond[N]` para cada filósofo.

2. **Execução Principal**:

   - As variáveis mutex e condição são inicializadas.
   - Cria-se uma thread para cada filósofo, que executa a função `philosopher`.

3. **Função `philosopher`**:

   - Cada filósofo entra em um loop onde alterna entre pensar e comer por cinco vezes.
   - **Pensar**:
     - O filósofo exibe que está pensando e realiza um `sleep(1)` para simular o tempo de pensamento.
   - **Pegar Garfos**:
     - Chama a função `takeChopsticks(i)` para tentar pegar os garfos (recursos compartilhados).
     - Se não for possível comer imediatamente, o filósofo espera na variável de condição.
   - **Comer**:
     - O filósofo exibe que está comendo e realiza um `sleep(1)` para simular o tempo de refeição.
   - **Devolver Garfos**:
     - Chama a função `putChopsticks(i)` para liberar os garfos.
   - Após cinco iterações, o filósofo anuncia que terminou e a thread termina.

4. **Função `takeChopsticks(int i)`**:

   - Bloqueia o mutex para acesso à região crítica.
   - Define o estado do filósofo como `FAMINTO` e chama a função `test(i)` para verificar se pode comer.
   - Se não puder comer, espera na variável de condição associada ao filósofo.
   - Desbloqueia o mutex.

5. **Função `putChopsticks(int i)`**:

   - Bloqueia o mutex.
   - Define o estado do filósofo como `PENSANDO`.
   - Chama `test` para os filósofos à esquerda e à direita para verificar se eles podem comer.
   - Desbloqueia o mutex.

6. **Função `test(int i)`**:

   - Verifica se o filósofo está faminto e se os vizinhos não estão comendo.
   - Se as condições forem satisfatórias, o filósofo pode comer:
     - Define seu estado como `COMENDO`.
     - Sinaliza a variável de condição associada para despertar qualquer thread em espera.

7. **Sincronização e Exclusão Mútua**:

   - O uso de mutexes garante que apenas um filósofo por vez execute as operações críticas de pegar e soltar os garfos.
   - As variáveis de condição permitem que os filósofos esperem de forma eficiente até que possam comer.

8. **Finalização**:
   - O programa aguarda que todas as threads de filósofos terminem.
   - Como cada filósofo realiza o ciclo cinco vezes, após a conclusão, o programa termina.

---

## Autor

Matheus Santos de Andrade

## Data

13/12/2024
