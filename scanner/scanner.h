#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include "../util/types.h"

void scan(std::istream& is, std::ostream& os, std::vector<Token>& stream, std::ostream& err);

#endif //SCANNER_H
