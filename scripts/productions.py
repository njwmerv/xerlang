from constants import productions

for prod in productions:
    PROD = prod.split(' ')
    # print(PROD)
    rhs = '{'
    count: int = 0

    if PROD[2] == 'ε':
        count = 0
        rhs = '{}'
    else:
        for sym in PROD[2:]:
            count += 1
            rhs += f'ParserSymbol::{sym},'
        rhs += '}'
    print('\t{' + f'ParserSymbol::{PROD[0]}, {count}, {rhs}' + '},')
