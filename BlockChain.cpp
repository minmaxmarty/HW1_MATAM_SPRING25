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

void delete_oldest_tran(BlockChain &blockChain, Block *block_to_delete) {
    blockChain.oldest_tran = blockChain.oldest_tran->next_block;
    blockChain.oldest_tran->prev_block = nullptr;
    block_to_delete->next_block = nullptr;
    delete block_to_delete;
}

void delete_newest_tran(BlockChain &blockChain, Block *block_to_delete) {
    blockChain.newest_tran = blockChain.newest_tran->prev_block;
    blockChain.newest_tran->next_block = nullptr;
    block_to_delete->prev_block = nullptr;
    delete block_to_delete;
}

void delete_middle_tran(Block *block_to_delete) {
    block_to_delete->prev_block->next_block = block_to_delete->next_block;
    block_to_delete->next_block->prev_block = block_to_delete->prev_block;
    block_to_delete->prev_block = nullptr;
    block_to_delete->next_block = nullptr;
    delete block_to_delete;
}

bool check_if_same(const Block *current, const Block *final) {
    return current->transaction.sender == final->transaction.sender && current->transaction.receiver == final->transaction.receiver;
}

void delete_block(BlockChain& blockChain, Block* block_to_delete) {
    if (block_to_delete == blockChain.newest_tran) {
        delete_newest_tran(blockChain, block_to_delete);
    } else if (block_to_delete == blockChain.oldest_tran) {
        delete_oldest_tran(blockChain, block_to_delete);
    } else {
        delete_middle_tran(block_to_delete);
    }
}

//---------------------------- BlockChain -----------------------------//

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
    Block* new_tran = new Block{transaction, timestamp, blockChain.oldest_tran, nullptr};
    blockChain.oldest_tran->prev_block = new_tran;
    blockChain.oldest_tran = new_tran;
    blockChain.size++;
}

BlockChain BlockChainLoad(ifstream &file) {
    string cur_line;
    Transaction temp_transaction{};
    Block* temp_block = new Block{temp_transaction, "", nullptr, nullptr};
    BlockChain new_blockChain{temp_block, temp_block, 0};
    string data[DATA_SIZE];
    while (getline(file, cur_line)) {
        get_data(data, cur_line);
        BlockChainAppendTransaction(new_blockChain, stoi(data[VALUE_INDEX]), data[SENDER_INDEX], data[RECIVER_INDEX],
                                    data[TIMESTEMP_INDEX]);
    }
    delete_newest_tran(new_blockChain, temp_block);
    return new_blockChain;
}

void BlockChainDump(const BlockChain &blockChain, ofstream &file) {
    file << "BlockChain Info:" << std::endl;
    const Block* current = blockChain.newest_tran;
    for (int i = 0; i < blockChain.size; i++) {
        file << i + 1 << "." << std::endl;
        TransactionDumpInfo(current->transaction, file);
        file << "Transaction timestamp: " << current->timestamp << std::endl;
        current = current->prev_block;
    }
}

void BlockChainDumpHashed(const BlockChain &blockChain, ofstream &file) {
    const Block* current = blockChain.newest_tran;
    for (int i = 0; i < blockChain.size; i++) {
        file << TransactionHashedMessage(current->transaction);
        if (i != blockChain.size - 1)
            file << std::endl;
        current = current->prev_block;
    }
}

bool BlockChainVerifyFile(const BlockChain &blockChain, std::ifstream &file) {
    const Block* current = blockChain.newest_tran;
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
    Block* current = blockChain.newest_tran;
    for (; current != nullptr; current = current->prev_block) {
        Block* final_same_block = current->prev_block;
        if (check_if_same(current, final_same_block)) {
            int decrease_size_by = 1;
            for (; check_if_same(current, final_same_block) && final_same_block != blockChain.oldest_tran;
                final_same_block = final_same_block->prev_block) {
                decrease_size_by++;
            }
            blockChain.size -= decrease_size_by;
            while (final_same_block->next_block != current) {
                current->transaction.value += final_same_block->next_block->transaction.value;
                delete_block(blockChain, final_same_block->next_block);
            }
            current->transaction.value += final_same_block->transaction.value;
            delete_block(blockChain, final_same_block);
        }
    }
}

void BlockChainTransform(BlockChain &blockChain, updateFunction function) {
    Block* current = blockChain.newest_tran;
    for (int i = 0; i < blockChain.size; i++) {
        current->transaction.value = function(current->transaction.value);
        current = current->prev_block;
    }
}
