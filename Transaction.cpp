#include "Transaction.h"
#include "Utilities.h"

void TransactionDumpInfo(const Transaction& transaction, ofstream& file) {
    const string to_print[] = {"Sender Name: ", "Receiver Name:", "Transaction Value: "};
    file << to_print[0] << transaction.sender << std::endl;
    file << to_print[1] << transaction.receiver << std::endl;
    file << to_print[2] << transaction.value << std::endl;
}

string TransactionHashedMessage(const Transaction &transaction) {
    return hash((int)transaction.value, transaction.sender, transaction.receiver);
}

bool TransactionVerifyHashedMessage(const Transaction &transaction, string hashedMessage) {
    return TransactionHashedMessage(transaction) == hashedMessage;
}
