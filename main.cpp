#include "Transaction.h"
#include "BlockChain.h"
#include "Utilities.h"
#include <iostream>

const int MIN_ARG = 4;
const int OP = 1;
const int SOURCE = 2;
const int TARGET = 3;
const int NUM_OF_OP = 4;
const int VERIFY = 0;
const int FORMAT = 1;
const int HASH = 2;
const int COMPRESS = 3;

int main(int argc, char** argv) {

    if (argc != MIN_ARG) {
        std::cout << getErrorMessage() << std::endl;
        return 1;
    }

    string arguments[] = {"verify", "format", "hash", "compress"};
    string currentOperation = argv[OP];
    bool is_valid_operation = false;
    for (int i = 0; i < NUM_OF_OP; i++) {
        if (currentOperation == arguments[i]) {
            is_valid_operation = true;
        }
        if (!is_valid_operation && i == NUM_OF_OP -1 ) {
            std::cout << getErrorMessage() << std::endl;
            return 1;
        }
    }



    std::ifstream source_file(argv[SOURCE]);

    BlockChain blockChain = BlockChainLoad(source_file);

    if (currentOperation == arguments[VERIFY]) {
        std::ifstream read_target_file(argv[TARGET]);
        bool verification_outcome = BlockChainVerifyFile(blockChain, read_target_file);
        verification_outcome
            ? (std::cout << "Verification passed" << std::endl)
            : (std::cout << "Verification failed" << std::endl);

    } else {
        std::ofstream target_file(argv[TARGET]);
        if (currentOperation == arguments[FORMAT]) {
            BlockChainDump(blockChain, target_file);
        }

        else if (currentOperation == arguments[HASH]) {
            BlockChainDumpHashed(blockChain, target_file);
        }

        else if (currentOperation == arguments[COMPRESS]) {
            BlockChainCompress(blockChain);
            BlockChainDump(blockChain, target_file);
        }
    }

    deleteBlockChain(blockChain);

    return 0;
}