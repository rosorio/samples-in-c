# Decoding protobuf in C

This is a sample code who decodes a protobuf message without prior knowledge 
of the .proto structure.
This code was write and debug using the following ressouces :
 * [https://protobuf-decoder.netlify.app/](Protobuf Decoder)
 * [https://www.domaintools.com/resources/blog/google-protocol-buffer-deserialization-the-hard-way/](Google Protocol Buffer Deserialization The Hard Way)
 * [https://protobuf.dev/programming-guides/](Protocol Buffers Documentation)

> [!CAUTION]
> The iterative sub-messages parsing can be wrong because we try to guess
> if the embedded messages is a protobuf itself.
