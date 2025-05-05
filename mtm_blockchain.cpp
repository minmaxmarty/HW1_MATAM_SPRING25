#include "Transaction.h"
#include "BlockChain.h"
#include "Utilities.h"
#include <iostream>
#include <cstring>

const int MIN_ARG = 4;
const int OP = 1;
const int SOURCE = 2;
const int TARGET = 3;

bool is_valid_operation(const char* op) {
    return strcmp(op, "verify") == 0 ||
           strcmp(op, "format") == 0 ||
           strcmp(op, "hash") == 0   ||
           strcmp(op, "compress") == 0;
}

int main(int argc, char** argv) {

    if (argc != MIN_ARG || !is_valid_operation(argv[OP])) {
        std::cout << getErrorMessage() << std::endl;
        return 1;
    }

    std::ifstream source_file(argv[SOURCE]);

    BlockChain blockChain = BlockChainLoad(source_file);

    if (strcmp(argv[OP], "verify") == 0) {
        std::ifstream read_target_file(argv[TARGET]);
        bool verification_outcome = BlockChainVerifyFile(blockChain, read_target_file);
        verification_outcome
            ? (std::cout << "Verification passed" << std::endl)
            : (std::cout << "Verification failed" << std::endl);

    } else {
        std::ofstream target_file(argv[TARGET]);
        if (strcmp(argv[OP], "format") == 0) {
            BlockChainDump(blockChain, target_file);
        }

        else if (strcmp(argv[OP], "hash") == 0) {
            BlockChainDumpHashed(blockChain, target_file);
        }

        else if (strcmp(argv[OP], "compress") == 0) {
            BlockChainCompress(blockChain);
            BlockChainDump(blockChain, target_file);
        }
    }

    delete_blockChain(blockChain);

    return 0;
}