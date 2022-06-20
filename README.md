# One-Time-Pads

Five small programs that work together to encrypt and decrypt information using a one-time pad-like system.

## Specifications

The program encrypts and decrypts plaintext into ciphertext, using a key, and modular arithmatic, using modulo 27 operations (the 27 characters are the 26 capital letters, and the space character).

There are five small programs in C. Two function as servers, and will be accessed using network sockets utilizing the POSIX Network IPC API. Two are clients, each one of these will use one of the servers to perform work, and the last program is a standalone utility.

The programs can be compiled using the Makefile by running:

```bash
Make
```

Here are the specifications of the five programs (The 4 network programs use localhost as the target IP address/host for testing):

### enc_server

This program is the encryption server and will run in the background as a daemon.

Its function is to perform the actual encoding and will listen on a particular port/socket, assigned when it is first ran (see syntax below).

When a connection is made, enc_server must call accept to generate the socket used for actual communication, and then use a separate process to handle the rest of the servicing for this client connection (see below), which will occur on the newly accepted socket.

This child process of enc_server must first check to make sure it is communicating with enc_client (see enc_client, below). After verifying that the connection to enc_server is coming from enc_client, then this child receives plaintext and a key from enc_client via the connected socket.

The enc_server child will then write back the ciphertext to the enc_client process that it is connected to via the same connected socket.The program can support up to five concurrent socket connections so since only in the child server process will do the actual encryption the original server daemon process continues listening for new connections, not encrypting data.

starting enc_server:

```bash
enc_server listening_port &
```

listening_port is the port that enc_server should listen on and it should always be started in the background by adding &, so for example:

```bash
$ enc_server 57171 &
```

### enc_client

This program connects to enc_server, and asks it to perform a one-time pad style encryption. By itself, enc_client doesn’t do the encryption - enc_server does. The syntax of enc_client is as follows:

```bash
enc_client plaintext key port
```

- plaintext: the name of a file in the current directory that contains the plaintext you wish to encrypt.
- key: the encryption key you wish to use to encrypt the text (Note that the key passed in must be at least as big as the plaintext).
- port is the port that enc_client should attempt to connect to enc_server on.

When enc_client receives the ciphertext back from enc_server, it will output it to stdout. Thus, enc_client can be launched in any of the following methods, and should send its output appropriately:

```bash
enc_client myplaintext mykey 57171
```

```bash
enc_client myplaintext mykey 57171 > myciphertext
```

```bash
enc_client myplaintext mykey 57171 > myciphertext &
```

### dec_server

This program performs exactly like enc_server, in syntax and usage, however, it will decrypt the ciphertext it is given, using the passed-in ciphertext and key and then return plaintext to the dec_client.

### dec_client

Similarly, this program will connect to dec_server and will ask it to decrypt ciphertext using a passed-in ciphertext and key, and otherwise performs exactly like enc_client, and is runnable in the same three ways. dec_client is NOT be able to connect to enc_server, even if it tries to connect on the correct port.

### keygen

This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard Unix randomization methods.

The syntax for keygen is as follows:

```bash
keygen keylength
```

where keylength is the length of the key file in characters. keygen outputs to stdout.

Here is an example run, which creates a key of 256 characters and redirects stdout a file called mykey (note that mykey is 257 characters long becausethere will be a newline character at the end.):

```bash
$ keygen 256 > mykey
```
