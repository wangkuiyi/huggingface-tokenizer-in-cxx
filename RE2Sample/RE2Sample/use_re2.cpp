//
//  use_re2.cpp
//  RE2Sample
//
//  Created by Yi Wang on 2/12/23.
//

#include <stdio.h>

#include <string>
#include <iostream>

#include <re2/re2.h>
#include <re2/stringpiece.h>

extern "C" void hello() {
  std::string s("Hello\n");
  re2::StringPiece input(s);
  std::cout << input.size() << std::endl;
}
