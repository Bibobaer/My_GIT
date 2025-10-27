#include "logic_hash.hpp"

#include "cryptopp/cryptlib.h"
#include "cryptopp/sha.h"
#include "cryptopp/hex.h"
#include "cryptopp/files.h"

#include <rpc.h>


#pragma comment(lib, "rpcrt4.lib")
unsigned int create_uuid() {
    UUID out;
    UuidCreate(&out);
    return (unsigned int)out.Data1;
    /*
        Вопрос уникальности такой штуки
        P(A - события, что произошла хотя бы одна коллизия) = 1 - e^(-n^2 / (2*M)), где
        n - количество сгенерированных ID
        M - кол-во возможных значкний - 2^32

        M = 4,294,967,296
        Для 1000
            n = 1000
            P(A) = 0,01%
        Для 10,000
            n = 10,000
            p(a) = 1,12%
        Для 20,000
            n = 20,000
            P(A) = 4,51%
        и т.к
        Более значимый вариант коллизии произойдет на 50,000 (P(A) = 25%)
        Но вероятность того, что создатут на 50,000 коммитов крайне мала (почти наверное 0)
        Поэтому все клёва и этот способ должен работать.
    */
}

std::string get_id_hash(const unsigned int& id) {
    CryptoPP::SHA1 hash;
    std::string digest;
    std::string output;

    CryptoPP::StringSource(std::to_string(id), true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::StringSink(digest)
        ) 
    );

    CryptoPP::StringSource(digest, true,
        new CryptoPP::HexEncoder(
            new CryptoPP::StringSink(output)
        )
    );

    return output;
}

std::string get_file_hash(const std::string& filename) {
    CryptoPP::SHA1 hash;

    std::string digest;
    std::string output;

    CryptoPP::FileSource(filename.c_str(), true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::StringSink(digest)
        )
    );

    CryptoPP::StringSource(digest, true,
        new CryptoPP::HexEncoder(
            new CryptoPP::StringSink(output)
        )
    );

	return output;
}