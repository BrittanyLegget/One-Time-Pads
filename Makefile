main:
	gcc -std=c99 -pthread -o keygen keygen.c
	gcc -std=c99 -pthread -o enc_client enc_client.c
	gcc -std=c99 -pthread -o enc_server enc_server.c
	gcc -std=c99 -pthread -o dec_client dec_client.c
	gcc -std=c99 -pthread -o dec_server dec_server.c

clean:
	rm -f keygen
	rm -f enc_client
	rm -f enc_server
	rm -f dec_client
	rm -f dec_server
