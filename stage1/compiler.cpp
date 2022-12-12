#include <iostream>

#include <token.h>
#include <tokenizer.h>

int main(int, char**) {
    std::cout << "Hello slof!" << std::endl;

    slof::Tokenizer tokenizer { "let else be if" };

    std::cout << "Token list: " << std::endl;
    while(!tokenizer.eos()) {
        std::cout << "   - " << tokenizer.consume_unchecked() << std::endl;
    }

    return 0;
}