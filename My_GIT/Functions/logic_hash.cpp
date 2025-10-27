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
        ������ ������������ ����� �����
        P(A - �������, ��� ��������� ���� �� ���� ��������) = 1 - e^(-n^2 / (2*M)), ���
        n - ���������� ��������������� ID
        M - ���-�� ��������� �������� - 2^32

        M = 4,294,967,296
        ��� 1000
            n = 1000
            P(A) = 0,01%
        ��� 10,000
            n = 10,000
            p(a) = 1,12%
        ��� 20,000
            n = 20,000
            P(A) = 4,51%
        � �.�
        ����� �������� ������� �������� ���������� �� 50,000 (P(A) = 25%)
        �� ����������� ����, ��� �������� �� 50,000 �������� ������ ���� (����� �������� 0)
        ������� ��� ���� � ���� ������ ������ ��������.
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