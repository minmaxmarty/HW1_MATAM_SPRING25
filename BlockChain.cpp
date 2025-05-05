#include "BlockChain.h"
#include "Transaction.h"

//-------------------------helper functions----------------------------//

// convert file string data into usable data for building a blockChain
string* getData(string* data, const string &line) {
    int startIndex = 0;
    int currentIndex = 0;
    for (int i = 0; i <= (int)line.size(); ++i) {
        if (line[i] == SPACE || i == (int)line.size()) {
            // if we find a space, or we are at the end of the string, we take the word before it and add it to the data array
            data[currentIndex] = line.substr(startIndex, i - startIndex);
            currentIndex++;
            startIndex = i + 1;
        }
    }
    return data;
}

bool checkSameSenderReceiver(const Block *current, const Block *final) {
    // return true if the sender and receiver are the same
    return current->m_transaction.sender == final->m_transaction.sender &&
        current->m_transaction.receiver == final->m_transaction.receiver;
}

bool compressCheckHelper(const Block* current) {
    return current != nullptr && current->m_prevBlock != nullptr;
}

void deleteBlock(BlockChain& blockChain, Block* blockToDelete) {
    //cache the blocks before and after the victim block
    Block* previousPtr = blockToDelete->m_prevBlock;
    Block* nextPtr = blockToDelete->m_nextBlock;
    if (blockChain.m_newestTransaction == blockChain.m_oldestTransaction) {
        delete blockToDelete;
        blockChain.m_size = 0;
        return;
    }
    if (previousPtr) {
        previousPtr->m_nextBlock = nextPtr;
    }
    else {
        blockChain.m_oldestTransaction = nextPtr;
    }
    if (nextPtr) {
        nextPtr->m_prevBlock = previousPtr;
    }
    else {
        blockChain.m_newestTransaction = previousPtr;
    }
    delete blockToDelete;
    blockChain.m_size--;
}

void deleteBlockChain(BlockChain& blockChain) {
    deleteMultipleBlocks(blockChain, blockChain.m_newestTransaction, blockChain.m_oldestTransaction);
}

void deleteMultipleBlocks(BlockChain& blockChain, Block* startBlock, Block* endBlock) {
    Block* toDelete = startBlock;
    Block* stopBlockPtr = endBlock->m_prevBlock;
    while (toDelete != stopBlockPtr) {
        Block* previous = toDelete->m_prevBlock;
        deleteBlock(blockChain, toDelete);
        toDelete = previous;
    }
}

Block* findFinalSameBlock(Block* current, Block* start, Block* oldest, bool &finalIsOldest) {
    Block* ptr = start;
    while (checkSameSenderReceiver(current, ptr)) {
        current->m_transaction.value += ptr->m_transaction.value;
        if (ptr != oldest) {
            ptr = ptr->m_prevBlock;
        } else {
            finalIsOldest = true;
            break;
        }
    }
    return ptr;
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
    bool finalIsOldest = false;
    for (Block* current = newest; compressCheckHelper(current); current = current->m_prevBlock) {
        Block* finalSameBlock = current->m_prevBlock;
        if (checkSameSenderReceiver(current, finalSameBlock)) {
            finalSameBlock = findFinalSameBlock(current, finalSameBlock, oldest, finalIsOldest);
            if (!finalIsOldest) {
                finalSameBlock = finalSameBlock->m_nextBlock;
            }
            deleteMultipleBlocks(blockChain, current->m_prevBlock, finalSameBlock);
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
