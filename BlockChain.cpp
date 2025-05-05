#include "BlockChain.h"
#include "Transaction.h"

//-------------------------helper functions----------------------------//
string* getData(string* data, const string &line) {
    int startIndex = 0;
    int currentIndex = 0;
    for (int i = 0; i <= (int)line.size(); ++i) {
        if (line[i] == SPACE || i == (int)line.size()) {
            data[currentIndex] = line.substr(startIndex, i - startIndex);
            currentIndex++;
            startIndex = i + 1;
        }
    }
    return data;
}

bool checkSameSenderReceiver(const Block *current, const Block *final) {
    return current->m_transaction.sender == final->m_transaction.sender && current->m_transaction.receiver == final->m_transaction.receiver;
}

bool compressCheckHelper(const Block* current) {
    return current != nullptr && current->m_prevBlock != nullptr;
}

void deleteBlock(BlockChain& blockChain, Block* blockToDelete) {
    Block* previousPtr = blockToDelete->m_prevBlock;
    Block* nextPtr = blockToDelete->m_nextBlock;

    if (previousPtr) {
        previousPtr->m_nextBlock = nextPtr;
    }
    else {
        blockChain.m_oldestTransaction = nextPtr;
        blockToDelete->m_nextBlock = nullptr;
    }
    if (nextPtr) {
        nextPtr->m_prevBlock = previousPtr;
    }
    else {
        blockChain.m_newestTransaction = previousPtr;
        blockToDelete->m_prevBlock = nullptr;
    }
    delete blockToDelete;
    blockChain.m_size--;
}

void deleteBlockChain(BlockChain& blockChain) {
    const Block* current = blockChain.m_newestTransaction;
    while (current) {
        const Block* previous = current->m_prevBlock;
        delete current;
        current = previous;
    }
    blockChain.m_size = 0;
}

bool compressHelper(BlockChain& blockChain, Block* finalSameBlock, Block* current) {
    Block* toDelete;
    bool toBreak = false;
    if (finalSameBlock->m_nextBlock != current) {
        toDelete = finalSameBlock->m_nextBlock;
    }
    else {
        toDelete = finalSameBlock;
        toBreak = true;
    }
    current->m_transaction.value += toDelete->m_transaction.value;
    deleteBlock(blockChain, toDelete);
    return toBreak;
}

//---------------------------- BlockChain -----------------------------//

int BlockChainGetSize(const BlockChain &blockChain) {
    return blockChain.m_size;
}

int BlockChainPersonalBalance(const BlockChain &blockChain, const string &name) {
    int sum = 0;
    const Block* current = blockChain.m_newestTransaction;
    while (current != nullptr) {
        if (current->m_transaction.receiver == name) {
            sum += (int)current->m_transaction.value;
        }
        else if (current->m_transaction.sender == name) {
            sum -= (int)current->m_transaction.value;
        }
        current = current->m_prevBlock;
    }
    return sum;
}

void BlockChainAppendTransaction(BlockChain &blockChain, unsigned int value, const string &sender,
    const string &receiver, const string &timestamp) {
    const Transaction newTransaction = {value, sender, receiver};
    BlockChainAppendTransaction(blockChain, newTransaction, timestamp);
}

void BlockChainAppendTransaction(BlockChain &blockChain, const Transaction &transaction, const string &timestamp) {
    Block* newBlock = new Block{
        transaction,
        timestamp,
        blockChain.m_oldestTransaction,
        nullptr
    };
    blockChain.m_oldestTransaction->m_prevBlock = newBlock;
    blockChain.m_oldestTransaction = newBlock;
    blockChain.m_size++;
}

BlockChain BlockChainLoad(ifstream &file) {
    string curLine;
    Transaction tempTransaction{};
    Block* tempBlock = new Block{
        tempTransaction,
        EMPTY,
        nullptr,
        nullptr
    };
    BlockChain newBlockChain{tempBlock, tempBlock, 1};
    string data[DATA_SIZE];
    while (getline(file, curLine)) {
        getData(data, curLine);
        BlockChainAppendTransaction(
            newBlockChain,
            stoi(data[VALUE_INDEX]),
            data[SENDER_INDEX],
            data[RECIVER_INDEX],
            data[TIMESTEMP_INDEX]
            );
    }
    deleteBlock(newBlockChain, tempBlock);
    return newBlockChain;
}

void BlockChainDump(const BlockChain &blockChain, ofstream &file) {
    file << "BlockChain Info:" << std::endl;
    int counter = 1;
    const Block* current = blockChain.m_newestTransaction;
    while (current != nullptr) {
        file << counter << "." << std::endl;
        TransactionDumpInfo(current->m_transaction, file);
        file << "Transaction timestamp: " << current->m_timestamp << std::endl;
        current = current->m_prevBlock;
        counter++;
    }
}

void BlockChainDumpHashed(const BlockChain &blockChain, ofstream &file) {
    const Block* current = blockChain.m_newestTransaction;
    const int blockChainSize = BlockChainGetSize(blockChain);
    for (int i = 0; i < blockChainSize; i++) {
        file << TransactionHashedMessage(current->m_transaction);
        if (i != blockChainSize - 1) {
            file << std::endl;
        }
        current = current->m_prevBlock;
    }
}

bool BlockChainVerifyFile(const BlockChain &blockChain, std::ifstream &file) {
    const Block* current = blockChain.m_newestTransaction;
    string curLine;
    while (getline(file, curLine)) {
        if (current == nullptr || !TransactionVerifyHashedMessage(current->m_transaction, curLine)) {
            return false;
        }
        current = current->m_prevBlock;
    }
    if (current == nullptr) {
        return true;
    }
    return false;
}

void BlockChainCompress(BlockChain &blockChain) {
    Block* newest = blockChain.m_newestTransaction;
    Block* oldest = blockChain.m_oldestTransaction;
    for (Block* current = newest; compressCheckHelper(current); current = current->m_prevBlock) {
        Block* finalSameBlock = current->m_prevBlock;
        if (checkSameSenderReceiver(current, finalSameBlock)) {
            for (; finalSameBlock != oldest && checkSameSenderReceiver(current, finalSameBlock->m_prevBlock);
                finalSameBlock = finalSameBlock->m_prevBlock) {}
            while (true) {
                const bool toBreak = compressHelper(blockChain, finalSameBlock, current);
                if (toBreak) {
                    break;
                }
            }
        }
    }
}

void BlockChainTransform(BlockChain &blockChain, updateFunction function) {
    Block* current = blockChain.m_newestTransaction;
    while (current != nullptr) {
        current->m_transaction.value = function(current->m_transaction.value);
        current = current->m_prevBlock;
    }
}
