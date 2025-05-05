#include "Transaction.h"
#include "BlockChain.h"
#include "Utilities.h"
#include <iostream>

const int MIN_ARG = 4;
const int OP = 1;
const int SOURCE = 2;
const int TARGET = 3;

int main(int argc, char* argv[]) {
    if (argc != MIN_ARG) {
        getErrorMessage();
        return 1;
    }

    std::ifstream source_file(argv[SOURCE]);
    BlockChain blockChain = BlockChainLoad(source_file);
    std::ofstream target_file(argv[TARGET]);
    std::ifstream read_target_file(argv[TARGET]);

    if (argv[OP] == "format") {
        BlockChainDump(blockChain, target_file);
    }

    else if (argv[OP] == "hash") {
        BlockChainDumpHashed(blockChain, target_file);
    }

    else if (argv[OP] == "compress") {
        BlockChainCompress(blockChain);
        BlockChainDump(blockChain, target_file);
    }

    else if (argv[OP] == "verify") {
        bool verification_outcome = BlockChainVerifyFile(blockChain, read_target_file);
        verification_outcome
            ? (std::cout << "Verification passed" << std::endl)
            : (std::cout << "Verification failed" << std::endl);
    }

    return 0;
}