# include "token.hpp"
# include <iostream>
# include <fstream>

int main(int argc, char* argv[]) {
  // ファイル名取得
  if (!argc == 2) throw std::runtime_error("<input file>");
  std::string fileName = argv[1];
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
  for (const Token& token : tokenizer.tokenize()) {
    std::cout << std::format("{}", token) << std::endl;
  }
}
