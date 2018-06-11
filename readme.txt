Instruções de compilação no relatório.
Mas se quiser aqui tem também:
	make clean
	make all
	make run-server
	make run-client
Esses comandos assumem a pré-configuração que está em código. Para configurar por conta própria, use:
	​ ./build/out -p <porta> -s
para rodar o servidor e:
	./build/out -p <porta> -c <ip_servidor>
para rodar o cliente.