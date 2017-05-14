Número de elementos do grupo - 3

Elementos:
	Afonso Jorge Moreira Maia Ramos - up2015060239
	Carlos Miguel da Silva de Freitas - up201504749
	Diogo Filipe Alves Dores - up201504614
	
Sucintamente, para resolver as situações de competição(race conditions) no acesso a elementos partilhados , decidimos
recorrer ao uso de mutexes, em que existe uma variavel que é o numero de lugares vagos na sauna.Esta variavel vai ser alterada
por cada thread que representa uma pessoa a usar a sauna, logo e usado mutex para resolver os conflitos de haver varias threads a 
tentar alterar a variavel ao mesmo tempo.	