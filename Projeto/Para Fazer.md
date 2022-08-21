-
- [ ] \<Tarefa\> \<Tempo\>



***

# 09/08 Qua

- [x] Restringir o numero de EVs
- [ ] ACO
- [ ] Tabela

Erro movimento shifit: 

	Tempo de saída no mv shifit deve ser de Solucao::satTempoChegMax e nao da instancia!. Provavelmente!

NumEVs:

- Quando numEvs == numMaxEvs:
	- Percorrer lista de candidatos 
	- Se evRoute do candidato i é vazia, encontrar uma nova rota(não vazia) para o cliente

***

***

# Outros
- [ ] Verificação da Solução(num. de EVs)
- [ ] Testes com > 1 execução!
- [ ] Peseudo códigos 
	- [ ] Construtívo
	- [x] viabilizador de rota
	- [ ] Gera rotas iniciais (somente cliente ou cliente cliente)
	- [x] Atualiza prob. add rota inicial




***
***

# Mudanças

- Número de rotas fixas: min(numEvMax, 5)
- Número max de EVs


***
***

# ACO

- Em um cliente i, quais arestas j são válidas?
	- cliente j não visitado (viável)
	- estação de recarga j
	- Viável: É possível ir do cliente até uma estação de recarga ou voltar para o depósito.


- feromônio incial igual 0

***
***


# Problemas:
-  


- ERRO HEURISTICA:
	- ./run ../../instancias/2e-vrp-tw/Customer_15/C202_C15x.txt 1661100950


























---

# Gurobi
- Parâmetros:
	- `NoRelHeurTime`
	- Symmetry
- 

