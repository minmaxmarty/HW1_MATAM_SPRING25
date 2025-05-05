#include "BlockChain.h"
#include "Transaction.h"

//-------------------------helper functions----------------------------//
string* get_data(string* data, const string &line) {
    int start_index = 0;
    int cur_data = 0;
    for (int i = 0; i <= (int)line.size(); ++i) {
        if (line[i] == SPACE || i == (int)line.size()) {
            data[cur_data] = line.substr(start_index, i - start_index);
            cur_data++;
            start_index = i + 1;
        }
    }
    return data;
}

bool check_if_same(const Block *current, const Block *final) {
    return current->transaction.sender == final->transaction.sender && current->transaction.receiver == final->transaction.receiver;
}

void delete_block(BlockChain& blockChain, Block* block_to_delete) {
    Block* previous = block_to_delete->prev_block;
    Block* next = block_to_delete->next_block;

    if (previous) {
        previous->next_block = next;
    } else {
        blockChain.oldest_transaction = next;
        block_to_delete->next_block = nullptr;
    }
    if (next) {
        next->prev_block = previous;
    } else {
        blockChain.newest_transaction = previous;
        block_to_delete->prev_block = nullptr;
    }
    delete block_to_delete;
    blockChain.size--;
}

void delete_blockChain(BlockChain& blockChain) {
    Block* current = blockChain.newest_transaction;
    while (current) {
        Block* previous = current->prev_block;
        delete current;
        current = previous;
    }
    blockChain.size = 0;
}

//---------------------------- BlockChain -----------------------------//

int BlockChainGetSize(const BlockChain &blockChain) {
    return blockChain.size;
}

int BlockChainPersonalBalance(const BlockChain &blockChain, const string &name) {
    int sum = 0;
    const Block* current = blockChain.newest_transaction;
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
    Block* new_tran = new Block{transaction, timestamp, blockChain.oldest_transaction, nullptr};
    blockChain.oldest_transaction->prev_block = new_tran;
    blockChain.oldest_transaction = new_tran;
    blockChain.size++;
}

BlockChain BlockChainLoad(ifstream &file) {
    string cur_line;
    Transaction temp_transaction{};
    Block* temp_block = new Block{temp_transaction, "", nullptr, nullptr};
    BlockChain new_blockChain{temp_block, temp_block, 1};
    string data[DATA_SIZE];
    while (getline(file, cur_line)) {
        get_data(data, cur_line);
        BlockChainAppendTransaction(new_blockChain, stoi(data[VALUE_INDEX]), data[SENDER_INDEX], data[RECIVER_INDEX],
                                    data[TIMESTEMP_INDEX]);
    }
    delete_block(new_blockChain, temp_block);
    return new_blockChain;
}

void BlockChainDump(const BlockChain &blockChain, ofstream &file) {
    file << "BlockChain Info:" << std::endl;
    const Block* current = blockChain.newest_transaction;
    for (int i = 0; i < blockChain.size; i++) {
        file << i + 1 << "." << std::endl;
        TransactionDumpInfo(current->transaction, file);
        file << "Transaction timestamp: " << current->timestamp << std::endl;
        current = current->prev_block;
    }
}

void BlockChainDumpHashed(const BlockChain &blockChain, ofstream &file) {
    const Block* current = blockChain.newest_transaction;
    for (int i = 0; i < blockChain.size; i++) {
        file << TransactionHashedMessage(current->transaction);
        if (i != blockChain.size - 1)
            file << std::endl;
        current = current->prev_block;
    }
}

bool BlockChainVerifyFile(const BlockChain &blockChain, std::ifstream &file) {
    const Block* current = blockChain.newest_transaction;
    string cur_line;
    while (getline(file, cur_line)) {
        if (current == nullptr)
            return false;
        if (!TransactionVerifyHashedMessage(current->transaction, cur_line))
            return false;
        current = current->prev_block;
    }
    if (current == nullptr) return true;
    return false;
}

void BlockChainCompress(BlockChain &blockChain) {
    Block* current = blockChain.newest_transaction;
    for (; current != nullptr && current->prev_block != nullptr ; current = current->prev_block) {
        Block* final_same_block = current->prev_block;
        if (check_if_same(current, final_same_block)) {
            for (; final_same_block != blockChain.oldest_transaction && check_if_same(current, final_same_block->prev_block);
                final_same_block = final_same_block->prev_block) {}
            while (true) {
                Block* to_delete;
                bool to_break = false;
                if (final_same_block->next_block != current) {
                    to_delete = final_same_block->next_block;
                }
                else {
                    to_delete = final_same_block;
                    to_break = true;
                }
                current->transaction.value += to_delete->transaction.value;
                delete_block(blockChain, to_delete);
                if (to_break) break;

            }
        }
    }
}

void BlockChainTransform(BlockChain &blockChain, updateFunction function) {
    Block* current = blockChain.newest_transaction;
    for (int i = 0; i < blockChain.size; i++) {
        current->transaction.value = function(current->transaction.value);
        current = current->prev_block;
    }
}
