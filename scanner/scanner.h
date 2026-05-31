#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include "../util/types.h"

void scan(std::istream& is, std::vector<Token>& stream, std::ostream& err);

#endif //SCANNER_H
