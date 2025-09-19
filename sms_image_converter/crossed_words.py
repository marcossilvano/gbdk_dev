
crossed_words = [
    "lmasdlsamxdlkmasdlkasdlmasd",
    "lmaxdlkmasdlkmasdlkasdlmasd",
    "lmasmlkmasdlkmasdlkxsdlmasd",
    "lmasdakmasdlkmasdlkmsdlmasd",
    "lmasdlsmasdlxmasdlkasdlmasd",
    "lmasdlkmasdlkmasdlkssdlmasd",
    "lmasdlkmasdlkmasdlkasdlmasd",
    "lmasdlkmasdlkmasdlkasdlmasd"
]

def check_limits(value: int, uppet_limit: int) -> bool:
    return value < 0 or value >= uppet_limit

def check_word(text: str, row: int, col: int, delta_row: int, delta_col) -> bool:
    for i in range(len(text)):
        if text[i] == crossed_words[row][col]:
            row += delta_row
            col += delta_col

            if row < 0 or row >= len(crossed_words) or \
               col < 0 or col >= len(crossed_words[0]):
                return False
        else:
            return False
    return True

def main():
    count: int = 0
    for row in range(len(crossed_words)):
        for col in range(len(crossed_words[0])):
            count += int(check_word("xmas", row, col, -1,  0) or
                         check_word("xmas", row, col,  1,  0) or
                         check_word("xmas", row, col,  0, -1) or
                         check_word("xmas", row, col,  0,  1) or
                         check_word("xmas", row, col,  1,  1) or
                         check_word("xmas", row, col, -1,  1) or
                         check_word("xmas", row, col,  1, -1) or
                         check_word("xmas", row, col, -1, -1))
            
    print("Words found:", count)

if __name__ == '__main__':
    main()