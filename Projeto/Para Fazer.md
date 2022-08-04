- [ ] \<Tarefa\> \<Tempo\>



***

# 01/08 Seg

- [ ] Verificar ordem de insercao heurística gulosa
- [x] Desabilitar inserção de rotas: sat <estacao...> cliente <estacao...> sat
- [x] Corrigir mv shifit
- [ ] ACO
- [ ] Tabela

Erro movimento shifit: 

	Tempo de saída no mv shifit deve ser de Solucao::satTempoChegMax e nao da instancia!. Provavelmente!


***

***

# Outros
- [ ] Testes com > 1 execução!
- [ ] Peseudo códigos 
	- [ ] Construtívo
	- [x] viabilizador de rota
	- [ ] Gera rotas iniciais (somente cliente ou cliente cliente)
	- [x] Atualiza prob. add rota inicial




***
***

# Mudanças

- Numéro de rotas fixas: min(numEvs, 5)



***
***

# ACO

- Em um cliente i, quais arestas j são válidas?
	- cliente j não visitado (viável)
	- estação de recarga j
	- Viável: É possível ir do cliente até uma estação de recarga ou voltar para o depósito.