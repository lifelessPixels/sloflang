#include <iostream>
#include <fstream>

#include <token.h>
#include <tokenizer.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "usage: " << argv[0] << " <input file>" << std::endl;
        return 1;
    }

    std::string input_file_path { argv[1] };
    std::ifstream input_file { input_file_path };
    if(!input_file) {
        std::cerr << "error: could not open file '" << input_file_path << "' for reading" << std::endl;
        return 1;  
    }

    std::string input_file_contents { 
        std::istreambuf_iterator<slof::c8>(input_file), 
        std::istreambuf_iterator<slof::c8>() 
    };

    auto tokenization_result = slof::Tokenizer::tokenize(input_file_contents);
    if(tokenization_result.is_error()) {
        std::cout << "error (tokenizer): " << tokenization_result.error_message() << std::endl;
    } else {
        auto& token_stream = tokenization_result.token_stream();
        std::cout << "Token list (" << token_stream.remaining_items() << " items)" << std::endl;
        while(!token_stream.eos()) {
            std::cout << "   - " << token_stream.consume_unchecked() << std::endl;
        }
    }

    return 0;
}