# compression
An implementation of Rincon's Probabilistic Compression Algorithm

Included in this repo. are applications for compressing/decompressing strings.

Client/Server serve as the programs that compress a string.
Decompress takes decompress data of the format specified by the Client/Server and returns the original string.

The client receives input from the user by way of input redirection.
Ex:./client < test1
where test1 is a .txt of the data being compressed.
The client will sort the data, create a socket, and write the information to the socket.
The server, which is running at the same time, will receive the data and proceed to compress it.
Server will spawn an additional process using fork(). This process is used to compress.
Note: This additional process is not used for performance reasons, simply as a way to test the use of processes.
Certain implementation restrictions were put into place due to the requirements of the homework I was assigned.

Finally, client will receive the compressed data back and print it to stdout.

Decompression will take a .txt from the user using input redirection.
Ex: ./decompression < test2
where test2 is a .txt containing the decompressed data. Format is important here.
Next, it will create multiple threads in order to decompress the data. Threads are not used for performance reasons.
As stated earlier, the reason for pointless implementations is that was what was required of by the homework.
The compressed data will be printed to the stdout.

//Many improvements are probably needed.
//Mergesort sorts in the wrong direction for example.
