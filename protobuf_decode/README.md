# Decoding protobuf in C

This is a sample code who decodes a protobuf message without prior knowledge 
of the .proto structure.
This code was write and debug using the following ressouces :
 * [Protobuf Decoder](https://protobuf-decoder.netlify.app/)
 * [Google Protocol Buffer Deserialization The Hard Way](https://www.domaintools.com/resources/blog/google-protocol-buffer-deserialization-the-hard-way/)
 * [Protocol Buffers Documentation](https://protobuf.dev/programming-guides/)

> [!CAUTION]
> The iterative sub-messages parsing can be wrong because we try to guess
> if the embedded messages is a protobuf itself.
