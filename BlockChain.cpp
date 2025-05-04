#include "BlockChain.h"

int BlockChainGetSize(const BlockChain &blockChain) {
    return blockChain.size;
}

int BlockChainPersonalBalance(const BlockChain &blockChain, const string &name) {
    int sum = 0;
    const Block* current = blockChain.newest_tran;
    for (int i = 0; i < blockChain.size; i++) {
        if (current->transaction.receiver == name)
            sum += current->transaction.value;
        else if (current->transaction.sender == name)
            sum -= current->transaction.value;
        current = current->prev_block;
    }
    return sum;
}

void BlockChainAppendTransaction(BlockChain &blockChain, unsigned int value, const string &sender,
    const string &receiver, const string &timestamp) {
    Transaction created_transaction = {value, sender, receiver};
    BlockChainAppendTransaction(blockChain, created_transaction, timestamp);
}

void BlockChainAppendTransaction(BlockChain &blockChain, const Transaction &transaction, const string &timestamp) {
    Block* new_tran = new (Block);
    new_tran->transaction = transaction;
    new_tran->timestamp = timestamp;
    new_tran->prev_block = blockChain.newest_tran;
    new_tran->next_block = nullptr;
    blockChain.newest_tran->next_block = new_tran;
    blockChain.newest_tran = new_tran;
    blockChain.size++;
}


string* get_data(const string &line) {
    string* data = new (string[DATA_SIZE]);
    int start_index = 0;
    int cur_data = 0;
    for (int i = 0; i < line.size(); ++i) {
        if (line[i] == SPACE) {
            data[cur_data] = line.substr(start_index, i - start_index);
            cur_data++;
            start_index = i + 1;
        }
    }
    return data;
}

void blockChain_init(BlockChain &blockChain) {
    blockChain.newest_tran->next_block = nullptr;
    blockChain.newest_tran->prev_block = nullptr;
    blockChain.oldest_tran = blockChain.newest_tran;
}

void delete_junk_block(BlockChain &blockChain) {
    blockChain.oldest_tran = blockChain.oldest_tran->next_block;
    blockChain.oldest_tran->prev_block->next_block = nullptr;
    delete blockChain.oldest_tran->prev_block;
    blockChain.oldest_tran->prev_block = nullptr;
}

BlockChain BlockChainLoad(ifstream &file) {
    string cur_line;
    Block* temp_block;
    BlockChain new_blockChain{temp_block, temp_block, 0};
    blockChain_init(new_blockChain);
    const string* data = nullptr;
    while (getline(file, cur_line)) {
        data = get_data(cur_line);
        BlockChainAppendTransaction(new_blockChain, stoi(data[VALUE_INDEX]),
            data[SENDER_INDEX], data[RECIVER_INDEX], data[TIMESTEMP_INDEX]);
    }
    delete[] data;
    delete_junk_block(new_blockChain);
    return new_blockChain;
}

void BlockChainDump(const BlockChain &blockChain, ofstream &file) {
}

void BlockChainDumpHashed(const BlockChain &blockChain, ofstream &file) {
}

bool BlockChainVerifyFile(const BlockChain &blockChain, std::ifstream &file) {
}

void BlockChainCompress(BlockChain &blockChain) {
}

void BlockChainTransform(BlockChain &blockChain, updateFunction function) {
}
