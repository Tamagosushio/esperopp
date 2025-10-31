# include <iostream>
# include <fstream>

# include "parser.hpp"

int main(int argc, char* argv[]) {
  // ファイル名取得
  // if (!argc == 2) throw std::runtime_error("<input file>");
  // std::string fileName = argv[1];
  std::string fileName = "./test.txt";
  // ファイルオープン
  std::ifstream file;
  file.open(fileName);
  if (!file.is_open()) throw std::runtime_error("Failed to open a " + fileName);
  // ファイル読み込み
  std::string content;
  file.seekg(0, std::ios::end);
  content.reserve(file.tellg());
  file.seekg(0, std::ios::beg);
  // イテレータで読みこむ
  content.assign(
    (std::istreambuf_iterator<char>(file)),
    std::istreambuf_iterator<char>()
  );
  // ファイル表示
  std::cout << content << std::endl;

  std::cout << std::endl;
  for(int i = 0; i < 64; ++i) std::cout << '-';
  std::cout << std::endl << std::endl;

  Tokenizer tokenizer(content);
  const std::vector<Token> tokens = tokenizer.tokenize();
  for (const Token& token : tokens) {
    std::cout << std::format("{}", token) << std::endl;
  }

  std::cout << std::endl;
  for(int i = 0; i < 64; ++i) std::cout << '-';
  std::cout << std::endl << std::endl;

  Parser parser(tokens);
  try {
    auto program = parser.parse();
    std::cout << program->toString() << std::endl << std::endl;
  } catch (const std::exception& err) {
    std::cerr << "Parse error: " << err.what() << std::endl;
    std::cerr << parser.getCurrentPosition() << std::endl;
    std::cerr << std::format("{}", parser.getCurrentToken()) << std::endl;
    return 1;
  }

}
